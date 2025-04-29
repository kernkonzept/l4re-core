
#pragma once

#include <l4/sys/consts.h>
#include <l4/sys/types.h>
#include <l4/sys/task.h>

namespace L4 {

class Task;
class Kobject;

template< typename T > class L4_EXPORT Cap;

/**
 * Base class for all kinds of capabilities.
 * \attention This class is not for direct use, use L4::Cap instead.
 * \headerfile l4/sys/capability
 *
 * This class contains all the things that are independent of the type
 * of the object referred by the capability.
 *
 * \see L4::Cap for typed capabilities.
 */
class L4_EXPORT Cap_base
{
public:
  /// Special value for uninitialized capability objects.
  enum No_init_type
  {
     /**
      * \brief Special value for constructing uninitialized Cap objects.
      */
     No_init
  };

  /**
   * \brief Invalid capability type.
   */
  enum Cap_type
  {
    Invalid = L4_INVALID_CAP ///< Invalid capability selector
  };

  /**
   * \brief Return capability selector.
   * \return Capability selector.
   */
  l4_cap_idx_t cap() const noexcept { return _c; }

  /**
   * Test whether the capability is a valid capability index (i.e.,
   * not L4_INVALID_CAP).
   *
   * \return True if capability is not invalid, false if invalid
   */
  bool is_valid() const noexcept { return !(_c & L4_INVALID_CAP_BIT); }

  /**
   * Return the transported error code in an invalid capability index.
   */
  int invalid_cap_error() const noexcept { return _c & ~L4_INVALID_CAP_BIT; }

  explicit operator bool () const noexcept
  { return !(_c & L4_INVALID_CAP_BIT); }

  /**
   * Return flexpage for the capability.
   *
   * \param rights   Rights, defaults to 'rws'
   *
   * \return flexpage
   */
  l4_fpage_t fpage(unsigned rights = L4_CAP_FPAGE_RWS) const noexcept
  { return l4_obj_fpage(_c, 0, rights); }

  /**
   * Return send base.
   *
   * \param grant  Indicates if object shall be granted. Allowed values:
   *               #L4_MAP_ITEM_MAP, #L4_MAP_ITEM_GRANT.
   * \param base   Base capability (first in a bundle of aligned capabilities)
   *
   * \return Map object.
   */
  l4_umword_t snd_base(unsigned grant = L4_MAP_ITEM_MAP,
                       l4_cap_idx_t base = L4_INVALID_CAP) const noexcept
  {
    if (base == L4_INVALID_CAP)
      base = _c;
    return l4_map_obj_control(base, grant);
  }


  /**
   * Test if two capabilities are equal.
   */
  bool operator == (Cap_base const &o) const noexcept
  { return _c == o._c; }

  /**
   * Test if two capabilities are not equal.
   */
  bool operator != (Cap_base const &o) const noexcept
  { return _c != o._c; }

  /**
   * Check whether a capability is present (refers to an object).
   *
   * \utcb{u}
   *
   * \retval "l4_msgtag_t::label() > 0"   Capability is present (refers to an
   *                                      object).
   * \retval "l4_msgtag_t::label() == 0"  No capability present (void object or
   *                                      invalid capability slot).
   *
   * A capability is considered present when it refers to an existing
   * kernel object.
   */
  inline l4_msgtag_t validate(l4_utcb_t *u = l4_utcb()) const noexcept;

  /**
   * Check whether a capability is present (refers to an object).
   *
   * \param task  Task to check the capability in.
   * \utcb{u}
   *
   * \retval "l4_msgtag_t::label() > 0"   Capability is present (refers to an
   *                                      object).
   * \retval "l4_msgtag_t::label() == 0"  No capability present (void object or
   *                                      invalid capability slot).
   *
   * A capability is considered present when it refers to an existing
   * kernel object.
   */
  inline l4_msgtag_t validate(Cap<Task> task,
                              l4_utcb_t *u = l4_utcb()) const noexcept;

  /**
   * Set this capability to invalid (L4_INVALID_CAP).
   */
  void invalidate() noexcept { _c = L4_INVALID_CAP; }
protected:
  /**
   * Generate a capability from its C representation.
   *
   * \param c  The C capability
   */
  explicit Cap_base(l4_cap_idx_t c) noexcept : _c(c) {}
  /**
   * Constructor to create an invalid capability.
   */
  explicit Cap_base(Cap_type cap) noexcept : _c(cap) {}

  /**
   * Initialize capability with one of the default capabilities.
   *
   * \param cap  Capability.
   */
  explicit Cap_base(l4_default_caps_t cap) noexcept : _c(cap) {}

  /**
   * \brief Create an uninitialized instance.
   */
  explicit Cap_base() noexcept {}

  /**
   * Replace this capability with the contents of `src`.
   *
   * \param src the source capability.
   *
   * After the operation this capability refers to the object formerly referred
   * to by the source capability `src`, and the source capability no longer
   * refers to an object.
   */
  void move(Cap_base const &src) const
  {
    if (!is_valid() || !src.is_valid())
      return;

    l4_task_map(L4_BASE_TASK_CAP, L4_BASE_TASK_CAP, src.fpage(L4_CAP_FPAGE_RWSD),
                snd_base(L4_MAP_ITEM_GRANT) | L4_FPAGE_C_OBJ_RIGHTS);
  }

  /**
   * Copy a capability.
   * \param src the source capability.
   *
   * After this operation this capability refers to the same object
   * as `src`.
   */
  void copy(Cap_base const &src) const
  {
    if (!is_valid() || !src.is_valid())
      return;

    l4_task_map(L4_BASE_TASK_CAP, L4_BASE_TASK_CAP, src.fpage(L4_CAP_FPAGE_RWSD),
                snd_base() | L4_FPAGE_C_OBJ_RIGHTS);
  }

  /**
   * \brief The C representation of a capability selector. */
  l4_cap_idx_t _c;
};


/**
 * C++ interface for capabilities.
 *
 * \tparam T  Type of the object the capability points to.
 *
 * The C++ version of a capability is comparable to a pointer, in fact
 * it is a kind of smart pointer for our kernel objects and the
 * objects derived from the kernel objects (L4::Kobject).
 *
 * Add
 *
 *     #include <l4/sys/capability>
 *
 * to your code to use the capability interface.
 */
template< typename T >
class L4_EXPORT Cap : public Cap_base
{
private:
  friend class L4::Kobject;

  /**
   * \internal
   * \brief Internal Constructor, use to generate a capability from a `this`
   *        pointer.
   *
   * \param p  The `this` pointer of the Kobject or derived object.
   *
   * \attention This constructor is only useful to generate a capability
   *            from the `this` pointer of an object that is an L4::Kobject.
   *            Do **never** use this constructor for something else!
   */
  explicit Cap(T const *p) noexcept
  : Cap_base(reinterpret_cast<l4_cap_idx_t>(p)) {}

public:

  /**
   * Perform the type conversion that needs to compile in order for a capability
   * of type From to be convertible to one of type T.
   *
   * \tparam From  Type to convert from
   */
  template< typename From >
  static void check_convertible_from() noexcept
  {
    using To = T;
    [[maybe_unused]] To* t = static_cast<From*>(nullptr);
  }

  /**
   * Perform the type conversion that needs to compile in order for a capability
   * of type From te be castable (via the correct cap_cast) to one of type T.
   *
   * \tparam From  Type to convert from
   */
  template< typename From >
  static void check_castable_from() noexcept
  {
    using To = T;
    [[maybe_unused]] To *t = static_cast<To *>(static_cast<From *>(nullptr));
  }

  /**
   * \brief Create a copy from `o`, supporting implicit type casting.
   * \param o  The source selector that shall be copied (and casted).
   */
  template< typename O >
  Cap(Cap<O> const &o) noexcept : Cap_base(o.cap())
  { check_convertible_from<O>(); }

  /**
   * Constructor to create an invalid capability selector.
   * \param cap  Capability selector.
   */
  Cap(Cap_type cap) noexcept : Cap_base(cap) {}

  /**
   * \brief Initialize capability with one of the default capability selectors.
   * \param cap  Capability selector.
   */
  Cap(l4_default_caps_t cap) noexcept : Cap_base(cap) {}

  /**
   * \brief Initialize capability, defaults to the invalid capability selector.
   * \param idx  Capability selector.
   */
  explicit Cap(l4_cap_idx_t idx = L4_INVALID_CAP) noexcept : Cap_base(idx) {}

  /**
   * \brief Create an uninitialized cap selector.
   */
  explicit Cap(No_init_type) noexcept {}

  /**
   * \brief Move a capability to this cap slot.
   * \param src the source capability slot.
   *
   * After this operation the source slot is no longer valid.
   */
  Cap move(Cap const &src) const
  {
    Cap_base::move(src);
    return *this;
  }

  /**
   * \brief Copy a capability to this cap slot.
   * \param src the source capability slot.
   */
  Cap copy(Cap const &src) const
  {
    Cap_base::copy(src);
    return *this;
  }

  /**
   * \brief Member access of a `T`.
   */
  T *operator -> () const noexcept { return reinterpret_cast<T*>(_c); }
};


/**
 * \internal
 * \brief Specialization for `void` capabilities.
 *
 * Include
 *
 *     #include <l4/sys/capability>
 *
 * to use this.
 */
template<>
class L4_EXPORT Cap<void> : public Cap_base
{
public:

  explicit Cap(void const *p) noexcept
  : Cap_base(reinterpret_cast<l4_cap_idx_t>(p)) {}

  /**
   * \brief Constructor to create an invalid capability selector.
   */
  Cap(Cap_type cap) noexcept : Cap_base(cap) {}

  /**
   * \brief Initialize capability with one of the default capability selectors.
   * \param cap  Capability selector.
   */
  Cap(l4_default_caps_t cap) noexcept : Cap_base(cap) {}

  /**
   * \brief Initialize capability, defaults to the invalid capability selector.
   * \param idx  Capability selector.
   */
  explicit Cap(l4_cap_idx_t idx = L4_INVALID_CAP) noexcept : Cap_base(idx) {}
  explicit Cap(No_init_type) noexcept {}

  template< typename From >
  static void check_convertible_from() noexcept {}

  template< typename From >
  static void check_castable_from() noexcept {}

  /**
   * \brief Move a capability to this cap slot.
   * \param src the source capability slot.
   *
   * After this operation the source slot is no longer valid.
   */
  Cap move(Cap const &src) const
  {
    Cap_base::move(src);
    return *this;
  }

  /**
   * \brief Copy a capability to this cap slot.
   * \param src the source capability slot.
   */
  Cap copy(Cap const &src) const
  {
    Cap_base::copy(src);
    return *this;
  }

  template< typename T >
  Cap(Cap<T> const &o) noexcept : Cap_base(o.cap()) {}
};

/**
 * \brief static_cast for capabilities.
 * \tparam T  The target type of the capability
 * \tparam F  The source type (and is usually implicitly set)
 * \param c   The source capability that shall be casted
 * \return A capability typed to the interface `T`.
 *
 * The use of this cast operator is similar to the `static_cast<>()` for
 * C++ pointers.  It does the same type checking and adjustments like
 * C++ does on pointers.
 *
 * Example code:
 *
 *     L4::Cap<L4::Kobject> obj = ... ;
 *     L4::Cap<L4::Icu> icu = L4::cap_cast<L4::Icu>(obj);
 */
template< typename T, typename F >
inline
Cap<T> cap_cast(Cap<F> const &c) noexcept
{
  Cap<T>::template check_castable_from<F>();
  return Cap<T>(c.cap());
}

// gracefully deal with L4::Kobject ambiguity
template< typename T >
inline
Cap<T> cap_cast(Cap<L4::Kobject> const &c) noexcept
{
  return Cap<T>(c.cap());
}

/**
 * reinterpret_cast for capabilities.
 * \tparam T  The target type of the capability
 * \tparam F  The source type (and is usually implicitly set)
 * \param c   The source capability that shall be casted
 * \return A capability typed to the interface `T`.
 *
 * The use of this cast operator is similar to the `reinterpret_cast<>()` for
 * C++ pointers.  It does not do any type checking or type adjustment.
 *
 * Example code:
 *
 *     L4::Cap<L4::Kobject> obj = ... ;
 *     L4::Cap<L4::Icu> icu = L4::cap_reinterpret_cast<L4::Icu>(obj);
 */
template< typename T, typename F >
inline
Cap<T> cap_reinterpret_cast(Cap<F> const &c) noexcept
{
  return Cap<T>(c.cap());
}

}
