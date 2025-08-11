/*
 * Copyright (C) 2025 Kernkonzept GmbH.
 * Author(s): Martin Decky <martin.decky@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

/**
 * \file
 * \brief Basic next-fit memory allocator
 *
 * This is a basic next-fit memory allocator that maintains multiple heap areas
 * using an embedded linked list. Each heap area contains adjacent aligned
 * blocks that are marked as free/allocated.
 *
 * During an allocation request, the blocks and eventually areas are traversed
 * starting with the recently touched block to find the first suitable free
 * block (which is then fragmented if required). If no suitable block is found
 * in any heap area, a new heap area is requested. For this purpose, the user
 * of this allocator is required to provide the implementation of the
 * #umalloc_area_create() function and provide the #umalloc_area_granularity
 * symbol with a value.
 *
 * During a deallocation request, the given block is marked as free and
 * possibly merged with the adjacent free blocks to limit the fragmentation.
 * For simplicity, heap areas are never disposed, even if entirely free.
 *
 * \note The current implementation is NOT thread-safe. If the user wants to
 *       use this allocator concurrently, they need to deploy their custom
 *       mutual exclusion mechanism around the public calls to the allocator.
 *
 * \note The current implementation does not use any run-time assertions
 *       (although such assertions could be useful to detect memory corruption
 *       bugs and other issues). Be careful when possibly adding such
 *       mechanisms in the future and make sure they themselves do not require
 *       memory allocation (since this allocator is not reentrant).
 */

#include <new>
#include <cerrno>
#include <cstdlib>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <limits>
#include <l4/cxx/numeric>
#include <l4/cxx/type_traits>
#include <l4/libumalloc/umalloc.h>

namespace umalloc {

enum : std::size_t
{
  /**
   * Base/default alignment of allocation and supporting structures.
   */
  Base_alignment = alignof(max_align_t)
};

/*
 * This is required in order to be able to combine the heap area pointer
 * and the free/allocated flag into a single unsigned integer field.
 */
static_assert(Base_alignment >= 2, "Base alignment is at least 2");

/**
 * Free/allocated flag.
 */
enum Flag : bool
{
  Alloc = false,  /**< Allocated. */
  Free = true     /**< Free. */
};

namespace value
{
  /**
   * Align up an unsigned value to the nearest higher multiple of a alignment.
   *
   * Contrary to the typical implementations, the alignment does not have to be
   * a power of two. If the input value is already a multiple of the alignment,
   * then the original value is returned.
   *
   * \tparam T          Unsigned integer type of the value/alignment.
   *
   * \param  value      Value to align up to the nearest higher multiple.
   * \param  alignment  Alignment to align up the value to. Must be non-zero.
   *
   * \return Value aligned up to the nearest higher multiple of the alignment.
   */
  template <typename T>
  constexpr T align_up(const T value, const size_t alignment)
  {
    static_assert(cxx::Type_traits<T>::is_unsigned, "Type must be unsigned");

    T remainder = value % alignment;
    return remainder ? value + (alignment - remainder) : value;
  }
};

namespace ptr
{
  /**
   * Pointer type erasure.
   *
   * \tparam T  Type to erase.
   *
   * \param ptr  Pointer to erase the type from.
   *
   * \return Original pointer with the type erased.
   */
  template <typename T>
  constexpr void *erasure(T *ptr) noexcept
  {
    return ptr;
  }

  /**
   * Constant pointer type erasure.
   *
   * \tparam T  Type to erase.
   *
   * \param ptr  Constant pointer to erase the type from.
   *
   * \return Original constant pointer with the type erased.
   */
  template <typename T>
  constexpr const void *erasure(const T *ptr) noexcept
  {
    return ptr;
  }

  /**
   * Pointer arithmetic addition.
   *
   * \tparam DEST  Destination type.
   * \tparam SRC   Source type.
   *
   * \param ptr     Input pointer.
   * \param offset  Offset to be added.
   *
   * \return Input pointer increased by the offset in bytes.
   */
  template <typename DEST, typename SRC>
  constexpr DEST *offset(SRC *ptr, const size_t offset) noexcept
  {
    auto ptr_offset = static_cast<std::byte *>(erasure(ptr)) + offset;
    return static_cast<DEST *>(erasure(ptr_offset));
  }

  /**
   * Pointer arithmetic subtraction.
   *
   * \tparam DEST  Destination type.
   * \tparam SRC   Source type.
   *
   * \param ptr     Input pointer.
   * \param offset  Offset to be subtracted.
   *
   * \return Input pointer decreased by the offset in bytes.
   */
  template <typename DEST, typename SRC>
  constexpr DEST *subtract(SRC *ptr, const size_t offset) noexcept
  {
    auto ptr_offset = static_cast<std::byte *>(erasure(ptr)) - offset;
    return static_cast<DEST *>(erasure(ptr_offset));
  }

  /**
   * Align up a pointer to the nearest higher multiple of a alignment.
   *
   * Contrary to the typical implementations, the alignment does not have to be
   * a power of two. If the input pointer is already aligned to a multiple of
   * the alignment, then the original pointer is returned.
   *
   * \tparam DEST  Destination type.
   * \tparam SRC   Source type.
   *
   * \param  ptr        Pointer to align up to the nearest higher multiple.
   * \param  alignment  Alignment to align up the pointer to. Must be non-zero.
   *
   * \return Pointer aligned up to the nearest higher multiple of the
   *         alignment.
   */
  template <typename DEST, typename SRC>
  constexpr DEST *align_up(SRC *ptr, const size_t alignment) noexcept
  {
    auto addr = reinterpret_cast<uintptr_t>(ptr);
    return reinterpret_cast<DEST *>(value::align_up(addr, alignment));
  }

  /**
   * Compute the difference of two pointers.
   *
   * \tparam T1  First argument type.
   * \tparam T2  Second argument type.
   *
   * \param ptr       First pointer.
   * \param ptr_next  Second pointer (must be higher or equal to the first
   *                  pointer).
   *
   * \return Difference between the second and the first pointer in bytes.
   */
  template <typename T1, typename T2>
  constexpr size_t diff(T1 *ptr, T2 *ptr_next) noexcept
  {
    auto _ptr = static_cast<const std::byte *>(erasure(ptr));
    auto _ptr_next = static_cast<const std::byte *>(erasure(ptr_next));

    return _ptr_next - _ptr;
  }
};

class Block;

/**
 * Heap area header.
 *
 * This structure represents the heap area itself and contains the metadata of
 * the heap area. The payload of the heap area (i.e. the heap blocks and the
 * user payload, see #Header and #Footer) starts beyond this metadata plus base
 * alignment.
 */
class Area
{
public:
  /**
   * Trivial custom list of heap areas.
   *
   * Due to the constrained environment this allocator might be deployed in,
   * we roll out our own trivial custom linked list of heap areas that we have
   * fully under control.
   *
   * Potential replacements could be #cxx::S_list_bss (if we can tolerate
   * pushing new items at the front of the list and hence a reverse order
   * of traversal) or #cxx:S_list_tail (if we can tolerate an explicit
   * non-trivial constructor of the list).
   */
  struct List
  {
    Area *first;  /**< First heap area in the linked list of heap areas. */
    Area *last;   /**< Last heap area in the linked list of heap areas. */
  };

  /**
   * Create a heap area.
   *
   * Note that the heap area contains the header and the payload.
   *
   * \param area_size  Raw heap area size (i.e. including the header).
   */
  Area(const size_t area_size);

  /**
   * Compute the raw size of a heap area.
   *
   * Compute the raw size of a heap area from the size of its payload (i.e.
   * heap blocks).
   *
   * \note This needs to be kept in sync with #first_block().
   *
   * \param size  Size of the payload of the heap area.
   *
   * \return Raw size of the heap area.
   */
  static size_t raw_size(const size_t size)
  {
    // Since the heap block headers in the payload need to be properly aligned,
    // we need to take that alignment into account here.
    return size + value::align_up(sizeof(Area), Base_alignment);
  }

  /**
   * Compute pointer offset within the heap area.
   *
   * \tparam T  Return type.
   *
   * \param offset  Byte offset within the heap area.
   *
   * \return Pointer to the given offset within the heap area.
   */
  template <typename T>
  T *offset(const size_t offset)
  {
    return ptr::offset<T>(this, offset);
  }

  /**
   * Get the first heap block of the heap area.
   *
   * \note This needs to be kept in sync with #raw_size().
   *
   * \return First heap block of the heap area.
   */
  Block *first_block()
  {
    // Since the heap block headers in the payload need to be properly aligned,
    // we need to take that alignment into account here.
    return ptr::align_up<Block>(this + 1, Base_alignment);
  }

  static List list;  /**< List of heap areas. */

  /**
   * Heap area end.
   *
   * First address after the last valid byte of the heap area.
   */
  void *end;

  Area *next;  /**< Next heap area in the linked list of heap areas. */
};

/**
 * Heap block footer.
 *
 * This structure represents the heap block itself and contains a copy of
 * the metadata of the heap block from the block header (#Header). Its purpose
 * is the implementation of the backwards traversal of the heap blocks.
 *
 * The payload of the heap block (i.e. the dynamically allocated memory
 * provided to the user) precedes this metadata.
 */
class Footer
{
public:
  /**
   * Raw size of the heap block.
   *
   * This includes the header, the payload and the footer.
   */
  size_t size;
};

/**
 * Heap block header.
 *
 * This structure represents the heap block itself and contains the metadata of
 * the heap block. The payload of the heap block (i.e. the dynamically allocated
 * memory provided to the user) starts beyond this metadata. The payload is
 * followed by the heap block footer (#Footer).
 */
class Block
{
public:
  /**
   * Get heap block from a payload pointer.
   *
   * \param ptr  Heap block payload pointer. Must point to the payload of
   *             a valid heap block.
   *
   * \return Corresponding heap block.
   */
  static Block *from_payload(void *ptr)
  {
    return ptr::subtract<Block>(ptr, sizeof(Block));
  }

  /**
   * Combine the heap area pointer and the free/allocated flag into a single
   * value.
   *
   * This is guaranteed to work thanks to the fact that the heap areas are
   * always at least #Base_alignment aligned.
   *
   * \param area  Heap area pointer to combine.
   * \param flag  Free/allocated flag to combine.
   *
   * \return The combined heap area pointer and the free/allocated flag value.
   */
  static uintptr_t combine_area_flag(uintptr_t area, const Flag flag)
  {
    return (area & Area_mask) | (static_cast<uintptr_t>(flag) & Flag_mask);
  }

  /**
   * Combine the heap area and the free/allocated flag into a single value.
   *
   * This is guaranteed to work thanks to the fact that the heap areas are
   * always at least #Base_alignment aligned.
   *
   * \param area  Heap area to combine.
   * \param flag  Free/allocated flag to combine.
   *
   * \return The combined heap area and the free/allocated flag value.
   */
  static uintptr_t combine_area_flag(Area *area, const Flag flag)
  {
    return combine_area_flag(reinterpret_cast<uintptr_t>(area), flag);
  }

  /**
   * Initialize a heap block.
   *
   * Note that the heap block contains the header, the payload and the footer.
   *
   * \param ptr         Raw pointer where to initialize the heap block.
   * \param block_size  Raw heap block size (i.e. including the header and
   *                    the footer).
   * \param flag        Free/allocated flag.
   * \param area        Heap area to which this heap block belongs.
   *
   * \return The initialized heap block (header).
   */
  static Block *init(void *ptr, const size_t block_size, const Flag flag,
                     Area *area)
  {
    auto header = static_cast<Block *>(ptr);

    header->size = block_size;
    header->_area_flag = combine_area_flag(area, flag);

    auto footer = header->footer();
    footer->size = block_size;

    return header;
  }

  /**
   * Compute the payload size of a heap block.
   *
   * Compute the payload size of a heap block from the raw size.
   *
   * \param raw_size  Raw size of the heap block.
   *
   * \return Payload size of the heap block.
   */
  static size_t payload_size(const size_t raw_size)
  {
    return raw_size - sizeof(Block) - sizeof(Footer);
  }

  /**
   * Compute the raw size of a heap block.
   *
   * Compute the raw size of a heap block from the size of its payload (i.e.
   * the dynamically allocated memory provided to the user).
   *
   * \param size  Size of the payload of the heap block.
   *
   * \return Raw size of the heap block.
   */
  static size_t raw_size(const size_t size)
  {
    return size + sizeof(Block) + sizeof(Footer);
  }

  /**
   * Compute pointer offset within the heap block.
   *
   * \tparam T  Return type.
   *
   * \param offset  Byte offset within the heap block.
   *
   * \return Pointer to the given offset within the heap block.
   */
  template <typename T>
  T *offset(const size_t offset)
  {
    return ptr::offset<T>(this, offset);
  }

  /**
   * Compute the payload pointer within the heap block.
   *
   * \param extra_offset  Optional extra offset within the payload.
   *
   * \return Pointer to the payload within the heap block (with optional
   *         extra offset).
   */
  void *payload(const size_t extra_offset = 0)
  {
    return offset<void>(sizeof(Block) + extra_offset);
  }

  /**
   * \return Footer of the heap block.
   */
  Footer *footer()
  {
    return offset<Footer>(size - sizeof(Footer));
  }

  /**
   * Get the next heap block.
   *
   * \note This method does not check if the current heap block
   *       is followed by a valid heap block within its heap area.
   *
   * \return Next heap block.
   */
  Block *next()
  {
    return offset<Block>(size);
  }

  /**
   * Get the previous heap block.
   *
   * \note This method does not check if the current heap block
   *       is preceded by a valid heap block within its heap area.
   *
   * \return Previous heap block.
   */
  Block *prev()
  {
    auto prev_footer = ptr::subtract<Footer>(this, sizeof(Footer));
    return ptr::subtract<Block>(this, prev_footer->size);
  }

  /**
   * Fragment the heap block and mark part of it as allocated.
   *
   * If it is possible to fragment the heap block into an allocated block
   * of the given size and a remanding free block, it is done so. Otherwise
   * the whole heap block is marked as allocated.
   *
   * \param mark_size  Raw size of the portion of the block that needs to be
   *                   marked as allocated.
   */
  void fragment_and_mark(const size_t mark_size)
  {
    auto _size = size;
    auto _area = area();

    // When fragmenting the block, we need to have space for the additional
    // block header and footer. Therefore we need to take that into account
    // when deciding whether to do the fragmentation.
    auto fragmentation_limit = Block::raw_size(mark_size);

    if (_size > fragmentation_limit)
      {
        // The block is sufficiently large to be fragmented.
        auto ptr_next = offset<void>(mark_size);
        Block::init(this, mark_size, Alloc, _area);
        Block::init(ptr_next, _size - mark_size, Free, _area);
      }
    else
      {
        // The block is not large enough to be fragmented. Just mark it as
        // allocated.
        flag(Alloc);
      }
  }

  /**
   * Previously touched heap block.
   *
   * During allocation, the heap blocks are traversed starting from this block
   * in a next-fit manner.
   */
  static Block *touched;

  /**
   * Raw size of the heap block.
   *
   * This includes the header, the payload and the footer.
   */
  size_t size;

  /**
   * \return Heap area to which this heap block belongs.
   */
  Area *area() const
  {
    return reinterpret_cast<Area *>(_area_flag & Area_mask);
  }

  /**
   * \return Free/allocated flag.
   */
  Flag flag() const
  {
    return static_cast<Flag>(_area_flag & Flag_mask);
  }

  /**
   * Set the free/allocated flag.
   *
   * \param value  Free/allocated flag to set.
   */
  void flag(const Flag value)
  {
    _area_flag = combine_area_flag(_area_flag & Area_mask, value);
  }

private:
  /**
   * Masks for accessing the combined heap area pointer and free/allocated
   * flag.
   *
   * See #area_flag.
   */
  enum : uintptr_t
  {
    Flag_mask = static_cast<uintptr_t>(1U),
    Area_mask = ~Flag_mask
  };

  /**
   * Combined pointer to the heap area to which this heap block belongs and
   * the free/allocated flag.
   *
   * The least significant bit contains the free/allocated flag and the
   * remaining bits contain the pointer to the heap area to which this heap
   * block belongs. This is guaranteed to work thanks to the fact that the
   * heap areas are always at least #Base_alignment aligned.
   */
  uintptr_t _area_flag;
};

Area::List Area::list = {};
Block *Block::touched = nullptr;

Area::Area(const size_t area_size)
{
  end = offset<void>(area_size);
  next = nullptr;

  auto block = first_block();
  auto block_size = ptr::diff(block, end);

  Block::init(block, block_size, Free, this);

  // Push the area into the list of areas.
  if (!list.last)
    {
      // The list is empty. Make the area the first and the last item.
      list.first = this;
      list.last = this;
    }
  else
    {
      // The list is non-empty. Push the area at the tail of the list and
      // make it the last item.
      list.last->next = this;
      list.last = this;
    }
}

/**
 * Allocate memory in the given heap area.
 *
 * This implements the core of the next-fit allocation algorithm, i.e.
 * traversing the heap blocks within the given heap area (starting at the given
 * first block and ending either at a sentinel block or at the last block of
 * the area), finding a suitable candidate free block and doing appropriate
 * fragmentation of the heap blocks.
 *
 * \param first         First block to start the traversal.
 * \param sentinel      Sentinel block on which to stop the traversal. Ignored
 *                      if nullptr.
 * \param request_size  Raw block size to allocate.
 * \param alignment     Payload alignment requirements. Must be at least
 *                      Base_alignment.
 *
 * \return Valid allocated memory or nullptr if the allocation failed.
 */
static void *alloc_in_area(Block *first, Block *sentinel, size_t request_size,
                           size_t alignment)
{
  auto area = first->area();

  // Iterate over the blocks in the given heap area starting with the first
  // block.
  for (auto block = first; block < area->end; block = block->next())
    {
      // Finish the iteration if we encounter the sentinel block.
      if (block == sentinel)
        break;

      // Check if the current block is a suitable candidate.
      if (block->flag() == Free && (block->size >= request_size))
        {
          // Account for the alignment requirements.
          auto ptr = block->payload();
          auto ptr_aligned = ptr::align_up<void>(ptr, alignment);

          if (ptr == ptr_aligned)
            {
              // The payload of the current block satisfies the alignment
              // requirements. Use it as is.
              block->fragment_and_mark(request_size);
              Block::touched = block;
              return ptr;
            }

          // Allocation prefix to satisfy the alignment requirements.
          auto prefix = ptr::diff(ptr, ptr_aligned);

          // Check if the current block is still a suitable candidate when
          // taking the allocation prefix into account.
          if (block->size >= prefix + request_size)
            {
              // The allocation prefix needs to be covered by a free block
              // that precedes the current block to preserve the integrity
              // of the heap area.

              if (block == area->first_block())
                {
                  // The current block is the first block in the heap area.
                  // Therefore we need to create a new free block that precedes
                  // the current block.

                  // The allocation prefix needs to be large enough to fit in
                  // a new free block.
                  if (prefix < Block::raw_size(0))
                    {
                      // Enlarge the allocation prefix to fit in a new free
                      // block (with at least zero payload size). We need to
                      // account for the allocation alignment requirements
                      // again.
                      auto ptr_extra = block->payload(Block::raw_size(0));
                      ptr_aligned = ptr::align_up<void>(ptr_extra, alignment);
                      prefix = ptr::diff(ptr, ptr_aligned);
                    }

                  // Check if the current block is still a suitable candidate
                  // when taking the potentially enlarged allocation prefix
                  // into account.
                  if (block->size >= prefix + request_size)
                    {
                      // Split the current block into a free block that covers
                      // the allocation prefix and a next block that is
                      // actually used for the allocation.
                      auto ptr_next = block->offset<void>(prefix);
                      auto suffix = block->size - prefix;

                      Block::init(block, prefix, Free, area);

                      auto next = Block::init(ptr_next, suffix, Free, area);
                      next->fragment_and_mark(request_size);

                      Block::touched = block;
                      return ptr_aligned;
                    }

                  // The current block is no longer a suitable candidate when
                  // taking the allocation prefix into account. Since the
                  // current block is the first block of the heap area, we have
                  // to continue the traversal.
                }
              else
                {
                  // There is a block preceding the current block.
                  auto prev = block->prev();

                  // Split up a next block that is actually used for the
                  // allocation from the current block.
                  auto ptr_next = block->offset<void>(prefix);
                  auto suffix = block->size - prefix;

                  if (prev->flag() == Alloc && (prefix >= Block::raw_size(0)))
                    {
                      // The preceding block is allocated, but the allocation
                      // prefix is sufficiently large to allow for filling in
                      // a new free block between the preceding block and the
                      // current block to cover the allocation prefix.
                      Block::init(block, prefix, Free, area);
                    }
                  else
                    {
                      // This branch covers two cases:
                      //
                      // (1) The preceding block is free.
                      // (2) The preceding block is allocated, but the
                      //     allocation prefix is not large enough to fill in
                      //     a new free block.
                      //
                      // In both cases we simply enlarge the preceding block
                      // to cover the allocation prefix and avoid fragmenting
                      // the heap. For the allocated block, this means that we
                      // enlarge the size of a previously allocated block, but
                      // this is harmless given our API.
                      Block::init(prev, prev->size + prefix, prev->flag(),
                                  area);
                    }

                  auto next = Block::init(ptr_next, suffix, Free, area);
                  next->fragment_and_mark(request_size);

                  Block::touched = next;
                  return ptr_aligned;
                }
            }
        }

      // The current block is not a suitable canditate. Continue with the next
      // block.
    }

  // No block suitable for allocation found.
  return nullptr;
}

/**
 * Try to create a new heap area and allocate in it.
 *
 * This function is typically used when the allocation within the existing
 * heap areas has been unsuccessful.
 *
 * \param request_size  Raw block size to allocate.
 * \param alignment     Payload alignment requirements. Must be at least
 *                      Base_alignment.
 *
 * \return Valid allocated memory or nullptr if the allocation failed.
 */
static void *grow_and_alloc(size_t request_size, size_t alignment)
{
  // The heap area to create needs to accommodate the required raw heap block
  // size (including the overhead of the heap area itself) and respect the
  // heap area granularity as defined by the user.
  auto area_size = value::align_up(Area::raw_size(request_size),
                                   umalloc_area_granularity);

  // Create the heap area.
  auto ptr = umalloc_area_create(area_size);
  if (!ptr)
    return nullptr;

  // Initialize the heap area.
  auto area = new (ptr) Area(area_size);

  // Allocate within the heap area.
  return alloc_in_area(area->first_block(), nullptr, request_size, alignment);
}

/**
 * Allocate memory.
 *
 * \param size       Size in bytes to allocate.
 * \param alignment  Alignment requirement. Adjusted to cover at least
 *                   Base_alignment.
 *
 * \return Valid allocated memory or nullptr if the allocation failed.
 */
static void *alloc(size_t size, size_t alignment = Base_alignment)
{
  // Adjust for invalid alignment.
  if (alignment == 0)
    alignment = Base_alignment;

  // Always cover both Base_alignment and requested alignment.
  auto total_alignment
    = cxx::lcm(alignment, static_cast<typeof(alignment)>(Base_alignment));

  // Overflow check.
  if (total_alignment < alignment)
    return nullptr;

  // The size of the allocated heap block needs to be aligned, too, because the
  // heap block footer structure also needs to reside on an aligned address in
  // order to avoid unaligned memory accesses.
  auto request_size = Block::raw_size(value::align_up(size, Base_alignment));

  // Previously touched heap block as the pivot for the next-fit approach.
  auto pivot = Block::touched;

  if (pivot)
    {
      // Next-fit approach. We start with the pivot.
      auto ptr = alloc_in_area(pivot, nullptr, request_size, total_alignment);
      if (ptr)
        return ptr;
    }

  // First-fit appraoch. We use the pivot as the sentinel to avoid traversing
  // the same blocks twice.
  for (auto area = Area::list.first; area; area = area->next)
    {
      auto first = area->first_block();
      auto ptr = alloc_in_area(first, pivot, request_size, total_alignment);
      if (ptr)
        return ptr;
    }

  // As a last resort, try creating a new heap area.
  return grow_and_alloc(request_size, total_alignment);
}

/**
 * Deallocate memory.
 *
 * \param ptr  Memory to deallocate. Must be a valid allocated memory.
 */
static void dealloc(void *ptr)
{
  // Get the corresponding heap block and area.
  auto block = Block::from_payload(ptr);
  auto area = block->area();

  // Mark the block as free.
  block->flag(Free);

  // Check the next block. If it is valid and free, merge it with the current
  // block to limit fragmentation.
  auto next = block->next();
  if ((next < area->end) && next->flag() == Free)
    {
      Block::init(block, block->size + next->size, Free, area);
      Block::touched = block;
    }

  // Check the preceding block. If it is valid and free, merge it with the
  // current block to limit fragmentation.
  if (block != area->first_block())
    {
      auto prev = block->prev();
      if (prev->flag() == Free)
        {
          Block::init(prev, prev->size + block->size, Free, area);
          Block::touched = prev;
        }
    }
}

/**
 * Reallocate previously allocated memory.
 *
 * This routine preserves the data in the intersection of the previously
 * and newly allocated memory. It tries to reallocate the memory in-place if
 * possible.
 *
 * \param ptr   Previously allocated valid memory.
 * \param size  Size in bytes of the memory to newly allocate.
 *
 * \return Valid reallocated memory or nullptr if the reallocation failed (in
 *         which case the previously allocated memory is not touched).
 */
static void *realloc(void *ptr, size_t size)
{
  // Get the corresponding heap block and area.
  auto block = Block::from_payload(ptr);
  auto area = block->area();

  // The size of the allocated heap block needs to be aligned, too, because the
  // heap block footer structure also needs to reside an aligned address in
  // order to avoid unaligned memory accesses.
  auto request_size = Block::raw_size(value::align_up(size, Base_alignment));
  auto block_size = block->size;

  // Skip the trivial case.
  if (block_size == request_size)
    return ptr;

  if (block_size > request_size)
    {
      // In-place shrinking of the heap block.
      auto excess = block_size - request_size;

      // Check if we are able to create a new free block following the shrunken
      // block. Otherwise we just keep the current block as is.
      if (excess >= Block::raw_size(0))
        {
          // Fragment the original block into an allocated block and a trailing
          // free block.
          auto ptr_next = block->offset<void>(request_size);
          Block::init(block, request_size, Alloc, area);
          Block::init(ptr_next, excess, Free, area);
        }

      return ptr;
    }

  // Make sure the block is not the last block of the heap area for the purpose
  // of in-place growing.
  auto next = block->next();
  if (next < area->end)
    {
      if (next->flag() == Free && (block_size + next->size >= request_size))
        {
          // In-place growing of the heap block by first merging the current
          // block with the trailing free block and then fragmenting the block
          // again. Note that the merged block is marked as free temporarily,
          // but that should not affect anything.
          Block::init(block, block_size + next->size, Alloc, area);
          block->fragment_and_mark(request_size);

          Block::touched = block;
          return ptr;
        }
    }

  // No in-place growing possible. Allocate new memory, move the original
  // payload into it and dispose of the original block.
  auto data = alloc(size);
  if (!data)
    return data;

  memcpy(data, ptr, Block::payload_size(block_size));
  dealloc(ptr);
  return data;
}

} // namespace umalloc

/**
 * Standard-compliant malloc implementation.
 *
 * \param size  Size in bytes to allocate.
 *
 * \return Valid allocated memory or nullptr if the allocation failed.
 */
void *malloc(size_t size) noexcept
{
  auto ptr = umalloc::alloc(size);
  if (!ptr)
    errno = ENOMEM;

  return ptr;
}

/**
 * Standard-compliant aligned-alloc implementation.
 *
 * \param alignment  Alignment requirement.
 * \param size       Size in bytes to allocate.
 *
 * \return Valid allocated memory or nullptr if the allocation failed.
 */
void *aligned_alloc(size_t alignment, size_t size) noexcept
{
  auto ptr = umalloc::alloc(size, alignment);
  if (!ptr)
    errno = ENOMEM;

  return ptr;
}

/**
 * Standard-compliant free implementation.
 *
 * \param ptr  Previously allocated valid memory.
 */
void free(void *ptr) noexcept
{
  if (ptr)
    umalloc::dealloc(ptr);
}

/**
 * Standard-compliant calloc implementation.
 *
 * \param nmemb  Number of members to allocate.
 * \param size   Size in bytes of each member.
 *
 * \return Valid allocated memory or nullptr if the allocation failed.
 */
void *calloc(size_t nmemb, size_t size) noexcept
{
  // Avoid multiplication overflow.
  if ((size > 0) && (nmemb > std::numeric_limits<typeof(nmemb)>::max() / size))
    return nullptr;

  auto total_size = nmemb * size;
  auto ptr = umalloc::alloc(total_size);

  // According to the specification, the allocated memory is zero-initialized.
  if (ptr)
    memset(ptr, 0, total_size);
  else
    errno = ENOMEM;

  return ptr;
}

/**
 * Standard-compliant realloc implementation.
 *
 * \param ptr   Previously allocated valid memory or nullptr.
 * \param size  Size in bytes to allocate.
 *
 * \return Valid reallocated memory or nullptr if the reallocation failed.
 *         (in which case the previously allocated memory is not touched).
 */
void *realloc(void *ptr, size_t size) noexcept
{
  if (!ptr)
    return malloc(size);

  ptr = umalloc::realloc(ptr, size);
  if (!ptr)
    errno = ENOMEM;

  return ptr;
}
