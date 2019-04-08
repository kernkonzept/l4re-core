/*
 * Copyright (C) 2019 Kernkonzept GmbH.
 * Author(s): Jakub Jermar <jakub.jermar@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2. Please see the COPYING-GPL-2 file for details.
 */

.data

.align 4

.global execute_data
execute_data:
	mov pc, lr
execute_data_end:

.global execute_data_size
execute_data_size:
	.long execute_data_end - execute_data


