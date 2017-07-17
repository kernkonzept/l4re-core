/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/re/log>
#include <l4/re/log-sys.h>
#include <l4/sys/kdebug.h>
#include <l4/cxx/minmax>

#include "globals.h"
#include "log.h"

#include <unistd.h>
#include <cstdio>
#include <stdarg.h>

static Moe::Log *last_log = 0;

class Pbuf
{
public:
  Pbuf() : _p(0) {}
  unsigned long size() const { return sizeof(_b); }
  void flush();
  void printf(char const *fmt, ...)
    __attribute__((format(printf, 2, 3)));
  void outnstring(char const *str, unsigned long len);

private:
  void checknflush(int n);
  char _b[1024];
  unsigned long _p;

  bool fits(unsigned l) const { return (_p + l) < sizeof(_b); }
};

static void my_outnstring(char const *s, unsigned long len)
{
  write(1, s, len);
}



void Pbuf::flush()
{
  my_outnstring(_b, _p);
  _p = 0;
}

void Pbuf::checknflush(int n)
{
  char *x = 0;
  x = (char*)memchr(_b, '\n', _p + n);

  if (x)
    {
      int rem = n - (x - _b + 1 - _p);
      _p = x - _b + 1;
      flush();
      if (rem)
        memmove(_b, x + 1, rem);
      _p = rem;
    }
  else
    _p += n;
}

void Pbuf::printf(char const *fmt, ...)
{
  if (!fits(strlen(fmt) + 50))
    flush();
  int n;
  va_list arg;
  va_start(arg, fmt);
  n = vsnprintf(_b + _p, size() - _p, fmt, arg);
  va_end(arg);
  checknflush(n);
}

void Pbuf::outnstring(char const *str, unsigned long len)
{
  if (!fits(len))
    flush();
  memcpy(_b + _p, str, len);
  checknflush(len);
}

l4_msgtag_t
Moe::Log::op_dispatch(l4_utcb_t *utcb, l4_msgtag_t tag, L4::Vcon::Rights)
{
  enum { Max_tag = 8 };
  if (tag.words() < 2)
    return l4_msgtag(-L4_EINVAL, 0, 0, 0);

  l4_msg_regs_t *m = l4_utcb_mr_u(utcb);
  L4::Opcode op = m->mr[0];

  // we only have one opcode
  if (op != L4Re::Log_::Print)
    return l4_msgtag(-L4_ENOSYS, 0, 0, 0);

  char *msg = log_buffer;
  unsigned long len_msg = sizeof(log_buffer);

  if (len_msg > (tag.words() - 2) * sizeof(l4_umword_t))
    len_msg = (tag.words() - 2) * sizeof(l4_umword_t);

  if (len_msg > m->mr[1])
    len_msg = m->mr[1];

  memcpy(msg, &m->mr[2], len_msg);

  static Pbuf ob;

  while (len_msg > 0 && msg[0])
    {
      if (color())
        ob.printf("\033[%s3%dm", (color() & 8) ? "01;" : "", (color() & 7));
      else
        ob.printf("\033[0m");

      if (last_log != this)
        {
          if (last_log != 0)
            ob.printf("\n");

          ob.outnstring(_tag, cxx::min<unsigned long>(_l, Max_tag));
          if (_l < Max_tag)
            ob.outnstring("             ", Max_tag-_l);

          if (_in_line)
            ob.printf(": ");
          else
            ob.printf("| ");
        }

      long i;
      for (i = 0; i < (long)len_msg; ++i)
        if (msg[i] == '\n' || msg[i] == 0)
          break;

      ob.outnstring(msg, i);

      if (i < (long)len_msg && msg[i] == '\n')
        {
          if (color())
            ob.printf("\033[0m\n");
          else
            ob.printf("\n");
          _in_line = false;
          last_log = 0;
          ++i;
        }
      else
        {
          last_log = this;
          _in_line = true;
        }

      msg += i;
      len_msg -= i;
    }

  if (_in_line && color())
    ob.printf("\033[0m");

  // and finally done
  return l4_msgtag(-L4_ENOREPLY, 0, 0, 0);
}


int
Moe::Log::color_value(cxx::String const &col)
{
  int c = 0, bright = 0;

  if (col.empty())
    return 0;

  switch (col[0])
    {
    case 'N': bright = 1; /* FALLTHRU */ case 'n': c = 0; break;
    case 'R': bright = 1; /* FALLTHRU */ case 'r': c = 1; break;
    case 'G': bright = 1; /* FALLTHRU */ case 'g': c = 2; break;
    case 'Y': bright = 1; /* FALLTHRU */ case 'y': c = 3; break;
    case 'B': bright = 1; /* FALLTHRU */ case 'b': c = 4; break;
    case 'M': bright = 1; /* FALLTHRU */ case 'm': c = 5; break;
    case 'C': bright = 1; /* FALLTHRU */ case 'c': c = 6; break;
    case 'W': bright = 1; /* FALLTHRU */ case 'w': c = 7; break;
    default: c = 0;
    }

  return (bright << 3) | c;
}


