/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
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

#include <l4/re/error_helper>
#include <l4/re/util/video/goos_fb>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/env_ns>

#include <l4/re/rm>
#include <l4/re/env>
#include <l4/re/namespace>

namespace L4Re { namespace Util { namespace Video {

void
Goos_fb::init()
{
  using namespace L4Re::Video;
  using L4Re::chksys;
  using L4Re::chkcap;

  Goos::Info gi;
  chksys(_goos->info(&gi), "requesting goos info");

  if (gi.has_dynamic_views())
    {
      chksys(_goos->create_view(&_view), "createing dynamic goos view");
      _flags |= F_dyn_view;
    }
  else // we just assume view 0 to be our's and ignore other possible views
    _view = _goos->view(0);

  View::Info vi;
  chksys(_view.info(&vi), "requesting goos view information");

  _buffer = chkcap(cap_alloc.alloc<L4Re::Dataspace>(),
                   "allocating goos buffer cap");

  if (vi.has_static_buffer())
    chksys(_goos->get_static_buffer(vi.buffer_index, _buffer),
           "requesting static goos buffer");
  else
    {
      unsigned long buffer_sz = gi.pixel_info.bytes_per_pixel() * gi.width * gi.height;
      _buffer_index = chksys(_goos->create_buffer(buffer_sz, _buffer),
                             "allocating goos buffer");
      _flags |= F_dyn_buffer;

      // use the allocated buffer, at offset 0
      vi.buffer_index = _buffer_index;
      vi.buffer_offset = 0;
      vi.pixel_info = gi.pixel_info;
      vi.bytes_per_line = gi.width * gi.pixel_info.bytes_per_pixel();

      // we want a fullscreen view
      vi.xpos = 0;
      vi.ypos = 0;
      vi.width = gi.width;
      vi.height = gi.height;

      chksys(_view.set_info(vi), "setting up dynamic view");
      chksys(_view.push_top(), "bringing view to top");
    }
}

void
Goos_fb::setup(L4::Cap<L4Re::Video::Goos> goos)
{
  _goos = goos;
  init();
}

void
Goos_fb::setup(char const *name)
{
  Env_ns ns;
  //_goos = chkcap(cap_alloc.alloc<L4Re::Video::Goos>(), "allocating goos cap");
  _goos = chkcap(ns.query<L4Re::Video::Goos>(name), "requesting goos cap", 0);
  _flags |= F_dyn_goos;

  //chksys(L4Re::Env::env()->names()->query(name, _goos), "requesting goos service");
  init();
}

Goos_fb::Goos_fb(L4::Cap<L4Re::Video::Goos> goos)
: _goos(goos), _buffer(L4_INVALID_CAP), _flags(0)
{ init(); }


Goos_fb::Goos_fb(char const *name)
: _goos(L4_INVALID_CAP), _buffer(L4_INVALID_CAP), _flags(0)
{ setup(name); }

void *
Goos_fb::attach_buffer()
{
  void *fb_addr = 0;
  L4Re::chkcap(_goos);
  L4Re::chksys(L4Re::Env::env()->rm()->attach(&fb_addr, _buffer->size(), L4Re::Rm::Search_addr, _buffer, 0, L4_SUPERPAGESHIFT), "attaching frame-buffer memory");
  return fb_addr;
}

Goos_fb::~Goos_fb()
{
  if (!_goos.is_valid())
    return;

  if (_flags & F_dyn_view)
    _goos->delete_view(_view);

  if (_flags & F_dyn_buffer)
    _goos->delete_buffer(_buffer_index);

  if (_buffer.is_valid())
    cap_alloc.free(_buffer);

  if (_flags & F_dyn_goos)
    cap_alloc.free(_goos);
}


}}}
