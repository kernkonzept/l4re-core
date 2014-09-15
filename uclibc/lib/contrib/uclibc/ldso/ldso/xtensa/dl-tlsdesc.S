/* Thread-local storage handling in the ELF dynamic linker.  Xtensa version.
   Copyright (C) 2012-2013 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <tls.h>
#include "tlsdesc.h"


	.text
	.align 4
	.hidden _dl_tlsdesc_return
	.global	_dl_tlsdesc_return
	.type	_dl_tlsdesc_return, @function
_dl_tlsdesc_return:
	entry	a1, 16
	rur.threadptr	a3
	add		a2, a2, a3
	retw
	.size	_dl_tlsdesc_return, .-_dl_tlsdesc_return

#ifdef SHARED


	/* This function is used for symbols that need dynamic TLS.

	   The argument passed to this function points to the TLS descriptor.

	   The assembly code that follows is a rendition of the following
	   C code, hand-optimized a little bit.

	   ptrdiff_t
	   _dl_tlsdesc_dynamic(struct tlsdesc_dynamic_arg *td)
	   {
	     dtv_t *dtv = (dtv_t *)THREAD_DTV();
	     if (td->gen_count <= dtv[0].counter
	         && dtv[td->tlsinfo.ti_module].pointer.val
	            != TLS_DTV_UNALLOCATED)
	       return dtv[td->tlsinfo.ti_module].pointer.val
	              + td->tlsinfo.ti_offset - __builtin_thread_pointer();
	     return __tls_get_addr (&td->tlsinfo) - __builtin_thread_pointer();
	   }
	 */

	.align 4
	.hidden _dl_tlsdesc_dynamic
	.global	_dl_tlsdesc_dynamic
	.type	_dl_tlsdesc_dynamic, @function
_dl_tlsdesc_dynamic:
	entry	a1, 32

	/* dtv_t *dtv = (dtv_t *)THREAD_DTV(); */
	rur.threadptr	a3
	l32i	a4, a3, 0

	/* if (td->gen_count <= dtv[0].counter */
	l32i	a6, a2, TLSDESC_GEN_COUNT
	l32i	a7, a4, 0
	blt	a7, a6, .Lslow

	/* && dtv[td->tlsinfo.ti_module].pointer.val != TLS_DTV_UNALLOCATED) */
	l32i	a6, a2, TLSDESC_MODID
	addx8	a6, a3, a6
	l32i	a6, a6, 0
	beqi	a6, -1, .Lslow

	/* return dtv[td->tlsinfo.ti_module].pointer.val
	     + td->tlsinfo.ti_offset - __builtin_thread_pointer(); */
	l32i	a6, a2, TLSDESC_MODOFF
	sub	a2, a6, a3
	retw

	/* return __tls_get_addr (&td->tlsinfo) - __builtin_thread_pointer(); */
.Lslow:
	mov	a10, a2
	movi	a8, __tls_get_addr
	callx8	a8
	sub	a2, a10, a3
	retw
	.size	_dl_tlsdesc_dynamic, .-_dl_tlsdesc_dynamic

#endif /* SHARED */
