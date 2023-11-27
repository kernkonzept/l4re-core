/*
 * Copyright (C) 2018 Kernkonzept GmbH.
 * Author(s): Adam Lackorzynski <adam@l4re.org>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <stdlib.h>

char *secure_getenv(const char *name);
char *secure_getenv(const char *name)
{
  return getenv(name);
}
