/**
 * \file
 * \brief Auxiliary information for binaries
 */
/*
 * (c) 2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */
#pragma once

#include <l4/sys/types.h>


/**
 * \defgroup api_l4re_elf_aux L4Re ELF Auxiliary Information
 * \ingroup api_l4re
 * \brief API for embedding auxiliary information into
 *        binary programs.
 *
 * This API allows information for the binary loader to be embedded into a
 * binary application.  This information can be reserved areas in the virtual
 * memory of an application and things such as the stack size to be allocated
 * for the first application thread.
 */
/*@{*/

/**
 * \brief Define an auxiliary vector element.
 *
 * This is the generic method for defining auxiliary vector elements.
 * A more convenient way is to use L4RE_ELF_AUX_ELEM_T.
 *
 * Usage:
 * \code
 * L4RE_ELF_AUX_ELEM l4re_elf_aux_vma_t decl_name =
 *   { L4RE_ELF_AUX_T_VMA, sizeof(l4re_elf_aux_vma_t), 0x2000, 0x4000 };
 * \endcode
 */
#define L4RE_ELF_AUX_ELEM const __attribute__((used, section(".rol4re_elf_aux"), aligned(sizeof(l4_umword_t))))

/**
 * \brief Define an auxiliary vector element.
 * \param type is the data type for the element (e.g., l4re_elf_aux_vma_t)
 * \param id is the identifier (variable name) for the declaration (the
 *           variable is defined with \c static storage class)
 * \param tag is the tag value for the element e.g., #L4RE_ELF_AUX_T_VMA
 * \param val are the values to be set in the descriptor
 *
 * Usage:
 * \code
 * L4RE_ELF_AUX_ELEM_T(l4re_elf_aux_vma_t, decl_name, L4RE_ELF_AUX_T_VMA, 0x2000, 0x4000 };
 * \endcode
 */
#define L4RE_ELF_AUX_ELEM_T(type, id, tag, val...) \
  static L4RE_ELF_AUX_ELEM type id = {tag, sizeof(type), val}

enum
{
  /**
   * \brief Tag for an invalid element in the auxiliary vector
   */
  L4RE_ELF_AUX_T_NONE = 0,

  /**
   * \brief Tag for descriptor for a reserved virtual memory area.
   */
  L4RE_ELF_AUX_T_VMA,

  /**
   * \brief Tag for descriptor that defines the stack size for
   *        the first application thread.
   */
  L4RE_ELF_AUX_T_STACK_SIZE,

  /**
   * \brief Tag for descriptor that defines the stack address
   *        for the first application thread.
   */
  L4RE_ELF_AUX_T_STACK_ADDR,

  /**
   * \brief Tag for descriptor that defines the KIP address
   *        for the binaries address space.
   */
  L4RE_ELF_AUX_T_KIP_ADDR,
};

/**
 * \brief Generic header for each auxiliary vector element.
 */
typedef struct l4re_elf_aux_t
{
  l4_umword_t type;
  l4_umword_t length;
} l4re_elf_aux_t;

/**
 * \brief Auxiliary vector element for a reserved virtual memory area.
 */
typedef struct l4re_elf_aux_vma_t
{
  l4_umword_t type;
  l4_umword_t length;
  l4_umword_t start;
  l4_umword_t end;
} l4re_elf_aux_vma_t;

/**
 * \brief Auxiliary vector element for a single unsigned data word.
 */
typedef struct l4re_elf_aux_mword_t
{
  l4_umword_t type;
  l4_umword_t length;
  l4_umword_t value;
} l4re_elf_aux_mword_t;

/*@}*/
