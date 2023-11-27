/*
 * Copyright (C) 2009 TU Dresden.
 * Author(s): Adam Lackorzynski <adam@l4re.org>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <stdio.h>
#include <unistd.h>

char *getpass( const char *prompt)
{
  printf("This would be the prompt: '%s', delivering something static\n",
         prompt);
  return "THE FAMOUS PASSWORD";
}
