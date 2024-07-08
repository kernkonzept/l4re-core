/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

/* Type used for the representation of TLS information in the GOT.  */
typedef struct
{
	unsigned long int ti_module;
	unsigned long int ti_offset;
} tls_index;

extern void *__tls_get_addr (tls_index *ti);

/* Value used for dtv entries for which the allocation is delayed.  */
#define TLS_DTV_UNALLOCATED    ((void *) -1l)
