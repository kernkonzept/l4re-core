/**
 * \file
 * \brief   Linkage
 * \ingroup l4sys_api
 */
/* SPDX-License-Identifier: ((GPL-2.0-only WITH mif-exception) OR LicenseRef-kk-custom) */
/*
 * Copyright (C) 2021 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 */
#pragma once

#ifdef __ASSEMBLY__

#ifndef ENTRY
#define ENTRY(name) \
  .globl name; \
  .p2align(2); \
  name:

#endif /* ! ENTRY */
#endif /* __ASSEMBLY__ */

/**
 * Define calling convention.
 * \ingroup l4sys_defines
 * \hideinitializer
 */
#define L4_CV

#define L4_FASTCALL(x)  x
#define l4_fastcall