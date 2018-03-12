/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
class Start_stop
{
public:
  Start_stop() throw();
  ~Start_stop() throw();
};

static unsigned long const __attribute__((section(".eh_frame_terminator"), used)) __eh_frame_terminator = 0;

extern "C" char __eh_frame_start__[];
extern "C" void __register_frame (const void *begin) throw();
extern "C" void __deregister_frame (const void *begin) throw();

Start_stop::Start_stop() throw()
{
  __register_frame(__eh_frame_start__);
}

Start_stop::~Start_stop() throw()
{
  __deregister_frame(__eh_frame_start__);
}

static Start_stop __start__stop__ __attribute__((init_priority(2005)));

