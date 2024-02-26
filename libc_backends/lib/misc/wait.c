/*
 * Copyright (C) 2024 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

pid_t wait(int *status)
{
  printf("Unimplemented: %s(%p)\n", __func__, status);
  errno = ECHILD;
  return -1;
}
