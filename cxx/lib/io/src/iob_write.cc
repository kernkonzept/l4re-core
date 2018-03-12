/*
 * (c) 2004-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Frank Mehnert <fm3@os.inf.tu-dresden.de>
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

#include <l4/cxx/basic_ostream>

namespace L4 
{

  IOModifier const hex(16);
  IOModifier const dec(10);

  static char const hex_chars[] = "0123456789abcdef";

  void IOBackend::write(IOModifier m)
  {
    if(m == dec)
      int_mode = 10;
    else if(m == hex)
      int_mode = 16;
  }

  void IOBackend::write(long long int u, int /*len*/)
  {
    char buffer[20];
    int  pos = 20;
    bool sign = u < 0;
    if(sign) u=-u;
    buffer[19] = '0';
    while(u)
      {
        buffer[--pos] = hex_chars[(u % int_mode)];
        u /= int_mode;
      }
    if(pos==20)
      pos = 19;
    if(sign)
      buffer[--pos] = '-';

    write(buffer + pos, 20-pos);
  }
  
  void IOBackend::write(long long unsigned u, int /*len*/)
  {
    char buffer[20];
    int  pos = 20;
    buffer[19] = '0';
    while(u)
      {
        buffer[--pos] = hex_chars[(u % int_mode)];
        u /= int_mode;
      }
    if(pos==20)
      pos = 19;

    write(buffer + pos, 20-pos);
  }

  void IOBackend::write(long long unsigned u, unsigned char base,
                        unsigned char len, char pad)
  {
    char buffer[30];
    unsigned pos = sizeof(buffer);
    buffer[sizeof(buffer)-1] = '0';
    while (pos > 0 && u)
      {
        buffer[--pos] = hex_chars[(u % base)];
        u /= base;
      }

    if (len >sizeof(buffer))
      len = sizeof(buffer);

    if (len && sizeof(buffer)-pos > len)
      pos = sizeof(buffer) - len;
    
    while (sizeof(buffer)-pos < len)
      buffer[--pos] = pad;
    
    if (pos==sizeof(buffer))
      pos--;

    write(buffer + pos, sizeof(buffer)-pos);
  }
};
