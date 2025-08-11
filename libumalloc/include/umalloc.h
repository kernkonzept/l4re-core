/*
 * Copyright (C) 2025 Kernkonzept GmbH.
 * Author(s): Martin Decky <martin.decky@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

/**
 * \file
 * \brief Public API for the basic next-fit memory allocator
 *
 * The user of this allocator is required to provide the implementation of the
 * #umalloc_area_create() function and provide the #umalloc_area_granularity
 * symbol with a value.
 *
 * \note The current implementation is NOT thread-safe. If the user wants to
 *       use this allocator concurrently, they need to deploy their custom
 *       mutual exclusion mechanism around the public calls to the allocator.
 */

#pragma once

#include <stddef.h>
#include <l4/sys/compiler.h>

L4_BEGIN_DECLS

/**
 * Heap area granularity.
 *
 * The allocator only requests the creation of heap areas with this granularity.
 */
extern size_t umalloc_area_granularity;

/**
 * Create a new heap area.
 *
 * \param area_size  Requested heap area size. Always a multiple of the heap
 *                   area granularity as reported by
 *                   #umalloc_area_granularity().
 *
 * \return Pointer to the successfully created heap area of (at least) the
 *         requested size. It is assumed that the pointer satisfies at least
 *         the #umalloc:Base_alignment alignment. If the value is nullptr, then
 *         it is assumed that the creation of the heap area failed.
 */
void *umalloc_area_create(size_t area_size) L4_NOTHROW;

L4_END_DECLS
