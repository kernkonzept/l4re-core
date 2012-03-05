/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */

#include <l4/re/env>
#include <l4/sys/factory>

#include "vcon_stream.h"
#include "vfs_api.h"

#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/ttydefaults.h>

namespace L4Re { namespace Core {
Vcon_stream::Vcon_stream(L4::Cap<L4::Vcon> s) throw()
: Be_file_stream(), _s(s), _irq(cap_alloc()->alloc<L4::Semaphore>())
{
  //printf("VCON: irq cap = %lx\n", _irq.cap());
  int res = l4_error(L4Re::Env::env()->factory()->create(_irq));
  //printf("VCON: irq create res=%d\n", res);

  if (res < 0)
    return; // handle errors!!!

  res = l4_error(_s->bind(0, _irq));
  //printf("VCON: bound irq to con res=%d\n", res);
}

ssize_t
Vcon_stream::readv(const struct iovec *iovec, int iovcnt) throw()
{
  ssize_t bytes = 0;
  for (; iovcnt > 0; --iovcnt, ++iovec)
    {
      if (iovec->iov_len == 0)
	continue;

      char *buf = (char *)iovec->iov_base;
      size_t len = iovec->iov_len;

      while (1)
	{
	  int ret = _s->read(buf, len);

	  // BS: what is this ??
	  if (ret > (int)len)
	    ret = len;

	  if (ret < 0)
	    return ret;
	  else if (ret == 0)
	    {
	      if (bytes)
		return bytes;

	      ret = _s->read(buf, len);
	      if (ret < 0)
		return ret;
	      else if (ret == 0)
		{
		  _irq->down();
		  continue;
		}
	    }

	  bytes += ret;
	  len   -= ret;
	  buf   += ret;

	  if (len == 0)
	    break;
	}
    }

  return bytes;
}

ssize_t
Vcon_stream::writev(const struct iovec *iovec, int iovcnt) throw()
{
  l4_msg_regs_t store;
  l4_msg_regs_t *mr = l4_utcb_mr();

  Vfs_config::memcpy(&store, mr, sizeof(store));

  ssize_t written = 0;
  while (iovcnt)
    {
      size_t sl = iovec->iov_len;
      char const *b = (char const *)iovec->iov_base;

      for (; sl > L4_VCON_WRITE_SIZE
           ; sl -= L4_VCON_WRITE_SIZE, b += L4_VCON_WRITE_SIZE)
        _s->send(b, L4_VCON_WRITE_SIZE);

      _s->send(b, sl);

      written += iovec->iov_len;

      ++iovec;
      --iovcnt;
    }
  Vfs_config::memcpy(mr, &store, sizeof(store));
  return written;
}

int
Vcon_stream::fstat64(struct stat64 *buf) const throw()
{
  buf->st_size = 0;
  buf->st_mode = 0666;
  buf->st_dev = _s.cap();
  buf->st_ino = 0;
  return 0;
}

int
Vcon_stream::ioctl(unsigned long request, va_list args) throw()
{
  switch (request) {
    case TCGETS:
	{
	  //vt100_tcgetattr(term, (struct termios *)argp);

	  struct termios *t = va_arg(args, struct termios *);

	  l4_vcon_attr_t l4a;
	  if (!l4_error(_s->get_attr(&l4a)))
            {
              t->c_iflag = l4a.i_flags;
              t->c_oflag = l4a.o_flags; // output flags
              t->c_cflag = 0; // control flags
              t->c_lflag = l4a.l_flags; // local flags
            }
          else
            t->c_iflag = t->c_oflag = t->c_cflag = t->c_lflag = 0;
#if 0
	  //t->c_lflag |= ECHO; // if term->echo
	  t->c_lflag |= ICANON; // if term->term_mode == VT100MODE_COOKED
#endif

	  t->c_cc[VEOF]   = CEOF;
	  t->c_cc[VEOL]   = _POSIX_VDISABLE;
	  t->c_cc[VEOL2]  = _POSIX_VDISABLE;
	  t->c_cc[VERASE] = CERASE;
	  t->c_cc[VWERASE]= CWERASE;
	  t->c_cc[VKILL]  = CKILL;
	  t->c_cc[VREPRINT]=CREPRINT;
	  t->c_cc[VINTR]  = CINTR;
	  t->c_cc[VQUIT]  = _POSIX_VDISABLE;
	  t->c_cc[VSUSP]  = CSUSP;
	  t->c_cc[VSTART] = CSTART;
	  t->c_cc[VSTOP] = CSTOP;
	  t->c_cc[VLNEXT] = CLNEXT;
	  t->c_cc[VDISCARD]=CDISCARD;
	  t->c_cc[VMIN] = CMIN;
	  t->c_cc[VTIME] = 0;

	}

      return 0;

    case TCSETS:
    case TCSETSW:
    case TCSETSF:
	{
	  //vt100_tcsetattr(term, (struct termios *)argp);
	  struct termios const *t = va_arg(args, struct termios const *);

	  // XXX: well, we're cheating, get this from the other side!

	  l4_vcon_attr_t l4a;
	  l4a.i_flags = t->c_iflag;
	  l4a.o_flags = t->c_oflag; // output flags
	  l4a.l_flags = t->c_lflag; // local flags
	  _s->set_attr(&l4a);
	}
      return 0;

    default:
      break;
  };
  return -EINVAL;
}

}}
