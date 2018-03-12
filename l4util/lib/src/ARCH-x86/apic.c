/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Frank Mehnert <fm3@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#include <l4/sys/ipc.h>
#include <l4/sys/kdebug.h>
#include <l4/util/port_io.h>
#include <l4/util/irq.h>
#include <l4/util/apic.h>


unsigned long apic_map_base = 0;
unsigned long apic_timer_divisor = 1;
unsigned long l4_scaler_apic_to_ms = 0;

static void
apic_show_register_block(unsigned int beg, unsigned int len)
{
  unsigned int i;
  outhex16(beg);
  outchar(':');
  for (i=beg; i<beg+len; i+=0x10)
    {
      outchar(' ');
      outhex32(apic_read(i));
    }
  outstring("\r\n");
}

L4_CV void
apic_show_registers(void)
{
  if (!apic_map_base)
    return;

  apic_show_register_block( 0x00, 0x80);  // ID, Version
  apic_show_register_block( 0x80, 0x80);  // TaskPrio, Arb, ProcPrio, DFR
  apic_show_register_block(0x100, 0x80);  // ISR 0-255
  apic_show_register_block(0x180, 0x80);  // TMR 0-255
  apic_show_register_block(0x200, 0x80);  // IRR 0-255
  apic_show_register_block(0x300, 0x80);  // ICR
  apic_show_register_block(0x380, 0x10);  // Initial Count Register
}

L4_CV void
apic_timer_set_divisor(int newdiv)
{
  int i;
  int div = -1;
  int divval = newdiv;
  unsigned long tmp_value;

  static int divisor_tab[8] =
    {
      APIC_TDR_DIV_1,  APIC_TDR_DIV_2,  APIC_TDR_DIV_4,  APIC_TDR_DIV_8,
      APIC_TDR_DIV_16, APIC_TDR_DIV_32, APIC_TDR_DIV_64, APIC_TDR_DIV_128
    };

  if (!apic_map_base)
    return;

  for (i=0; i<8; i++)
    {
      if (divval & 1)
	{
	  if (divval & ~1)
	    {
	      enter_kdebug("bad APIC divisor");
	    }
	  div = divisor_tab[i];
	  break;
	}
      divval >>= 1;
    }

  if (div != -1)
    {
      apic_timer_divisor = newdiv;
      tmp_value = apic_read(APIC_TDCR);
      tmp_value &= ~0x1F;
      tmp_value |= div;
      apic_write(APIC_TDCR, tmp_value);
    }
}


L4_CV int
apic_check_working(void)
{
#define CLOCK_TICK_RATE 1193180  /* i8254 ticks per second */
  unsigned long count;
  unsigned long tt1, tt2;

  unsigned int calibrate_latch = (CLOCK_TICK_RATE / 20); /* 50 ms */

  if (!apic_map_base)
    return 0;

  apic_timer_disable_irq();
  apic_timer_set_divisor(1);
  apic_timer_write(1000000000);

  /* Set the Gate high, disable speaker */
  l4util_out8((l4util_in8(0x61) & ~0x02) | 0x01, 0x61);

  l4util_out8(0xb0, 0x43);  /* binary, mode 0, LSB/MSB, Ch 2 */
  l4util_out8(calibrate_latch & 0xff, 0x42); /* LSB of count */
  l4util_out8(calibrate_latch >> 8,   0x42); /* MSB of count */

  tt1=apic_timer_read();
  count = 0;
  do
    {
      count++;
    } while ((l4util_in8(0x61) & 0x20) == 0);

  tt2=apic_timer_read();
  return (tt1-tt2) != 0;
}


/* activate APIC after activating by MSR was successful *
 * see "Intel Architecture Software Developer's Manual, *
 *      Volume 3: System Programming Guide, Appendix E" */
L4_CV void
apic_activate_by_io(void)
{
  char old_21, old_A1;
  unsigned long tmp_val;
  l4_umword_t flags;

  /* mask 8259 interrupts */
  old_21 = l4util_in8(0x21);
  l4util_out8(0xff, 0x21);
  old_A1 = l4util_in8(0xA1);
  l4util_out8(0xff, 0xA1);

  l4util_flags_save(&flags);
  l4util_cli();

  apic_soft_enable();

  /* set LINT0 to ExtINT, edge triggered */
  tmp_val = apic_read(APIC_LVT0);
  tmp_val &= 0xfffe58ff;
  tmp_val |= 0x00000700;
  apic_write(APIC_LVT0, tmp_val);

  /* set LINT1 to NMI, edge triggered */
  tmp_val = apic_read(APIC_LVT1);
  tmp_val &= 0xfffe58ff;
  tmp_val |= 0x00000400;
  apic_write(APIC_LVT1, tmp_val);

  /* unmask 8259 interrupts */
  l4util_flags_restore(&flags);
  l4util_out8(old_A1, 0xA1);
  l4util_out8(old_21, 0x21);
}

/*
 * Return APIC clocks per ms
 */
L4_CV unsigned long
l4_calibrate_apic (void)
{
  unsigned int calibrate_latch = (CLOCK_TICK_RATE / 20); /* 50 ms */
  unsigned int calibrate_time  = 50;                     /* 50 ms */

  if (!apic_map_base)
    return 0;

  apic_timer_disable_irq();
  apic_timer_set_divisor(apic_timer_divisor);
  apic_timer_write(1000000000);

  /* Set the Gate high, disable speaker */
  l4util_out8((l4util_in8(0x61) & ~0x02) | 0x01, 0x61);

  l4util_out8(0xb0, 0x43);  /* binary, mode 0, LSB/MSB, Ch 2 */
  l4util_out8(calibrate_latch & 0xff, 0x42); /* LSB of count */
  l4util_out8(calibrate_latch >> 8,   0x42); /* MSB of count */

    {
      unsigned long count;
      unsigned long tt1, tt2;
      unsigned long result;

      tt1=apic_timer_read();
      count = 0;
      do 
	{
      	  count++;
      	} while ((l4util_in8(0x61) & 0x20) == 0);
      tt2=apic_timer_read();
        
      result = (tt1-tt2)*apic_timer_divisor;

      /* Error: ECTCNEVERSET */
      if (count <= 1)
	goto bad_ctc;

      /* Error: ECPUTOOSLOW */
      if (result <= calibrate_time)
	goto bad_ctc;

      __asm__ ("divl %1"
	      :"=a" (result)
	      :"r" (calibrate_time),
	       "0" (result),
	       "d" (0));

      l4_scaler_apic_to_ms = result;

      return result;
    }

bad_ctc:
    return 0;
}

