/* SPDX-License-Identifier: GPL-2.0-only or License-Ref-kk-custom */
/*
 * Copyright (C) 2020 Kernkonzept GmbH.
 * Author(s): Frank Mehnert <frank.mehnert@kernkonzept.com>
 *
 */

#include <cstdlib>
#include <exception>
#include <bits/exception_defines.h>
#include <cxxabi.h>
#include <cstdio>

#include <l4/cxx/exceptions>

namespace std { namespace L4 {

// Verbose terminate handler being aware of the std::L4 exceptions. We have to
// consider the different layout of L4 exceptions depending on the definition of
// L4_CXX_EXCEPTION_BACKTRACE.
#ifdef L4_CXX_EXCEPTION_BACKTRACE
void terminate_handler_exc_backtrace();
void terminate_handler_exc_backtrace()
#else
void terminate_handler_no_exc_backtrace();
void terminate_handler_no_exc_backtrace()
#endif
{
  static bool terminating;
  if (terminating)
    {
      fputs("terminate called recursively\n", stderr);
      abort ();
    }
  terminating = true;

  // Make sure there was an exception; terminate is also called for an
  // attempt to rethrow when there is no suitable exception.
  type_info *t = __cxxabiv1::__cxa_current_exception_type();
  if (t)
    {
      // Note that "name" is the mangled name.
      char const *name = t->name();
        {
          int status = -1;
          char *dem = 0;

          dem = __cxxabiv1::__cxa_demangle(name, 0, 0, &status);

          fputs("terminate called after throwing an instance of '", stderr);
          if (status == 0)
            fputs(dem, stderr);
          else
            fputs(name, stderr);
          fputs("'\n", stderr);

          if (status == 0)
            free(dem);
        }

      // If the exception is derived from std::exception, we can
      // give more information.
      __try { __throw_exception_again; }
      __catch (::L4::Runtime_error const &e)
        {
          char const *s = e.str();
          char const *es = e.extra_str();
          fputs("  what: ", stderr);
          fputs(s, stderr);
          fputs(": ", stderr);
          fputs(es, stderr);
          fputs("\n", stderr);
        }
      __catch (::L4::Base_exception const &e)
        {
          char const *s = e.str();
          fputs("  what: ", stderr);
          fputs(s, stderr);
          fputs("\n", stderr);
        }
      __catch(::std::exception const &exc)
        {
          char const *w = exc.what();
          fputs("  what():  ", stderr);
          fputs(w, stderr);
          fputs("\n", stderr);
        }
      __catch(...) { }
    }
  else
    fputs("terminate called without an active exception\n", stderr);

  abort();
}

} }
