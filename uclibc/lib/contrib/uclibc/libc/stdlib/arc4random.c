/*
 * Copyright (c) 1996, David Mazieres <dm@uun.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Arc4 random number generator for OpenBSD.
 *
 * This code is derived from section 17.1 of Applied Cryptography,
 * second edition, which describes a stream cipher allegedly
 * compatible with RSA Labs "RC4" cipher (the actual description of
 * which is a trade secret).  The same algorithm is used as a stream
 * cipher called "arcfour" in Tatu Ylonen's ssh package.
 *
 * Here the stream cipher has been modified always to include entropy
 * when initializing the state.  That makes it impossible to
 * regenerate the same random sequence twice, so this can't be used
 * for encryption, but will generate good random numbers.
 *
 * RC4 is a registered trademark of RSA Laboratories.
 */

/*	$OpenBSD: arc4random.c,v 1.16 2007/02/12 19:58:47 otto Exp $	*/

#include <features.h>

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

struct arc4_stream {
	u_int8_t i;
	u_int8_t j;
	u_int8_t s[256];
};

static smallint rs_initialized;
static struct arc4_stream rs;
static pid_t arc4_stir_pid;
static int arc4_count;

static __inline__ void
arc4_init(struct arc4_stream *as)
{
	int     n;

	for (n = 0; n < 256; n++)
		as->s[n] = n;
	as->i = 0;
	as->j = 0;
}

static __inline__ u_int8_t
arc4_getbyte(struct arc4_stream *as)
{
	u_int8_t si, sj;

	as->i = (as->i + 1);
	si = as->s[as->i];
	as->j = (as->j + si);
	sj = as->s[as->j];
	as->s[as->i] = sj;
	as->s[as->j] = si;
	return (as->s[(si + sj) & 0xff]);
}

static __inline__ void
arc4_addrandom(struct arc4_stream *as, u_char *dat, int datlen)
{
	int     n;
	u_int8_t si;

	as->i--;
	for (n = 0; n < 256; n++) {
		as->i = (as->i + 1);
		si = as->s[as->i];
		as->j = (as->j + si + dat[n % datlen]);
		as->s[as->i] = as->s[as->j];
		as->s[as->j] = si;
	}
	as->j = as->i;
}

static void
arc4_stir(struct arc4_stream *as)
{
	int	n;
	u_char	rnd[128];
	struct timeval tv;

#ifndef __ARC4RANDOM_USES_NODEV__
	int	fd;

	fd = open("/dev/urandom", O_RDONLY);
	if (fd != -1) {
		read(fd, rnd, sizeof(rnd));
		close(fd);
	}
	/* Did the pseudo-random device fail? Use gettimeofday(). */
	else
#endif
	if (gettimeofday(&tv, NULL) != (-1)) {

		/* Initialize the first element so it's hopefully not '0',
		 * to help out the next loop. Tossing in some prime numbers
		 * probably can't hurt. */
		rnd[0] = (tv.tv_sec % 10000) * 3 + tv.tv_usec * 7 + \
			(getpid() % 1000) * 13;

		for (n = 1; n < 127 ; n++) {

		/* Take advantage of the stack space. Only initialize
		 * elements equal to '0'. This will make the rnd[]
		 * array much less vulnerable to timing attacks. Here
		 * we'll stir getpid() into the value of the previous
		 * element. Approximately 1 in 128 elements will still
		 * become '0'. */

			if (rnd[n] == 0) {
				rnd[n] = ((rnd[n - 1] + n) ^ \
					((getpid() % 1000) * 17));
			}
		}
	}
	else {
	/* gettimeofday() failed? Do the same thing as above, but only
	 * with getpid(). */

		rnd[0] = (getpid() % 1000) * 19;
		for (n = 1; n < 127 ; n++) {
			if (rnd[n] == 0) {
				rnd[n] = ((rnd[n - 1] + n) ^ \
					((getpid() % 1000) * 23));
			}
		}
	}

	arc4_stir_pid = getpid();
	arc4_addrandom(as, rnd, sizeof(rnd));

	/*
	 * Discard early keystream, as per recommendations in:
	 * http://www.wisdom.weizmann.ac.il/~itsik/RC4/Papers/Rc4_ksa.ps
	 */
	for (n = 0; n < 256; n++)
		(void)arc4_getbyte(as);
	arc4_count = 1600000;
}

#if 0
static void __arc4random_stir(void);
/*
 * __arc4_getbyte() is a libc private function intended for use
 * with malloc.
 */
u_int8_t
__arc4_getbyte(void)
{
	if (--arc4_count == 0 || !rs_initialized)
		__arc4random_stir();
	return arc4_getbyte(&rs);
}
#endif

static __inline__ u_int32_t
arc4_getword(struct arc4_stream *as)
{
	u_int32_t val;
	val = arc4_getbyte(as) << 24;
	val |= arc4_getbyte(as) << 16;
	val |= arc4_getbyte(as) << 8;
	val |= arc4_getbyte(as);
	return val;
}

static void
__arc4random_stir(void)
{
	if (!rs_initialized) {
		arc4_init(&rs);
		rs_initialized = 1;
	}
	arc4_stir(&rs);
}
strong_alias(__arc4random_stir,arc4random_stir)

void
arc4random_addrandom(u_char *dat, int datlen)
{
	if (!rs_initialized)
		__arc4random_stir();
	arc4_addrandom(&rs, dat, datlen);
}

u_int32_t
arc4random(void)
{
	arc4_count -= 4;
	if (arc4_count <= 0 || !rs_initialized || arc4_stir_pid != getpid())
		__arc4random_stir();
	return arc4_getword(&rs);
}
