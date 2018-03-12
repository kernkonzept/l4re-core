/*
 * (c) 2004-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include <l4/cxx/basic_ostream>
#include <l4/sys/vcon>
#include <l4/util/atomic.h>
#include <stddef.h>

inline void *operator new (size_t, void *p) { return p; }

namespace L4 {

  class LogIOBackend : public IOBackend
  {
  public:
    LogIOBackend();
  protected:
    void write(char const *str, unsigned len);
  };

  LogIOBackend::LogIOBackend()
  {}

  void LogIOBackend::write(char const *str, unsigned len)
  {
    l4_msg_regs_t store;
    l4_msg_regs_t *mr = l4_utcb_mr();
    unsigned l = len;
    L4::Cap<L4::Vcon> log(L4_BASE_LOG_CAP);

    __builtin_memcpy(&store, mr, sizeof(store));

    while (len)
      {
	l = len;
	if (l > L4_VCON_WRITE_SIZE)
	  l = L4_VCON_WRITE_SIZE;
	log->write(str, l);
	len -= l;
        str += l;
      }

    __builtin_memcpy(mr, &store, sizeof(store));
  }

  typedef char Fake_iobackend[sizeof(LogIOBackend)]
    __attribute__((aligned(__alignof__(LogIOBackend))));
  typedef char Fake_ostream[sizeof(BasicOStream)]
    __attribute__((aligned(__alignof__(BasicOStream))));

  Fake_ostream cout;
  Fake_ostream cerr;

  static Fake_iobackend _iob;

  void iostream_init();

  void iostream_init()
  {
    static l4_umword_t _initialized;
    if (l4util_xchg(&_initialized, 1) == 0)
      {
	LogIOBackend *iob = new (&_iob) LogIOBackend();
	new (&cerr) BasicOStream(iob);
	new (&cout) BasicOStream(iob);
      }
  }
};
