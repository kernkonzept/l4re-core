/* OpenRISC 1000 shared library loader suppport
 *
 * Copyright (C) 2012 Stefan Kristansson
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. The name of the above contributors may not be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

static const char * const _dl_reltypes_tab[] =
	{
		"R_OR1K_NONE",
		"R_OR1K_32",
		"R_OR1K_16",
		"R_OR1K_8",
		"R_OR1K_LO_16_IN_INSN",
		"R_OR1K_HI_16_IN_INSN",
		"R_OR1K_INSN_REL_26",
		"R_OR1K_GNU_VTENTRY",
		"R_OR1K_GNU_VTINHERIT",
		"R_OR1K_32_PCREL",
		"R_OR1K_16_PCREL",
		"R_OR1K_8_PCREL",
		"R_OR1K_GOTPC_HI16",
		"R_OR1K_GOTPC_LO16",
		"R_OR1K_GOT16",
		"R_OR1K_PLT26",
		"R_OR1K_GOTOFF_HI16",
		"R_OR1K_GOTOFF_LO16",
		"R_OR1K_COPY",
		"R_OR1K_GLOB_DAT",
		"R_OR1K_JMP_SLOT",
		"R_OR1K_RELATIVE",
	};
