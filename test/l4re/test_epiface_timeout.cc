/*
 * Copyright (C) 2016 Kernkonzept GmbH.
 * Author(s): Jean Wolter <jean.wolter@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/**
 * Test the timeout handling of the L4Re::Util::Registry_server.
 */
#include <l4/atkins/tap/main>

#include <l4/re/util/object_registry>
#include <l4/re/util/br_manager>
#include <l4/sys/cxx/ipc_epiface>
#include <l4/cxx/unique_ptr>

enum
{
  Num_timeouts = 5,
  Millisecond = 1000
};

/*
 * Definition of registry with overwritten error hook used to forward
 * IPC errors to the test class
*/
struct My_br_manager_timeout_hooks : L4Re::Util::Br_manager_timeout_hooks
{
  static void error(l4_msgtag_t tag, l4_utcb_t *);
};

/*
 * Timeout class used to
 * - construct objects added to the timeout queue
 * - store expected timeout value
 * - forward expire events to the test class
 */
struct Test_timeout : public L4::Ipc_svr::Timeout_queue::Timeout
{
  l4_kernel_clock_t expected;
  void expired();
  Test_timeout() : expected{0} {}
  Test_timeout(l4_kernel_clock_t to) : expected{to} {}
};

class EpifaceTimeout : public ::testing::Test
{
public:
  // invoked before running a test
  void SetUp() override;

  // invoked after the test finished
  void TearDown() override;

  // the test function
  void run(unsigned const *add, unsigned number, unsigned start,
           bool delete_first, unsigned expected_ipc_errors);

  void timeout_expired(Test_timeout const *to);

  void ipc_error(l4_msgtag_t tag);

  static EpifaceTimeout *test_class;

private:
  void add_timeouts(unsigned const *add, unsigned number, unsigned start,
                    bool delete_first, unsigned expected_ipc_errors);

  Test_timeout _timeouts[Num_timeouts];
  unsigned _to_index;
  unsigned _last_timeout;

  unsigned _num_ipc_errors;
  unsigned _expected_ipc_errors;

  L4Re::Util::Registry_server<My_br_manager_timeout_hooks> server;
};

EpifaceTimeout *EpifaceTimeout::test_class;

// Exception used to terminate the server loop regularily
class Terminate_ok
{
};

/*
 * Forward timeout errors and expired timeouts to test class
 */
void
My_br_manager_timeout_hooks::error(l4_msgtag_t tag, l4_utcb_t *)
{ EpifaceTimeout::test_class->ipc_error(tag); }

void
Test_timeout::expired()
{ EpifaceTimeout::test_class->timeout_expired(this); }

void
EpifaceTimeout::SetUp()
{
  // sorted timeouts, first timeout in the past
  auto current = l4_kip_clock(l4re_kip());
  _timeouts[0] = Test_timeout(current - 1   * Millisecond);
  _timeouts[1] = Test_timeout(current + 200 * Millisecond);
  _timeouts[2] = Test_timeout(current + 400 * Millisecond);
  _timeouts[3] = Test_timeout(current + 400 * Millisecond);
  _timeouts[4] = Test_timeout(current + 600 * Millisecond);

  test_class = this;
}

void
EpifaceTimeout::TearDown()
{ test_class = nullptr; }

void
EpifaceTimeout::timeout_expired(Test_timeout const *to)
{
  // timeouts with the same timeout value might be re-ordered in the
  // queue so we do not fail if 'to' does not match '&timeouts[to_index]'
  // EXPECT_EQ(to, &timeouts[to_index]);

  // Since one IPC timeout might expire several timeouts we check them
  // here too (again)
  l4_kernel_clock_t current_time = l4_kip_clock(l4re_kip());
  EXPECT_LE(to->expected, current_time);

  if (++_to_index >= _last_timeout)
    {
      EXPECT_EQ(_expected_ipc_errors, _num_ipc_errors);
      throw Terminate_ok();
    }
}

void
EpifaceTimeout::ipc_error(l4_msgtag_t tag)
{
  /*
   * The error hook, checks whether
   * - we actually see a an ipc timeout
   * - the timeout, that is triggered, is actually the one we expect
   *   (timeouts[_to_index])
   * - the timeout did not trigger to early
   */
  if (l4_error(tag) != -(L4_EIPC_LO + L4_IPC_RETIMEOUT))
    throw L4::Runtime_error(-L4_EINVAL, "Unexpected error");

  ++_num_ipc_errors;
  // timeout, that has triggered according to ipc timeout queue
  l4_kernel_clock_t next_to = server.queue.next_timeout();

  // timeout, that we expect to have triggered
  l4_kernel_clock_t expected_to = _timeouts[_to_index].expected;

  l4_kernel_clock_t current_time = l4_kip_clock(l4re_kip());

  // printf("timeouts[%d] = %lld == %lld <= %lld\n",
  //         _to_index, expected_to, next_to, current_time);
  EXPECT_EQ(expected_to, next_to);
  EXPECT_LE(next_to, current_time);

  if (next_to == expected_to && next_to <= current_time)
    return;

  throw L4::Runtime_error(-L4_EINVAL, "Unexpected timeout");
}

void
EpifaceTimeout::add_timeouts(unsigned const *add, unsigned number,
                             unsigned start, bool delete_first,
                             unsigned expected_ipc_errors)
{
  assert(start + number <= Num_timeouts);
  ASSERT_EQ(server.queue.next_timeout(), (l4_kernel_clock_t)0);

  Test_timeout first;

  _num_ipc_errors = 0;
  _expected_ipc_errors = expected_ipc_errors;

  _to_index = start;
  _last_timeout = start + number;

  for (unsigned i = 0; i < number; ++i)
    {
      assert(add[i] <= Num_timeouts);
      auto to = &_timeouts[add[i]];

      if (delete_first && (i == 0))
        {
          // use a locally allocated object, that goes out of scope to
          // test the removal of a timeout due to destruction
          first = Test_timeout(to->expected);
          to = &first;
          ++_to_index;
        }
      // printf("Add: %p:%p:%lld\n", timeouts + add[i], to, to->expected);
      server.add_timeout(to, to->expected);
    }
}

void
EpifaceTimeout::run(unsigned const *add, unsigned number, unsigned start,
                    bool delete_first, unsigned expected_ipc_errors)
{
  add_timeouts(add, number, start, delete_first, expected_ipc_errors);
  try
    {
      server.loop();
      FAIL();
    }
  catch (Terminate_ok)
    {
      // nothing to do
    }
}

template<typename T, unsigned N>
static unsigned
array_length(T (&)[N])
{ return N; }

/**
 * Add an already expired timeout to the timeout queue and expect to be
 * notified, but don't expect an timeout error.
 */
TEST_F(EpifaceTimeout, TimeoutAlreadyExpired)
{
  unsigned add[] = {0};
  run(add, array_length(add), 0, false, 0);
}

/**
 * Add a sorted list of timeouts to the timeout queue and expect them to expire
 * in natural order and to receive the one timeout error for each timeout value
 * in the future exactly once. Expect an expired notification for each timeout
 * passed.
 */
TEST_F(EpifaceTimeout, TimeoutSorted)
{
  unsigned add[] = {0, 1, 2, 3, 4};
  run(add, array_length(add), 0, false, 3);
}

/**
 * Add an unsorted list of timeouts to the timeout queue and expect them to
 * expire in natural order and to receive the one timeout error for each
 * timeout value in the future exactly once. Expect an expired notification for
 * each timeout passed.
 */
TEST_F(EpifaceTimeout, TimeoutUnsorted)
{
  unsigned add[] = {1, 0, 4, 3, 2};
  run(add, array_length(add), 0, false, 3);
}

/**
 * Add a list of timeouts in reverse order to the timeout queue and expect them
 * to expire in natural order and to receive the one timeout error for each
 * timeout value in the future exactly once. Expect an expired notification for
 * each timeout passed.
 *
 */
TEST_F(EpifaceTimeout, TimeoutReverse)
{
  unsigned add[] = {4, 3, 2, 1, 0};
  run(add, array_length(add), 0, false, 3);
}

/**
 * Add a list of timeouts to the timeout queue and destruct an added timeout
 * object. Expect that the destructed timeout does neither trigger an timeout
 * error nor an expired notification. All other timeouts should raise timeout
 * errors and notifications as usual.
 */
TEST_F(EpifaceTimeout, DeletedTimeout)
{
  unsigned add[] = {1, 2, 3, 4};
  run(add, array_length(add), 1, true, 2);
}
