/**
 * \file
 * \brief APIC for X86
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Frank Mehnert <fm3@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#ifndef __L4_UTIL_APIC_H
#define __L4_UTIL_APIC_H

/*
 * Local APIC programming library
 *
 * For documentation, see
 *
 * "Intel Architecture Software Developer's Manual", Volume 3, chapter 7.5:
 * "Advanced Programmable Interrupt Controller (APIC)"
 *
 * Local APIC is present since
 * - INTEL P6 (PPro)
 * - AMD K7 (Athlon), Model 2
 *
 * In non-SMP-boards, local APIC is disabled, but
 * can be activated by writing to a MSR register.
 * For using APIC see packets cpufreq and l4rtl.
 *
 * See linux/include/asm-i386/i82489.h for further details.
 */

#define APIC_PHYS_BASE              0xFEE00000
#define APIC_MAP_BASE               0xA0200000
#define APIC_BASE_MSR               0x1b

#define APIC_ID                     0x20
#define   GET_APIC_ID(x)              (((x)>>24)&0x0F)
#define APIC_LVR                    0x30
#define   GET_APIC_VERSION(x)         ((x)&0xFF)
#define APIC_TASKPRI                0x80
#define   APIC_TPRI_MASK              0xFF
#define APIC_EOI                    0xB0
#define APIC_LDR                    0xD0
#define   APIC_LDR_MASK               (0xFF<<24)
#define APIC_DFR                    0xE0
#define   SET_APIC_DFR(x)             ((x)<<28)
#define APIC_SPIV                   0xF0
#define APIC_LVTT                   0x320
#define APIC_LVTPC                  0x340
#define APIC_LVT0                   0x350
#define   SET_APIC_TIMER_BASE(x)      (((x)<<18))
#define   APIC_TIMER_BASE_DIV         0x2
#define APIC_LVT1                   0x360
#define APIC_LVTERR                 0x370
#define APIC_TMICT                  0x380
#define APIC_TMCCT                  0x390
#define APIC_TDCR                   0x3E0

#define APIC_LVT_MASKED             (1<<16)
#define APIC_LVT_TIMER_PERIODIC     (1<<17)
#define APIC_TDR_DIV_1              0xB
#define APIC_TDR_DIV_2              0x0
#define APIC_TDR_DIV_4              0x1
#define APIC_TDR_DIV_8              0x2
#define APIC_TDR_DIV_16             0x3
#define APIC_TDR_DIV_32             0x8
#define APIC_TDR_DIV_64             0x9
#define APIC_TDR_DIV_128            0xA

#include <l4/sys/compiler.h>
#include <l4/sys/types.h>

EXTERN_C_BEGIN

/* prototypes */
extern unsigned long apic_map_base;
extern unsigned long apic_timer_divisor;

extern unsigned long l4_scaler_apic_to_ms;

L4_CV void apic_show_registers(void);
L4_CV int  apic_check_working(void);
L4_CV void apic_activate_by_io(void);
L4_CV void apic_timer_set_divisor(int divisor);

L4_CV unsigned long l4_calibrate_apic(void);

EXTERN_C_END

L4_INLINE void apic_write(unsigned long reg, unsigned long v);
L4_INLINE unsigned long apic_read(unsigned long reg);
L4_INLINE void apic_activate_by_msr(void);
L4_INLINE void apic_deactivate_by_msr(void);
L4_INLINE unsigned long apic_read_phys_address(void);
L4_INLINE int  apic_test_present(void);
L4_INLINE void apic_soft_enable(void);
L4_INLINE void apic_init(unsigned long map_addr);
L4_INLINE void apic_done(void);
L4_INLINE void apic_irq_ack(void);

L4_INLINE void apic_lvt0_disable_irq(void);
L4_INLINE void apic_lvt0_enable_irq(void);
L4_INLINE void apic_lvt1_disable_irq(void);
L4_INLINE void apic_lvt1_enable_irq(void);

L4_INLINE void apic_timer_write(unsigned long value);
L4_INLINE unsigned long apic_timer_read(void);
L4_INLINE void apic_timer_disable_irq(void);
L4_INLINE void apic_timer_enable_irq(void);
L4_INLINE void apic_timer_assign_irq(unsigned long vector);
L4_INLINE void apic_timer_set_periodic(void);
L4_INLINE void apic_timer_set_one_shot(void);

L4_INLINE void apic_perf_disable_irq(void);
L4_INLINE void apic_perf_enable_irq(void);
L4_INLINE void apic_perf_assign_irq(unsigned long vector);


/* write APIC register */
L4_INLINE void
apic_write(unsigned long reg, unsigned long v)
{
  *((volatile unsigned long *)(apic_map_base+reg))=v;
}
    

/* read APIC register */
L4_INLINE unsigned long
apic_read(unsigned long reg)
{
  return *((volatile unsigned long *)(apic_map_base+reg));
}


/* disable LINT0 */
L4_INLINE void
apic_lvt0_disable_irq(void)
{
  unsigned long tmp_val;
  tmp_val = apic_read(APIC_LVT0);
  tmp_val |= APIC_LVT_MASKED;
  apic_write(APIC_LVT0, tmp_val);
}


/* enable LINT0 */
L4_INLINE void
apic_lvt0_enable_irq(void)
{
  unsigned long tmp_val;
  tmp_val = apic_read(APIC_LVT0);
  tmp_val &= ~(APIC_LVT_MASKED);
  apic_write(APIC_LVT0, tmp_val);
}


/* disable LINT1 */
L4_INLINE void
apic_lvt1_disable_irq(void)
{
  unsigned long tmp_val;
  tmp_val = apic_read(APIC_LVT1);
  tmp_val |= APIC_LVT_MASKED;
  apic_write(APIC_LVT1, tmp_val);
}


/* enable LINT1 */
L4_INLINE void
apic_lvt1_enable_irq(void)
{
  unsigned long tmp_val;
  tmp_val = apic_read(APIC_LVT1);
  tmp_val &= ~(APIC_LVT_MASKED);
  apic_write(APIC_LVT1, tmp_val);
}


/* write APIC timer register */
L4_INLINE void
apic_timer_write(unsigned long value)
{
  apic_read(APIC_TMICT);
  apic_write(APIC_TMICT,value);
}


/* read APIC timer register */
L4_INLINE unsigned long
apic_timer_read(void)
{
  return apic_read(APIC_TMCCT);
}


/* disable IRQ when APIC timer passes 0 */
L4_INLINE void
apic_timer_disable_irq(void)
{
  unsigned long tmp_val;
  tmp_val = apic_read(APIC_LVTT);
  tmp_val |= APIC_LVT_MASKED;
  apic_write(APIC_LVTT, tmp_val);
}


/* enable IRQ when APIC timer passes 0 */
L4_INLINE void
apic_timer_enable_irq(void)
{
  unsigned long tmp_val;
  tmp_val = apic_read(APIC_LVTT);
  tmp_val &= ~(APIC_LVT_MASKED);
  apic_write(APIC_LVTT, tmp_val);
}


L4_INLINE void
apic_timer_set_periodic(void)
{
  unsigned long tmp_val;
  tmp_val = apic_read(APIC_LVTT);
  tmp_val |= APIC_LVT_TIMER_PERIODIC;
  tmp_val |= APIC_LVT_MASKED;
  apic_write(APIC_LVTT, tmp_val);
}


L4_INLINE void
apic_timer_set_one_shot(void)
{
  unsigned long tmp_val;
  tmp_val = apic_read(APIC_LVTT);
  tmp_val &= ~APIC_LVT_TIMER_PERIODIC;
  tmp_val |= APIC_LVT_MASKED;
  apic_write(APIC_LVTT, tmp_val);
}


/* set vector of APIC timer irq */
L4_INLINE void
apic_timer_assign_irq(unsigned long vector)
{
  unsigned long tmp_val;
  tmp_val = apic_read(APIC_LVTT);
  tmp_val &= 0xffffff00;
  tmp_val |= vector;
  tmp_val |= APIC_LVT_MASKED;
  apic_write(APIC_LVTT, tmp_val);
}


/* disable IRQ when performance counter passes 0 */
L4_INLINE void
apic_perf_disable_irq(void)
{
  unsigned long tmp_val;
  tmp_val = apic_read(APIC_LVTPC);
  tmp_val |= APIC_LVT_MASKED;
  apic_write(APIC_LVTPC, tmp_val);
}


/* enable IRQ when performance counter passes 0 */
L4_INLINE void
apic_perf_enable_irq(void)
{
  unsigned long tmp_val;
  tmp_val = apic_read(APIC_LVTPC);
  tmp_val &= ~(APIC_LVT_MASKED);
  apic_write(APIC_LVTPC, tmp_val);
}


/* set vector of performance counter irq */
L4_INLINE void
apic_perf_assign_irq(unsigned long vector)
{
  unsigned long tmp_val;
  tmp_val = apic_read(APIC_LVTPC);
  tmp_val &= 0xffffff00;
  tmp_val |= vector;
  tmp_val |= APIC_LVT_MASKED;
  apic_write(APIC_LVTPC, tmp_val);
}


/* activate APIC by writing to appropriate MSR */
L4_INLINE void
apic_activate_by_msr(void)
{
  unsigned long low;
  unsigned long high;
    
  /* rdmsr */
  asm volatile(".byte 0xf; .byte 0x32\n"
               :"=a" (low),
                "=d" (high)
               :"c" (APIC_BASE_MSR)
               );

  low |= 0x800;                         /* activate APIC */
  low &= 0x00000fff;
  low |= (APIC_PHYS_BASE & 0xfffff000); /* set address */
  
  /* wrmsr */
  asm volatile(".byte 0xf; .byte 0x30\n"
               :
               :"c" (APIC_BASE_MSR),
                "a" (low),
                "d" (high)
               );
}


/* deactivate APIC by writing to appropriate MSR */
L4_INLINE void
apic_deactivate_by_msr(void)
{
  unsigned long low;
  unsigned long high;
    
  /* rdmsr */
  asm volatile(".byte 0xf; .byte 0x32\n"
               :"=a" (low),
                "=d" (high)
               :"c" (APIC_BASE_MSR)
               );

  low  &= 0xfffff7ff;                    /* deactivate APIC */
    
  /* wrmsr */
  asm volatile(".byte 0xf; .byte 0x30\n"
               :
               :"c" (APIC_BASE_MSR),
                "a" (low),
                "d" (high)
               );
}


/* read memory mapped address of apic */
L4_INLINE unsigned long
apic_read_phys_address(void)
{
  unsigned long low;
  unsigned long high;

  /* rdmsr */
  asm volatile(".byte 0xf; .byte 0x32\n"
               :"=a" (low),
                "=d" (high)
               :"c" (APIC_BASE_MSR)
               );

  return (low  &= 0xfffff000);
}


/* test if APIC present */
L4_INLINE int
apic_test_present(void)
{
  unsigned int dummy;
  unsigned int capability;

  asm volatile("pushl %%ebx ; cpuid ; popl %%ebx"
             : "=a" (dummy),
               "=c" (dummy),
               "=d" (capability)
             : "a" (0x00000001)
             : "cc");

  return ((capability & 1<<9) !=0);
}


L4_INLINE void
apic_soft_enable(void)
{
  unsigned long tmp_val;
  tmp_val = apic_read(APIC_SPIV);
  tmp_val |= (1<<8);          /* enable APIC */
  tmp_val &= ~(1<<9);         /* enable Focus Processor Checking */
  tmp_val |= 0xff;            /* Set spurious IRQ vector to 0xff */
  apic_write(APIC_SPIV, tmp_val);
}


L4_INLINE void
apic_init(unsigned long base_addr)
{
  apic_map_base = base_addr;
}


L4_INLINE void
apic_done(void)
{
  apic_map_base = 0;
}


L4_INLINE void
apic_irq_ack(void)
{
  apic_read(APIC_SPIV);
  apic_write(APIC_EOI, 0);
}


#endif /* __L4_UTIL_APIC_H */
