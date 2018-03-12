/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/util/mb_info.h>
#include <l4/sys/consts.h>

#include <l4/re/dataspace>
#include <l4/re/video/goos>

#include <l4/sigma0/sigma0.h>

#include <l4/cxx/iostream>
#include <l4/cxx/l4iostream>

#include "dataspace_static.h"
#include "globals.h"
#include "name_space.h"
#include "page_alloc.h"
#include "pages.h"
#include "vesa_fb.h"

#include <l4/re/util/video/goos_svr>
#include <l4/sys/cxx/ipc_epiface>

using L4Re::Dataspace;

class Vesa_fb :
  public L4Re::Util::Video::Goos_svr,
  public L4::Epiface_t<Vesa_fb, L4Re::Video::Goos, Moe::Server_object>
{
private:
  l4util_mb_vbe_ctrl_t *vbe;
  l4util_mb_vbe_mode_t *vbi;
  unsigned long base_offset;
  unsigned long map_size;

public:
  Vesa_fb(l4util_mb_info_t *mbi);
  virtual ~Vesa_fb() {}
};

void
init_vesa_fb(l4util_mb_info_t *mbi)
{
  static Vesa_fb video(mbi);
  (void)video;
}

Vesa_fb::Vesa_fb(l4util_mb_info_t *mbi)
{
  if (!(mbi->flags & L4UTIL_MB_VIDEO_INFO))
    return;
  vbe = (l4util_mb_vbe_ctrl_t*)(unsigned long)mbi->vbe_ctrl_info;
  vbi = (l4util_mb_vbe_mode_t*)(unsigned long)mbi->vbe_mode_info;
  if (!vbe || !vbi)
    return;

  base_offset = vbi->phys_base & (L4_SUPERPAGESIZE - 1);
  unsigned long paddr = vbi->phys_base & ~(L4_SUPERPAGESIZE - 1);
  unsigned long fb_size = 64 * 1024 * vbe->total_memory;
  map_size = (fb_size + base_offset + L4_SUPERPAGESIZE - 1)
             & ~(L4_SUPERPAGESIZE - 1);

  unsigned long vaddr
      = l4_round_size(Moe::Pages::max_addr, L4_SUPERPAGESHIFT);
  if (vaddr >= Moe::Virt_limit::end - map_size - L4_SUPERPAGESIZE)
    vaddr = (unsigned long)Single_page_alloc_base::_alloc(map_size);
  if (vaddr == 0)
    {
      L4::cerr << "Failed to get memory for VESA video memory\n";
      return;
    }

  switch (l4sigma0_map_iomem(Sigma0_cap, paddr, vaddr, map_size, 1)) 
    {
    case -2:
      L4::cerr << "IPC error mapping video memory\n";
      return;
    case -3:
      L4::cerr << "No fpage received\n";
      return;
    default:
      break;
    }

  Moe::Dataspace_static *fb = new Moe::Dataspace_static((void*)vaddr, map_size, Moe::Dataspace_static::Writable | L4Re::Dataspace::Map_bufferable);

  _screen_info.width = vbi->x_resolution;
  _screen_info.height = vbi->y_resolution;
  _screen_info.flags = L4Re::Video::Goos::F_auto_refresh;
  _screen_info.pixel_info = L4Re::Video::Pixel_info(vbi);

  _view_info.buffer_offset = base_offset;
  _view_info.bytes_per_line = vbi->bytes_per_scanline;

  init_infos();

  _fb_ds = L4::cap_cast<L4Re::Dataspace>(object_pool.cap_alloc()->alloc(fb));

  object_pool.cap_alloc()->alloc(this);
  root_name_space()->register_obj("vesa", 0, this);

  L4::cout << "  VESAFB: " << obj_cap() << _fb_ds
    << " @" << (void*)(unsigned long)vbi->phys_base
    << " (size=" << L4::hex << 64*1024*vbe->total_memory << ")\n" << L4::dec;

}

