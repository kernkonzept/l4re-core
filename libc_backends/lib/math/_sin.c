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
 * License: see LICENSE.spdx (in this directory or the directories above)
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
