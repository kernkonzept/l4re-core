/* @(#)xdr_float.c	2.1 88/07/29 4.0 RPCSRC */
/*
 * Sun RPC is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify Sun RPC without charge, but are not authorized
 * to license or distribute it to anyone else except as part of a product or
 * program developed by the user.
 *
 * SUN RPC IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 *
 * Sun RPC is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 *
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY SUN RPC
 * OR ANY PART THEREOF.
 *
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 *
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */
#if 0
static char sccsid[] = "@(#)xdr_float.c 1.12 87/08/11 Copyr 1984 Sun Micro";
#endif

/*
 * xdr_float.c, Generic XDR routines implementation.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 *
 * These are the "floating point" xdr routines used to (de)serialize
 * most common data items.  See xdr.h for more info on the interface to
 * xdr.
 */

#include <stdio.h>
#include <endian.h>

#include <rpc/types.h>
#include <rpc/xdr.h>

/*
 * NB: Not portable.
 * This routine works on Suns (Sky / 68000's) and Vaxen.
 */

#define LSW	(__FLOAT_WORD_ORDER == __BIG_ENDIAN)

bool_t
xdr_float(XDR *xdrs, float *fp)
{
	switch (xdrs->x_op) {

	case XDR_ENCODE:
		if (sizeof(float) == sizeof(long))
			return (XDR_PUTLONG(xdrs, (long *)fp));
		else if (sizeof(float) == sizeof(int)) {
			long tmp = *(int *)fp;
			return (XDR_PUTLONG(xdrs, &tmp));
		}
		break;

	case XDR_DECODE:
		if (sizeof(float) == sizeof(long))
			return (XDR_GETLONG(xdrs, (long *)fp));
		else if (sizeof(float) == sizeof(int)) {
			long tmp;
			if (XDR_GETLONG(xdrs, &tmp)) {
				*(int *)fp = tmp;
				return (TRUE);
			}
		}
		break;

	case XDR_FREE:
		return (TRUE);
	}
	return (FALSE);
}

bool_t
xdr_double(XDR *xdrs, double *dp)
{

	switch (xdrs->x_op) {

	case XDR_ENCODE:
		if (2*sizeof(long) == sizeof(double)) {
			long *lp = (long *)dp;
			return (XDR_PUTLONG(xdrs, lp+!LSW) &&
				XDR_PUTLONG(xdrs, lp+LSW));
		} else if (2*sizeof(int) == sizeof(double)) {
			int *ip = (int *)dp;
			long tmp[2];
			tmp[0] = ip[!LSW];
			tmp[1] = ip[LSW];
			return (XDR_PUTLONG(xdrs, tmp) &&
				XDR_PUTLONG(xdrs, tmp+1));
		}
		break;

	case XDR_DECODE:
		if (2*sizeof(long) == sizeof(double)) {
			long *lp = (long *)dp;
			return (XDR_GETLONG(xdrs, lp+!LSW) &&
				XDR_GETLONG(xdrs, lp+LSW));
		} else if (2*sizeof(int) == sizeof(double)) {
			int *ip = (int *)dp;
			long tmp[2];
			if (XDR_GETLONG(xdrs, tmp+!LSW) &&
			    XDR_GETLONG(xdrs, tmp+LSW)) {
				ip[0] = tmp[0];
				ip[1] = tmp[1];
				return (TRUE);
			}
		}
		break;

	case XDR_FREE:
		return (TRUE);
	}
	return (FALSE);
}
