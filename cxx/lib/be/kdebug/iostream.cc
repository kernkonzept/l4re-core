/*
 * (c) 2004-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <l4/cxx/basic_ostream>
#include <l4/sys/kdebug.h>
#include <l4/sys/l4int.h>
#include <l4/util/atomic.h>

#include <stddef.h>

inline void *operator new (size_t, void *p) { return p; }

namespace L4 {

  class KdbgIOBackend : public IOBackend
  {
  protected:
    void write(char const *str, unsigned len) override;
  };

  void KdbgIOBackend::write(char const *str, unsigned len)
  {
    outnstring(str,len);
  }

  typedef char Fake_iobackend[sizeof(KdbgIOBackend)]
    __attribute__((aligned(__alignof__(KdbgIOBackend))));
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
	KdbgIOBackend *iob = new (&_iob) KdbgIOBackend();
	new (&cerr) BasicOStream(iob);
	new (&cout) BasicOStream(iob);
      }
  }
};
