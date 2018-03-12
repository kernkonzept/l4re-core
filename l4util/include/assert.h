/*****************************************************************************/
/**
 * \file
 * \brief   Some useful assert-style macros.
 *
 * \date    09/2009
 * \author  Bjoern Doebel <doebel@tudos.org>
 */
/*
 * (c) 2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

/*****************************************************************************/
#pragma once

#ifdef NDEBUG

#define DO_NOTHING              do {} while (0)
#define ASSERT_VALID(c)         DO_NOTHING
#define ASSERT_EQUAL(a,b)       DO_NOTHING
#define ASSERT_NOT_EQUAL(a,b)   DO_NOTHING
#define ASSERT_LOWER_EQ(a,b)    DO_NOTHING
#define ASSERT_GREATER_EQ(a,b)  DO_NOTHING
#define ASSERT_BETWEEN(a,b,c)   DO_NOTHING
#define ASSERT_IPC_OK(i)        DO_NOTHING
#define ASSERT_OK(e)            do { (void)e; } while (0)
#define ASSERT_NOT_NULL(p)      DO_NOTHING
#ifndef assert
#define assert(cond)            DO_NOTHING
#endif

#else // NDEBUG

#ifndef ASSERT_PRINTF
#include <stdio.h>
#define ASSERT_PRINTF printf
#endif
#ifndef ASSERT_ASSERT
#include <assert.h>
#define ASSERT_ASSERT(x) assert(x)
#endif

#define ASSERT_VALID(cap) \
	do { \
		typeof(cap) _cap = cap; \
		if (l4_is_invalid_cap(_cap)) { \
			ASSERT_PRINTF("%s: Cap invalid.\n", __func__); \
			ASSERT_ASSERT(!l4_is_invalid_cap(_cap)); \
		} \
	} while (0)


#define ASSERT_EQUAL(a, b) \
	do { \
		typeof(a) _a = a; \
		typeof(b) _b = b; \
		if (_a != _b) { \
			ASSERT_PRINTF("%s:\n", __func__); \
			ASSERT_PRINTF("    "#a" (%lx) != "#b" (%lx)\n", (unsigned long)_a, (unsigned long)_b); \
			ASSERT_ASSERT(_a == _b); \
		} \
	} while (0)


#define ASSERT_NOT_EQUAL(a, b) \
	do { \
		typeof(a) _a = a; \
		typeof(b) _b = b; \
		if (_a == _b) { \
			ASSERT_PRINTF("%s:\n", __func__); \
			ASSERT_PRINTF("    "#a" (%lx) == "#b" (%lx)\n", (unsigned long)_a, (unsigned long)_b); \
			ASSERT_ASSERT(_a != _b); \
		} \
	} while (0)


#define ASSERT_LOWER_EQ(val, max) \
	do { \
		typeof(val) _val = val; \
		typeof(max) _max = max; \
		if (_val > _max) { \
			ASSERT_PRINTF("%s:\n", __func__); \
			ASSERT_PRINTF("    "#val" (%lx) > "#max" (%lx)\n", (unsigned long)_val, (unsigned long)_max); \
			ASSERT_ASSERT(_val <= _max); \
		} \
	} while (0)


#define ASSERT_GREATER_EQ(val, min) \
	do { \
		typeof(val) _val = val; \
		typeof(min) _min = min; \
		if (_val < _min) { \
			ASSERT_PRINTF("%s:\n", __func__); \
			ASSERT_PRINTF("    "#val" (%lx) < "#min" (%lx)\n", (unsigned long)_val, (unsigned long)_min); \
			ASSERT_ASSERT(_val >= _min); \
		} \
	} while (0)


#define ASSERT_BETWEEN(val, min, max) \
	ASSERT_LOWER_EQ((val), (max)); \
	ASSERT_GREATER_EQ((val), (min));


#define ASSERT_IPC_OK(msgtag) \
	do { \
		int _r = l4_ipc_error(msgtag, l4_utcb()); \
		if (_r) { \
			ASSERT_PRINTF("%s: IPC Error: %lx\n", __func__, _r); \
			ASSERT_ASSERT(_r == 0); \
		} \
	} while (0)

#define ASSERT_OK(val)         ASSERT_EQUAL((val), 0)
#define ASSERT_NOT_NULL(ptr)   ASSERT_NOT_EQUAL((ptr), (void *)0)

#endif // NDEBUG
