/**
 */

#pragma once

#include <l4/sys/vcpu.h>

enum L4_vm_state_modified_bits
{
  /**
   * Guest.Config0..7 registers.
   *
   * Stored in `gcp0`.
   */
  L4_VM_MOD_CFG = 1UL << 0,

  /**
   * Guest MMU registers: Index, EntryHi, EntryLo0..1, Context, PageMask.
   *
   * Stored in `gcp0`.
   */
  L4_VM_MOD_MMU = 1UL << 1,

  /**
   * Guest address translation registers: PageGrain, Wired, SegCtl.
   *
   * Stored in `gcp0`.
   */
  L4_VM_MOD_XLAT = 1UL << 2,

  /**
   * Guest page walker registers: PWBase, PWField, PWSize, PWCtl.
   *
   * Stored in `gcp0`.
   */
  L4_VM_MOD_PW = 1UL << 3,

  /**
   * Guest: BadVAddr, BadInstr, BadInstrP registers.
   *
   * Stored in `gcp0`.
   */
  L4_VM_MOD_BAD = 1UL << 4,

  /**
   * Guest KScr 1..7 registers.
   */
  L4_VM_MOD_KSCR = 1UL << 5,

  /**
   * Guest.Status register.
   *
   * The Guest.Status register is saved on each VM -> VMM transition.
   *
   * Stored in `gcp0`.
   */
  L4_VM_MOD_STATUS = 1UL << 6,

  /**
   * Guest.Cause register.
   *
   * Stored in `gcp0`.
   */
  L4_VM_MOD_CAUSE = 1UL << 7,

  /**
   * Guest EPC and ErrorEPC registers.
   *
   * Stored in `gcp0`.
   */
  L4_VM_MOD_EPC = 1UL << 8,

  /**
   * Guest.HWRena register.
   *
   * Stored in `gcp0`.
   */
  L4_VM_MOD_HWRENA = 1UL << 9,

  /**
   * Guest.IntCtl register.
   *
   * Stored in `gcp0`.
   */
  L4_VM_MOD_INTCTL = 1UL << 10,

  /**
   * Guest.EBase register.
   *
   * Stored in `gcp0`.
   */
  L4_VM_MOD_EBASE = 1UL << 11,

  /**
   * Guest.ULR register.
   *
   * Stored in `gcp0`.
   */
  L4_VM_MOD_ULR = 1UL << 12,

  /**
   * Guest.DESAVE register.
   *
   * Stored in `gcp0`.
   */
  L4_VM_MOD_DESAVE = 1UL << 13,

  /**
   * GuestCtl0 register.
   *
   * The GuestCtl0 register is saved on each VM -> VMM transition.
   *
   * Stored in `guest_ctl_0`.
   */
  L4_VM_MOD_GUEST_CTL_0 = 1UL << 16,

  /**
   * GuestCtl0Ext register.
   *
   * Never changed by the VM (so never saved).
   *
   * Stored in `guest_ctl_0_ext`.
   */
  L4_VM_MOD_GUEST_CTL_0_EXT = 1UL << 17,

  /**
   * GuestCtl2 register.
   *
   * Never changed by the VM (so never saved).
   *
   * Stored in `guest_ctl_2`.
   */
  L4_VM_MOD_GUEST_CTL_2 = 1UL << 18,

  /**
   * GTOffset (Guest Timer Offset) register.
   *
   * Never changed by the VM (so never saved).
   *
   * Stored in `guest_timer_offset`.
   */
  L4_VM_MOD_GTOFFSET = 1UL << 24,

  /**
   * Guest.Compare register.
   *
   * Stored in `gcp0`.
   */
  L4_VM_MOD_COMPARE = 1UL << 25,

  /**
   * Guest.LLbit.
   *
   * When this bit is set in the modified mask the LLBit in Guest.LLAddr
   * will be cleared. Only available when Guest.Config5.LLB is set.
   */
  L4_VM_MOD_LLBIT = 1UL << 26,
};

enum L4_vm_cp0_regs
{
  /* 32 */ L4_VM_CP0_INDEX            = ( 0 << 3),
  /* 32 */ L4_VM_CP0_VP_CONTROL       = ( 0 << 3) + 4,
  /* 32 */ L4_VM_CP0_RANDOM           = ( 1 << 3),
  /* 64 */ L4_VM_CP0_ENTRY_LO1        = ( 2 << 3),
  /* 64 */ L4_VM_CP0_ENTRY_LO2        = ( 3 << 3),
  /* 32 */ L4_VM_CP0_GLOBAL_NUMBER    = ( 3 << 3) + 1,
  /* 64 */ L4_VM_CP0_CONTEXT          = ( 4 << 3),
  /* 32 */ L4_VM_CP0_CONTEXT_CONFIG   = ( 4 << 3) + 1,
  /* 64 */ L4_VM_CP0_USER_LOCAL       = ( 4 << 3) + 2,
  /* 64 */ L4_VM_CP0_XCONTEXT_CONFIG  = ( 4 << 3) + 3,
  /* 32 */ L4_VM_CP0_DEBUG_CONTEXT_ID = ( 4 << 3) + 4,
  /* 64 */ L4_VM_CP0_PAGE_MASK        = ( 5 << 3),
  /* 32 */ L4_VM_CP0_PAGE_GRAIN       = ( 5 << 3) + 1,
  /* 64 */ L4_VM_CP0_SEG_CTL_0        = ( 5 << 3) + 2,
  /* 64 */ L4_VM_CP0_SEG_CTL_1        = ( 5 << 3) + 3,
  /* 64 */ L4_VM_CP0_SEG_CTL_2        = ( 5 << 3) + 4,
  /* 64 */ L4_VM_CP0_PW_BASE          = ( 5 << 3) + 5,
  /* 64 */ L4_VM_CP0_PW_FIELD         = ( 5 << 3) + 6,
  /* 64 */ L4_VM_CP0_PW_SIZE          = ( 5 << 3) + 7,
  /* 32 */ L4_VM_CP0_WIRED            = ( 6 << 3),
  /* 32 */ L4_VM_CP0_PW_CTL           = ( 6 << 3) + 6,
  /* 32 */ L4_VM_CP0_HW_RENA          = ( 7 << 3),
  /* 64 */ L4_VM_CP0_BAD_V_ADDR       = ( 8 << 3),
  /* 32 */ L4_VM_CP0_BAD_INSTR        = ( 8 << 3) + 1,
  /* 32 */ L4_VM_CP0_BAD_INSTR_P      = ( 8 << 3) + 2,
  /* 32 */ L4_VM_CP0_COUNT            = ( 9 << 3),
  /* 64 */ L4_VM_CP0_ENTRY_HI         = (10 << 3),
  /* 32 */ L4_VM_CP0_COMPARE          = (11 << 3),
  /* 32 */ L4_VM_CP0_STATUS           = (12 << 3),
  /* 32 */ L4_VM_CP0_INT_CTL          = (12 << 3) + 1,
  /* 32 */ L4_VM_CP0_SRS_CTL          = (12 << 3) + 2,
  /* 32 */ L4_VM_CP0_SRS_MAP          = (12 << 3) + 3,
  /* 32 */ L4_VM_CP0_CAUSE            = (13 << 3),
  /* 32 */ L4_VM_CP0_NESTED_EXC       = (13 << 3) + 5,
  /* 64 */ L4_VM_CP0_EPC              = (14 << 3),
  /* 64 */ L4_VM_CP0_NESTED_EPC       = (14 << 3) + 2,
  /* 32 */ L4_VM_CP0_PROC_ID          = (15 << 3),
  /* 64 */ L4_VM_CP0_EBASE            = (15 << 3) + 1,
  /* 64 */ L4_VM_CP0_CDMM_BASE        = (15 << 3) + 2,
  /* 64 */ L4_VM_CP0_CMGCR_BASE       = (15 << 3) + 3,
  /* 32 */ L4_VM_CP0_BEVVA            = (15 << 3) + 4,
  /* 32 */ L4_VM_CP0_CONFIG_0         = (16 << 3),
  /* 32 */ L4_VM_CP0_CONFIG_1         = (16 << 3) + 1,
  /* 32 */ L4_VM_CP0_CONFIG_2         = (16 << 3) + 2,
  /* 32 */ L4_VM_CP0_CONFIG_3         = (16 << 3) + 3,
  /* 32 */ L4_VM_CP0_CONFIG_4         = (16 << 3) + 4,
  /* 32 */ L4_VM_CP0_CONFIG_5         = (16 << 3) + 5,
  /* 32 */ L4_VM_CP0_CONFIG_6         = (16 << 3) + 6,
  /* 32 */ L4_VM_CP0_CONFIG_7         = (16 << 3) + 7,
  /* 64 */ L4_VM_CP0_LOAD_LINKED_ADDR = (17 << 3),
  /* 64 */ L4_VM_CP0_MAAR_0           = (17 << 3) + 1,
  /* 64 */ L4_VM_CP0_MAAR_1           = (17 << 3) + 2,
  /* 64 */ L4_VM_CP0_WATCH_LO         = (18 << 3),
  /* 64 */ L4_VM_CP0_WATCH_HI         = (19 << 3),
  /* 64 */ L4_VM_CP0_XCONTEXT         = (20 << 3),
  /* 64 */ L4_VM_CP0_DEBUG            = (23 << 3),
  /* 64 */ L4_VM_CP0_DEBUG_2          = (23 << 3) + 6,
  /* 64 */ L4_VM_CP0_DEPC             = (24 << 3),
  /* 32 */ L4_VM_CP0_PERF_CTL_0       = (25 << 3),
  /* 64 */ L4_VM_CP0_PERF_COUNTER_0   = (25 << 3) + 1,
  /* 32 */ L4_VM_CP0_PERF_CTL_1       = (25 << 3) + 2,
  /* 64 */ L4_VM_CP0_PERF_COUNTER_1   = (25 << 3) + 3,
  /* 32 */ L4_VM_CP0_PERF_CTL_2       = (25 << 3) + 4,
  /* 64 */ L4_VM_CP0_PERF_COUNTER_2   = (25 << 3) + 5,
  /* 32 */ L4_VM_CP0_PERF_CTL_3       = (25 << 3) + 6,
  /* 64 */ L4_VM_CP0_PERF_COUNTER_3   = (25 << 3) + 7,
  /* 64 */ L4_VM_CP0_ERR_CTL          = (26 << 3),
  /* 64 */ L4_VM_CP0_CACHE_ERR        = (27 << 3),
  /* 64 */ L4_VM_CP0_TAG_LO_0         = (28 << 3),
  /* 64 */ L4_VM_CP0_DATA_LO_0        = (28 << 3) + 1,
  /* 64 */ L4_VM_CP0_TAG_LO_1         = (28 << 3) + 2,
  /* 64 */ L4_VM_CP0_DATA_LO_1        = (28 << 3) + 3,
  /* 64 */ L4_VM_CP0_TAG_HI_0         = (29 << 3),
  /* 64 */ L4_VM_CP0_DATA_HI_0        = (29 << 3) + 1,
  /* 64 */ L4_VM_CP0_TAG_HI_1         = (29 << 3) + 2,
  /* 64 */ L4_VM_CP0_DATA_HI_1        = (29 << 3) + 3,
  /* 64 */ L4_VM_CP0_ERR_EPC          = (30 << 3),
  /* 64 */ L4_VM_CP0_DESAVE           = (31 << 3),
  /* 64 */ L4_VM_CP0_KSCRATCH_1       = (31 << 3) + 2,
  /* 64 */ L4_VM_CP0_KSCRATCH_2       = (31 << 3) + 3,
  /* 64 */ L4_VM_CP0_KSCRATCH_3       = (31 << 3) + 4,
  /* 64 */ L4_VM_CP0_KSCRATCH_4       = (31 << 3) + 5,
  /* 64 */ L4_VM_CP0_KSCRATCH_5       = (31 << 3) + 6,
  /* 64 */ L4_VM_CP0_KSCRATCH_6       = (31 << 3) + 7,
};

typedef struct l4_mips_vm_tlb_entry_t
{
  l4_umword_t mask;
  l4_umword_t entry_hi;
  l4_umword_t entry_lo[2];
} l4_mips_vm_tlb_entry_t;

enum { L4_MIPS_MAX_GUEST_WIRED = 16 };

/**
 * L4 extended vCPU state for MIPS.
 *
 * Contains the additional MIPS guest state accompanying the `l4_vcpu_state_t`.
 */
typedef struct l4_vm_state_t
{
  l4_umword_t version;
  l4_umword_t size;

  /**
   * GuestCtl0Ext.
   *
   * Initialized when setting up extended vCPU mode.
   * Never stored. Loaded with #L4_VM_MOD_GUEST_CTL_0_EXT in
   * `modified_cp0_map`.
   */
  l4_umword_t guest_ctl_0_ext;

  /**
   * GTOffset.
   *
   * Initialized when setting up extended vCPU mode.
   * Never stored. Loaded with #L4_VM_MOD_GTOFFSET in `modified_cp0_map`.
   */
  l4_umword_t guest_timer_offset;

  /**
   * GuestCtl1.
   *
   * Initialized when setting up extended vCPU mode.
   * Never stored. Never loaded.
   */
  l4_umword_t guest_ctl_1;

  /**
   * GuestCtl2.
   *
   * Initialized when setting up extended vCPU mode.
   * Never stored. Loaded with #L4_VM_MOD_GUEST_CTL_2 in `modified_cp0_map`.
   */
  l4_umword_t guest_ctl_2;

  /**
   * Bitmap that indicates which registers (gcp0 and guest_ctl_*) are currently
   * in sync with VM state.
   *
   * Bits correspond to bits definitions in `L4_vm_state_modified_bits`. A set
   * bit indicates that the corresponding registers are in sync, a cleared bit
   * means that the VM might have modified the corresponding registers.
   *
   * Additional state can be requested by the VMM using
   * l4_thread_mips_save_vm_state().
   */
  l4_uint32_t clean_cp0_map;

  /**
   * Bitmap that indicates which state has been modified by the VMM and must be
   * loaded on `l4_thread_vcpu_resume`.
   *
   * The bits correspond to the definitions in `L4_vm_state_modified_bits`.
   * A set bit indicates that the registers shall be loaded on vCPU resume.
   * A cleared bit indicates that the registers are not touched by the VMM.
   *
   * Note the vCPU resume operation might load more than the indicated state.
   */
  l4_uint32_t modified_cp0_map;

  /**
   * GuestCtl0.
   *
   * Initialized when setting up extended vCPU mode.
   * Stored on each VM -> VMM switch. Loaded with #L4_VM_MOD_GUEST_CTL_0 in
   * `modified_cp0_map`.
   */
  l4_umword_t guest_ctl_0;
  l4_umword_t g_status;      // $12, 0

  /**
   * This is an internal timestamp not to be used or modified by the VMM.
   */
  l4_uint64_t saved_cause_timestamp;
  l4_umword_t g_cause;       // $13, 0
  l4_umword_t g_compare;     // $11, 0

  l4_umword_t g_cfg[6];      // Guest.Config[0..5]

  l4_umword_t g_index;       // $0, 0
  l4_umword_t g_entry_lo[2]; // $2 and $3
  l4_umword_t g_context;     // $4, 0
  l4_umword_t g_page_mask;   // $5, 0
  l4_umword_t g_wired;       // $6, 0
  l4_umword_t g_entry_hi;    // $10, 0

  l4_umword_t g_page_grain;  // $5, 1
  l4_umword_t g_seg_ctl[3];  // $5, 2 .. 4

  l4_umword_t g_pw_base;     // $5, 5
  l4_umword_t g_pw_field;    // $5, 6
  l4_umword_t g_pw_size;     // $5, 7
  l4_umword_t g_pw_ctl;      // $6, 6

  l4_umword_t g_intctl;      // $12, 1
  l4_umword_t g_ulr;         // $4, 2
  l4_umword_t g_epc;         // $14, 0
  l4_umword_t g_error_epc;   // $30, 0
  l4_umword_t g_ebase;       // $15, 1

  l4_umword_t g_hwrena;      // $7, 0
  l4_umword_t g_bad_v_addr;  // $8, 0
  l4_umword_t g_bad_instr;   // $8, 1
  l4_umword_t g_bad_instr_p; // $8, 2

  l4_umword_t g_kscr[8];     // $31, 0 .. 7; where 0 is actually desave

  l4_mips_vm_tlb_entry_t g_tlb_wired[L4_MIPS_MAX_GUEST_WIRED];
} l4_vm_state_t;


L4_INLINE l4_vm_state_t *
l4_vm_state(l4_vcpu_state_t *vcpu) L4_NOTHROW;

L4_INLINE l4_vm_state_t *
l4_vm_state(l4_vcpu_state_t *vcpu) L4_NOTHROW
{ return (l4_vm_state_t *)((char *)vcpu + 0x400); }

