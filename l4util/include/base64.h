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
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#ifndef B64_EN_DECODE
#define B64_EN_DECODE

#include <l4/sys/compiler.h>

__BEGIN_DECLS

/**
 * \defgroup l4util_internal Internal functions
 * \ingroup l4util_api
 */
/**@{*/

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

__END_DECLS

/**@}*/
#endif //B64_EN_DECODE
