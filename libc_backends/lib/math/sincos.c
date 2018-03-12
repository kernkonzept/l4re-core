/**
 * \file   sincos.c
 * \brief  Provide a sincos and sincosf implementation, used implicitly by gcc
 *
 * \date   2008-05-27
 * \author Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *
 */
/*
 * (c) 2008-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#include <math.h>

/* The following is a tiny bit tricky, uclibc does not really provide
 * sincos. Additionally, gcc has an optimization that merges close calls of
 * sin and cos to sincos, i.e. sincos in here would recurse if we'd just use
 * sin and cos. So we trick gcc by using an additional .o-file */

void sincos(double x, double *s, double *c);
void sincosf(float x, float *s, float *c);

double _sin(double x);
float  _sinf(float x);

void sincos(double x, double *s, double *c)
{
  *s = _sin(x);
  *c = cos(x);
}

void sincosf(float x, float *s, float *c)
{
  *s = _sinf(x);
  *c = cosf(x);
}
