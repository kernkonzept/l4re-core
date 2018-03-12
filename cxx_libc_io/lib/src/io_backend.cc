/*
 * (c) 2004-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#include <cstdio>
#include <l4/cxx/basic_ostream>
#include <l4/util/atomic.h>
#include <stddef.h>

inline void *operator new (size_t, void *p) throw() { return p; }

namespace L4 {

  class LogIOBackend : public IOBackend
  {
  public:
    LogIOBackend(FILE *stream);
  protected:
    void write(char const *str, unsigned len);
  private:
    FILE *stream;
  };

  LogIOBackend::LogIOBackend(FILE *stream)
    : stream(stream)
  {}

  void LogIOBackend::write(char const *str, unsigned len)
  {
    size_t pos = 0;
    while(len)
      {
	size_t l = ::fwrite(str + pos, 1, len, stream);
        pos += l;
	len -= l;
      }
  }

  typedef char Fake_iobackend[sizeof(LogIOBackend)]
    __attribute__((aligned(__alignof__(LogIOBackend))));
  typedef char Fake_ostream[sizeof(BasicOStream)]
    __attribute__((aligned(__alignof__(BasicOStream))));

  Fake_ostream cout;
  Fake_ostream cerr;

  static Fake_iobackend _iob_out;
  static Fake_iobackend _iob_err;

  void iostream_init();

  void iostream_init()
  {
    static l4_umword_t _initialized;
    if (l4util_xchg(&_initialized, 1) == 0)
      {
	new (&cerr) BasicOStream(new (&_iob_err) LogIOBackend(stderr));
	new (&cout) BasicOStream(new (&_iob_out) LogIOBackend(stdout));
      }
  }
};
