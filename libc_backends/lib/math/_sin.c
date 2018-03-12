/**
 * \file   _sin.c
 * \brief  sin() wrapper
 *
 * \date   2009-10
 * \author Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *
 */
/*
 * (c) 2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#include <math.h>

double _sin(double x);
double _sin(double x)
{
  return sin(x);
}

float _sinf(float x);
float _sinf(float x)
{
  return sinf(x);
}
