/*
 * Copyright (C) 2019 Kernkonzept GmbH.
 * Author(s): Frank Mehnert <frank.mehnert@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

#include <climits>
#include <cstring>

#include <l4/re/util/icu_svr>
#include <l4/sys/factory>
#include <l4/sys/kdebug.h>
#include <l4/sys/ktrace.h>

#include <l4/atkins/tap/main>
#include <l4/atkins/debug>
#include <l4/atkins/factory>
#include <l4/atkins/fixtures/epiface_provider>
#include <l4/atkins/l4_assert>

static L4Re::Env const *const env = L4Re::Env::env();

enum
{
  Num_irqs = 3,
};

class Test_icu_svr
: public L4Re::Util::Icu_cap_array_svr<Test_icu_svr>,
  public L4::Epiface_t<Test_icu_svr, L4::Icu>
{
public:
  typedef L4Re::Util::Icu_cap_array_svr<Test_icu_svr> Icu_svr;

  Test_icu_svr() : Icu_svr(Num_irqs, _irq)
  {}

  void trigger_irq(unsigned irqnum) const
  { _irq[irqnum].trigger(); }

  Icu_svr::Irq _irq[Num_irqs];
};


class TestIcuSvr : public Atkins::Fixture::Epiface_thread<Test_icu_svr>
{
public:
  TestIcuSvr() : Epiface_thread(0 /* rcv_cap_flags */)
  {}
};


/**
 * The Icu_svr provides information about IRQ lines.
 */
TEST_F(TestIcuSvr, Info)
{
  L4::Icu::Info info;
  ASSERT_L4OK(scap()->info(&info))
    << "Call Icu_svr::info()";
  ASSERT_EQ(0U, info.features)
    << "Features returned from Icu_svr::info().";
  ASSERT_EQ(Num_irqs, info.nr_irqs)
    << "Number of IRQs at the Icu_svr.";
  ASSERT_EQ(0U, info.nr_msis)
    << "Number of MSI interrupts at the Icu_svr.";
}

/**
 * The Icu_svr allows to call the msi_info() for an IRQ line. As there are no
 * MSI interrupts implemented at Icu_svr, the function will fail arbitrary.
 */
TEST_F(TestIcuSvr, MsiInfo)
{
  l4_icu_msi_info_t info;
  for (unsigned i = 0; i < Num_irqs; ++i)
    {
      // Icu_svr::msi_info() return -L4_EINVAL for 
      ASSERT_L4IPC_OK(scap()->msi_info(i, 0, &info))
        << "Call Icu_svr::msi_info(" << i << ").";
    }
}

/**
 * Icu_svr ignores masking and unmasking of an IRQs.
 *
 * Go through the IRQ lines at the Icu_svr and first mask, then unmask the
 * corresponding line. Verify that the send-only IPC succeeds in both cases.
 */
TEST_F(TestIcuSvr, MaskUnmask)
{
  for (unsigned i = 0; i < Num_irqs; ++i)
    {
      // Icu::mask is a send-only IPC:
      // There is only an undefined return code so test for IPC error.
      ASSERT_L4IPC_OK(scap()->mask(i))
        << "Call Icu_svr::mask(" << i << ").";

      // Icu::unmask is a send-only IPC:
      // There is only an undefined return code so test for IPC error.
      ASSERT_L4IPC_OK(scap()->unmask(i))
        << "Call Icu_svr::unmask(" << i << ").";
    }
}

/**
 * Icu_svr allows to bind IRQ lines to an IRQ object and to unbind from an IRQ
 * object.
 *
 * Create a local IRQ object. Go through the IRQ lines at the Icu_svr and first
 * bind the IRQ line to the local IRQ object, then unbind the IRQ line from the
 * local IRQ object. Verify that the operation succeeds in both cases.
 */
TEST_F(TestIcuSvr, BindUnbindIrq)
{
  auto irq = Atkins::Factory::del_kobj<L4::Irq>("Emerge IRQ object.");

  for (unsigned i = 0; i < Num_irqs; ++i)
    {
      ASSERT_L4OK(scap()->bind(i, irq.get()))
        << "Bind the Icu_svr IRQ line " << i << " to the IRQ object.";

      ASSERT_L4OK(scap()->unbind(i, irq.get()))
        << "Unbind the Icu_svr IRQ line " << i << " from the IRQ object.";
    }
}

/**
 * Icu_svr allows to set the mode of an IRQ line.
 */
TEST_F(TestIcuSvr, SetMode)
{
  for (unsigned i = 0; i < Num_irqs; ++i)
    {
      ASSERT_L4OK(scap()->set_mode(0, L4_IRQ_F_NONE))
        << "Set interrupt mode at Icu_svr for the IRQ line " << i << ".";
    }
}

/**
 * Triggering an IRQ object through an IRQ line at the Icu_svr works.
 *
 * Create an IRQ object. Bind the first IRQ line at the Icu_svr to that IRQ
 * object. Trigger an IRQ at the first IRQ line at the Icu_svr. Verify that we
 * can receive that interrupt from the local IRQ object. Trigger an IRQ at the
 * second and third IRQ line of the Icu_svr. Verify that we do not receive an
 * interrupt from the local IRQ object.
 */
TEST_F(TestIcuSvr, TriggerIrq)
{
  auto irq = Atkins::Factory::del_kobj<L4::Irq>("Emerge IRQ object.");

  L4Re::chksys(scap()->bind(0, irq.get()),
               "Bind the Icu_svr to the IRQ object.");

  L4Re::chksys(irq->bind_thread(env->main_thread(), 0x0),
               "Bind the main thread to the IRQ object.");

  handler().trigger_irq(0);
  ASSERT_L4OK(irq->receive(L4_IPC_RECV_TIMEOUT_0))
    << "Receive the triggered IRQ.";

  handler().trigger_irq(1);
  handler().trigger_irq(2);
  ASSERT_L4IPC_ERR(L4_IPC_RETIMEOUT, irq->receive(L4_IPC_RECV_TIMEOUT_0))
    << "Don't receive an interrupt from not triggered sources.";
}
