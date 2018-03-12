/*! 
 * \file
 * \brief   base 64 encoding and decoding functions
 *          adapted from  Bob Trower 08/04/01
 * \ingroup utils
 *
 * \date    04/26/2002
 * \author  Joerg Nothnagel <jn6@os.inf.tu-dresden.de>
 */
/*
 * (c) 2008-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#ifndef B64_EN_DECODE
#define B64_EN_DECODE

#include <l4/sys/compiler.h>

EXTERN_C_BEGIN

/**
 * \defgroup l4util_internal Internal functions
 * \ingroup l4util_api
 */
/*@{*/

/*!
 * \brief base-64-encode string \a infile
 * \internal
 *
 * \param infile string to be encoded
 * \param in_size length of \a infile 
 * \retval outfile the base-64-encoded representation of \a infile
 *
 *  base-64-encode string \a infile adding padding as per spec
 */
L4_CV void base64_encode( const char *infile, unsigned int in_size, char **outfile);

/*!
 * \brief decode base-64-encoded string \a infile
 * \internal
 *
 * \param infile string to be decoded
 * \param in_size length of \a infile 
 * \retval outfile the decoded representation of \a infile
 *
 *  base-64-decode string \a infile discarding padding, line breaks and noise
 */
L4_CV void base64_decode(const char *infile, unsigned int in_size, char **outfile);

EXTERN_C_END

/*@}*/
#endif //B64_EN_DECODE
