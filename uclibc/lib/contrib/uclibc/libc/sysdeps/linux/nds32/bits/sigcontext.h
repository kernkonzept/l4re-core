/*
 * Copyright (C) 2016 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifndef _BITS_SIGCONTEXT_H
#define _BITS_SIGCONTEXT_H 1

#ifndef sigcontext_struct
#define sigcontext_struct sigcontext

struct sigcontext{
        unsigned long trap_no;
        unsigned long error_code;
        unsigned long oldmask;
        unsigned long nds32_r0;
        unsigned long nds32_r1;
        unsigned long nds32_r2;
        unsigned long nds32_r3;
        unsigned long nds32_r4;
        unsigned long nds32_r5;
        unsigned long nds32_r6;
        unsigned long nds32_r7;
        unsigned long nds32_r8;
        unsigned long nds32_r9;
        unsigned long nds32_r10;
        unsigned long nds32_r11;
        unsigned long nds32_r12;
        unsigned long nds32_r13;
        unsigned long nds32_r14;
        unsigned long nds32_r15;
        unsigned long nds32_r16;
        unsigned long nds32_r17;
        unsigned long nds32_r18;
        unsigned long nds32_r19;
	unsigned long nds32_r20;
	unsigned long nds32_r21;
	unsigned long nds32_r22;
	unsigned long nds32_r23;
	unsigned long nds32_r24;
	unsigned long nds32_r25;
        unsigned long nds32_fp;   //r28
        unsigned long nds32_gp;   //r29
        unsigned long nds32_lp;   //r30
        unsigned long nds32_sp;   //r31
        unsigned long nds32_d1lo;
        unsigned long nds32_d1hi;
        unsigned long nds32_d0lo;
        unsigned long nds32_d0hi;
        unsigned long nds32_ipsw;
        unsigned long nds32_ipc;
        unsigned long fault_address;
};

#define sc_pc  nds32_ipc /* For sysdeps/generic/profil-counter.h.  */

#endif /* sigcontext_struct  */

#endif /* _BITS_SIGCONTEXT_H  */
