/**
 * \file   dietlibc/lib/backends/simple_sleep/sleep.c
 * \brief  
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/*
 * (c) 2004-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#include <errno.h>
#include <sys/time.h>
#include <time.h>

#include <l4/util/util.h>

int nanosleep(const struct timespec *req, struct timespec *rem)
{
    int milis;

    (void)rem;
    if (req == NULL)
    {
        errno = EFAULT; // or maybe EINVAL ???
        return -1;
    }

    if (req->tv_nsec < 0 || req->tv_nsec > 999999999 || req->tv_sec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    milis = (req->tv_sec * 1000) + (req->tv_nsec / 1000000);
    l4_sleep(milis);

    return 0;
}
