/*! 
 * \file
 * \brief   base 64 encoding and decoding functions
 *          adapted from  Bob Trower 08/04/01
 *
 * \date    04/26/2002
 * \author  Joerg Nothnagel <jn6@os.inf.tu-dresden.de>
 */
/*
 * (c) 2008-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#ifndef B64_EN_DECODE
#define B64_EN_DECODE

#include <l4/sys/compiler.h>
#include <stddef.h>

L4_BEGIN_DECLS

/**
 * \defgroup l4util_internal Internal functions
 * \ingroup l4util_api
 */
/**@{*/

/*!
 * base-64-encode string \a in_data.
 * \internal
 *
 * \param in_data        Data to be encoded.
 * \param in_size        Length of \a in_data.
 * \param[out] out_data  The base-64-encoded representation of \a in_data.
 * \param[out] out_size  The size of the data in \a out_data.
 *
 * base-64-encode string \a in_data adding padding as per spec.
 */
L4_CV void base64_encode(char const *in_data, size_t in_size, char **out_data,
                         size_t *out_size);

/*!
 * decode base-64-encoded string \a in_data.
 * \internal
 *
 * \param in_data        String to be decoded.
 * \param in_size        Length of \a in_data.
 * \param[out] out_data  The decoded representation of \a in_data.
 * \param[out] out_size  The size of the data in \a out_data.
 *
 * base-64-decode string \a in_data.
 */
L4_CV void base64_decode(char const *in_data, size_t in_size, char **out_data,
                         size_t *out_size);

L4_END_DECLS

/**@}*/
#endif //B64_EN_DECODE
