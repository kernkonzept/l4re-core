/**
 * \file
 * \brief Perfomance Monitoring using P5/P6 Measurement Counters.
 *
 * Define either CPU_PENTIUM or CPU_P6
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Frank Mehnert <fm3@os.inf.tu-dresden.de>,
 *               Lars Reuther <reuther@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#ifndef __L4UTIL_PERFORM_H
#define __L4UTIL_PERFORM_H

#include <l4/sys/types.h>
#include <l4/sys/compiler.h>

EXTERN_C_BEGIN

extern const char*strp6pmc_event(l4_uint32_t event);

#ifndef CONFIG_PERFORM_ONLY_PROTOTYPES

#if ! (defined CPU_PENTIUM  ^ defined CPU_P6 ^ defined CPU_K7)

#error You must define your target architecture.
#error Define EITHER CPU_PENTIUM for Intel Pentium or CPU_P6 for Intel PPro/PII/PIII.

#else

/* P5/P6/K7 section */

/* Makros for access to model specific registers (MSR) */

/* Write the 64-Bit Model Specific Register. First argument is the register,
   second the 64-Bit value. This can only be called at priviledge level 0.
   With L4, the kernel emulates the WRMSR when calling in PL 3.
   */
static inline void l4_i586_wrmsr(unsigned reg,unsigned long long*val){
  unsigned long dummyeax, dummyecx, dummyedx;

  asm volatile(
        ".byte 0xf; .byte 0x30\n"	/* wrmsr */
	: "=a" (dummyeax), "=d" (dummyedx), "=c" (dummyecx)
	: "2" (reg), "0" (*(unsigned *)val), "1" (*((unsigned *)val+1))
	);
}

/* Read the 64-Bit Model Specific Register. First argument is the register,
   second the address to a 64-Bit value. This can only be called at
   priviledge level 0.  With L4, the kernel emulates the RDMSR when calling
   in PL 3.
   */
static inline void l4_i586_rdmsr(unsigned reg,unsigned long long*val){
  unsigned dummy;

  asm volatile(
        ".byte 0xf; .byte 0x32\n"	/* rdmsr */
	: "=a" (*(unsigned *)val), "=d" (*((unsigned *)val+1)), "=c" (dummy)
	: "2" (reg)
	);
}


#ifdef CPU_PENTIUM
/* Pentium section */

/* functions and events defined here are only usable at Pentium
   Processors. P6 architecture does NOT support this kind of measuring and
   these events. P6 architecture has its own counters and its own events.
   See P6-section for details. */

/* from l4linux/arch/l4-i386/include/perform.h */

static inline void 
l4_i586_reset_event_counter(void){
   asm volatile("xor %%eax, %%eax\n"
		"xor %%edx, %%edx\n"
		"movl $0x12, %%ecx\n"
		".byte 0x0f, 0x30\n"
		"movl $0x13, %%ecx\n"
		".byte 0x0f, 0x30\n"
		: : : "cx", "ax", "dx" 
		);
};

static inline void
l4_i586_read_event_counter_long(long long *counter0, long long *counter1)
{
  asm volatile(
	       /*	       "movl	$0, %%eax\n"
	       "movl	$0x11, %%ecx\n"
	       ".byte 0x0f, 0x30\n" *//* stop event counting */
	       "movl $0x12, %%ecx\n"
	       ".byte 0x0f, 0x32\n"
	       "movl %%eax, (%%ebx)\n"
	       "movl %%edx, 4(%%ebx)\n"
	       "movl $0x13, %%ecx\n"
	       ".byte 0x0f, 0x32\n"
	       "movl %%eax, (%%esi)\n"
	       "movl %%edx, 4(%%esi)\n"
	       : /* no output */
	       : "b" (counter0), "S" (counter1)
	       : "ax", "cx", "dx"
	       );
}

static inline void
l4_i586_read_event_counter(int *counter0, int *counter1)
{
  asm volatile("pushl	%%edx\n"
	       ".byte 0x0f, 0x30\n"
	       "movl $0x12, %%ecx\n"
	       ".byte 0x0f, 0x32\n"
	       "movl %%eax, %%ebx\n"
	       "movl $0x13, %%ecx\n"
	       ".byte 0x0f, 0x32\n"
	       "popl	%%edx\n"
	       : "=b" (*counter0), "=a" (*counter1)
	       : "1" (0), "c" (0x11)
	       );
}

static inline void 
l4_i586_select_event(int event0, int event1)
{
   asm volatile(".byte 0x0f, 0x30\n"
		:
		:
 		"a" (event0 + (event1 << 16)),
		"d" (0),
		"c" (0x11)
		);
};

#define P5_RD_MISS          0x003	/* 000011B */
#define P5_WR_MISS          0x008	/* 000100B */
#define P5_RW_MISS          0x029	/* 101001B */
#define P5_EX_MISS          0x00e	/* 001110B */

#define P5_D_WBACK          0x006	/* 000110B */

#define P5_RW_TLB           0x002	/* 00010B */
#define P5_EX_TLB           0x00d	/* 01101B */

#define P5_A_STALL          0x01f	/* 11111B */
#define P5_W_STALL          0x019	/* 11001B */
#define P5_R_STALL          0x01a	/* 11010B */
#define P5_X_STALL          0x01b	/* 11011B */

#define P5_AGI_STALL        0x01f	/* 11111B */

#define P5_PIPLINE_FLUSH    0x015	/* 10101B */

#define P5_NON_CACHE_RD     0x01e	/* 11110B */
#define P5_NCACHE_REFS      0x01e	/* 11110B */
#define P5_LOCKED_BUS       0x01c	/* 11100B */

#define P5_MEM2PIPE         0x009	/* 01001B */
#define P5_BANK_CONF        0x00a	/* 01010B */


#define P5_INSTRS_EX        0x016	/* 10110B */
#define P5_INSTRS_EX_V      0x017	/* 10111B */


#define P5_CNT_NOTHING      (0x00 << 6)	/* 00B << 6 */
#define P5_CNT_EVENT_PL0    (0x01 << 6)	/* 01B << 6 */
#define P5_CNT_EVENT_PL3    (0x02 << 6)	/* 10B << 6 */
#define P5_CNT_EVENT        (0x03 << 6)	/* 11B << 6 */
#define P5_CNT_CLOCKS_PL0   (0x05 << 6)	/* 101B << 6 */
#define P5_CNT_CLOCKS_PL3   (0x06 << 6)	/* 110B << 6 */
#define P5_CNT_CLOCKS       (0x07 << 6)	/* 111B << 6 */


#else
#if defined CPU_P6
/* PPro/PII/PIII section */

/*-
 * Copyright (c) 1997 The President and Fellows of Harvard College.
 * All rights reserved.
 * Copyright (c) 1997 Aaron B. Brown.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Harvard University
 *      and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY HARVARD AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL HARVARD UNIVERSITY OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*********************************************************************
 ** Symbolic names for counter numbers (used in select_p6counter()) **
 *********************************************************************
 *
 * These correspond in order to the Pentium Pro counters. Add new counters at
 * the end. These agree with the mneumonics in the Pentium Pro Family
 * Developer's Manual, vol 3.
 *
 * Those events marked with a $ require a MESI unit field; those marked with
 * a @ require a self/any unit field. Those marked with a 0 are only supported
 * in counter 0; those marked with 1 are only supported in counter 1.
 */

/* Data cache unit */
#define P6_DATA_MEM_REFS	0x43	/* total memory refs */
#define P6_DCU_LINES_IN		0x45	/* all lines allocated in cache unit */
#define P6_DCU_M_LINES_IN	0x46	/* M lines allocated in cache unit */
#define P6_DCU_M_LINES_OUT	0x47	/* M lines evicted from cache */
#define P6_DCU_MISS_OUTSTANDING	0x48	/* #cycles a miss is outstanding */

/* Instruction fetch unit */
#define P6_IFU_IFETCH		0x80	/* instruction fetches */
#define P6_IFU_IFETCH_MISS	0x81	/* instruction fetch misses */
#define P6_ITLB_MISS		0x85	/* ITLB misses */
#define P6_IFU_MEM_STALL	0x86	/* number of cycles IFU is stalled */
#define P6_ILD_STALL		0x87	/* #stalls in instr length decode */

/* L2 Cache */
#define P6_L2_IFETCH		0x28	/* ($) l2 ifetches */
#define P6_L2_LD		0x29	/* ($) l2 data loads */
#define P6_L2_ST		0x2a	/* ($) l2 data stores */
#define P6_L2_LINES_IN		0x24	/* lines allocated in l2 */
#define P6_L2_LINES_OUT		0x26	/* lines removed from l2 */
#define P6_L2_M_LINES_INM	0x25	/* modified lines allocated in L2 */
#define P6_L2_M_LINES_OUTM	0x27	/* modified lines removed from L2 */
#define P6_L2_RQSTS		0x2e	/* ($) number of l2 requests */
#define P6_L2_ADS		0x21	/* number of l2 addr strobes */
#define P6_L2_DBUS_BUSY		0x22	/* number of data bus busy cycles */
#define P6_L2_DBUS_BUSY_RD	0x23	/* #bus cycles xferring l2->cpu */

/* External bus logic */
#define P6_BUS_DRDY_CLOCKS	0x62	/* (@) #clocks DRDY is asserted */
#define P6_BUS_LOCK_CLOCKS	0x63	/* (@) #clocks LOCK is asserted */
#define P6_BUS_REQ_OUTSTANDING	0x60	/* #bus requests outstanding */
#define P6_BUS_TRAN_BRD		0x65	/* (@) bus burst read txns */
#define P6_BUS_TRAN_RFO		0x66	/* (@) bus read for ownership txns */
#define P6_BUS_TRAN_WB		0x67	/* (@) bus writeback txns */
#define P6_BUS_TRAN_IFETCH	0x68	/* (@) bus instr fetch txns */
#define P6_BUS_TRAN_INVAL	0x69	/* (@) bus invalidate txns */
#define P6_BUS_TRAN_PWR		0x6a	/* (@) bus partial write txns */
#define P6_BUS_TRANS_P		0x6b	/* (@) bus partial txns */
#define P6_BUS_TRANS_IO		0x6c	/* (@) bus I/O txns */
#define P6_BUS_TRAN_DEF		0x6d	/* (@) bus deferred txns */
#define P6_BUS_TRAN_BURST	0x6e	/* (@) bus burst txns */
#define P6_BUS_TRAN_ANY		0x70	/* (@) total bus txns */
#define P6_BUS_TRAN_MEM		0x6f	/* (@) total memory txns */
#define P6_BUS_DATA_RCV		0x64	/* #busclocks CPU is receiving data */
#define P6_BUS_BNR_DRV		0x61	/* #busclocks CPU is driving BNR pin */
#define P6_BUS_HIT_DRV		0x7a	/* #busclocks CPU is driving HIT pin */
#define P6_BUS_HITM_DRV		0x7b	/* #busclocks CPU is driving HITM pin*/
#define P6_BUS_SNOOP_STALL	0x7e	/* #clkcycles bus is snoop-stalled */

/* FPU */
#define P6_FLOPS	       	0xc1	/* (0) number of FP ops retired */
#define	P6_FP_COMP_OPS		0x10	/* (0) computational FPOPS exec'd */
#define P6_FP_ASSIST		0x11	/* (1) FP excep's handled in ucode */
#define P6_MUL			0x12	/* (1) number of FP multiplies */
#define P6_DIV			0x13	/* (1) number of FP divides */
#define P6_CYCLES_DIV_BUSY	0x14	/* (0) number of cycles divider busy */

/* Memory ordering */
#define P6_LD_BLOCKS		0x03	/* number of store buffer blocks */
#define P6_SB_DRAINS		0x04	/* # of store buffer drain cycles */
#define P6_MISALING_MEM_REF	0x05	/* # misaligned data memory refs */

/* Instruction decoding and retirement */
#define P6_INST_RETIRED		0xc0	/* number of instrs retired */
#define P6_UOPS_RETIRED		0xc2	/* number of micro-ops retired */
#define P6_INST_DECODER		0xd0	/* number of instructions decoded */

/* Interrupts */
#define P6_HW_INT_RX		0xc8	/* number of hardware interrupts */
#define P6_CYCLES_INT_MASKED	0xc6	/* number of cycles hardints masked */
#define P6_CYCLES_INT_PENDING_AND_MASKED 0xc7 /* #cycles masked but pending */

/* Branches */
#define P6_BR_INST_RETIRED	0xc4	/* number of branch instrs retired */
#define P6_BR_MISS_PRED_RETIRED	0xc5	/* number of mispred'd brs retired */
#define P6_BR_TAKEN_RETIRED	0xc9	/* number of taken branches retired */
#define P6_BR_MISS_PRED_TAKEN_RET 0xca	/* #taken mispredictions br's retired*/
#define P6_BR_INST_DECODED    	0xe0	/* number of branch instrs decoded */
#define P6_BTB_MISSES		0xe2	/* # of branches that missed in BTB */
#define P6_BR_BOGUS		0xe4	/* number of bogus branches */
#define P6_BACLEARS		0xe6	/* # times BACLEAR is asserted */

/* Stalls */
#define P6_RESOURCE_STALLS	0xa2	/* # resource-related stall cycles */
#define P6_PARTIAL_RAT_STALLS	0xd2	/* # cycles/events for partial stalls*/

/* Segment register loads */
#define P6_SEGMENT_REG_LOADS	0x06	/* number of segment register loads */

/* Clocks */
#define P6_CPU_CLK_UNHALTED	0x79	/* #clocks CPU is not halted */

/* Unit field tags */
#define P6_UNIT_M		0x0800
#define P6_UNIT_E		0x0400
#define P6_UNIT_S		0x0200
#define P6_UNIT_I		0x0100
#define P6_UNIT_MESI		0x0f00

#define P6_UNIT_SELF		0x0000
#define P6_UNIT_ANY		0x2000

/****************************************************************************
 ** Flag bit definitions (used for the 'flag' field in select_p6counter()) **
 ****************************************************************************
 *
 * The driver accepts fully-formed counter specifications from user-level.
 * The following flags are mneumonics for the bits that get set in the
 * PerfEvtSel0 and PerfEvtSel1 MSR's
 *
 */
#define P6CNT_U  0x010000	/* Monitor user-level events */
#define P6CNT_K  0x020000	/* Monitor kernel-level events */
#define P6CNT_E	 0x040000	/* Edge detect: count state transitions */
#define P6CNT_PC 0x080000	/* Pin control: ?? */
#define P6CNT_IE 0x100000	/* Int enable: enable interrupt on overflow */
#define P6CNT_F  0x200000	/* Freeze counter (handled in software) */
#define P6CNT_EN 0x400000	/* enable counters (in PerfEvtSel0) */
#define P6CNT_IV 0x800000	/* Invert counter mask comparison result */

/*****************************
 ** Miscellaneous constants **
 *****************************
 *
 * Number of Pentium Pro programable hardware counters. 
 */
#define NUM_P6HWC 2

/*****************************************************************************
*
* End of Copyright by Harvard College
*
*****************************************************************************/


#define MSR_P6_EVNTSEL0 0x186
#define MSR_P6_EVNTSEL1 0x187
#define MSR_P6_PERFCTR0 0xc1
#define MSR_P6_PERFCTR1 0xc2

/* P6-specific Makros to manipulate and read counters */

/* Read the 40 bit performance monitoring counter. This requires 
   the PCE-flag in CR4 to be set. Otherwise GP0 is raised. Works only
   at P6.
   */
#define l4_i686_rdpmc(cntr, res_p) \
  __asm __volatile(						\
	 "movl %2, %%ecx	# put counter number in		\n\
	 .byte 0xf; .byte 0x33	# RDPMC instruction		\n\
         movl %%edx, %1		# High order 32 bits		\n\
         movl %%eax, %0		# Low order 32 bits"		\
	: "=g" (*(int *)(res_p)), "=g" (*(((int *)res_p)+1)) 	\
	: "g" (cntr)						\
	: "ecx", "eax", "edx")

static inline l4_uint32_t l4_i686_rdpmc_32(int cntr){
  l4_uint32_t x;
  
  __asm__ __volatile__(
	 ".byte 0xf; .byte 0x33	# RDPMC instruction"
	: "=a" (x)
	: "c" (cntr)
	: "ecx", "eax", "edx");
  return x;
}

static inline void l4_i686_select_perfctr_event(int counter, 
                                                unsigned long long val){
  l4_i586_wrmsr(MSR_P6_EVNTSEL0+counter, &val);
}

static inline void l4_i686_select_perfctr0_event(long long *val){
  asm volatile(
               "movl $MSR_P6_EVNTSEL0, %%ecx\n"
               "movl (%%ebx), %%eax\n"
               "movl 4(%%ebx), %%edx\n"
               //".byte 0xcc, 0xeb, 0x01, 0x21\n"
               ".byte 0x0f, 0x30\n"	// wrmsr
               //".byte 0xcc, 0xeb, 0x01, 0x21\n"
               : /* no output */
               : "b" (val)
               : "ax", "cx", "dx", "bx"
               );

}

/* end of P6 section */
#else

#define K7CNT_U  0x010000	/* Monitor user-level events */
#define K7CNT_K  0x020000	/* Monitor kernel-level events */
#define K7CNT_E	 0x040000	/* Edge detect: count state transitions */
#define K7CNT_PC 0x080000	/* Pin control: ?? */
#define K7CNT_IE 0x100000	/* Int enable: enable interrupt on overflow */
#define K7CNT_F  0x200000	/* Freeze counter (handled in software) */
#define K7CNT_EN 0x400000	/* enable counters (in PerfEvtSel0) */
#define K7CNT_IV 0x800000	/* Invert counter mask comparison result */

#define MSR_K7_EVNTSEL0 0xC0010000
#define MSR_K7_EVNTSEL1 0xC0010001
#define MSR_K7_EVNTSEL2 0xC0010002
#define MSR_K7_EVNTSEL3 0xC0010003
#define MSR_K7_PERFCTR0 0xC0010004
#define MSR_K7_PERFCTR1 0xC0010005
#define MSR_K7_PERFCTR2 0xC0010006
#define MSR_K7_PERFCTR3 0xC0010007

#endif

#endif

/* end of P5/P6/K7 section*/
#endif

/* end of not only lib-prototypes section */
#endif

EXTERN_C_END

#endif
