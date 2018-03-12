/**
 * \file   l4util/lib/src/getopt2.c
 * \brief  initialize argc/argv from multiboot structure
 *
 * \author Frank Mehnert <fm3@os.inf.tu-dresden.de> */

/*
 * (c) 2003-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#include <string.h>
#include <stdio.h>

//#include <l4/util/mbi_argv.h>
//#include <l4/crtn/crt0.h>

#define MAXARGC 50
#define MAXENVC 50

char *l4util_argv[MAXARGC];
int  l4util_argc = 0;

#define isspace(c) ((c)==' '||(c)=='\t'||(c)=='\r'||(c)=='\n')

void __l4util_parse_args(char *argbuf);
void __l4util_parse_args(char *argbuf)
{
  char *cp;
  char quote = 0;

  /* make l4util_argc, l4util_argv */
  l4util_argc = 0;
  cp = argbuf;

  /* Note, there's no support for escaping quotes! */

  while (*cp && l4util_argc < MAXARGC-1)
    {
      /* Skip whitespace */
      while (*cp && isspace(*cp))
	cp++;

      /* New elem? */
      if (*cp)
	{
	  /* Start of a quote? */
	  if (*cp == '"' || *cp == '\'')
	    {
	      quote = *cp;
	      cp++;
	    }

	  l4util_argv[l4util_argc++] = cp;

	  /* Forward to next whitespace / quote character */
	  while (*cp && ((!quote && !isspace(*cp)) || (quote && *cp != quote)))
	    cp++;

	  /* Terminate array elem */
	  if (*cp)
	    *cp++ = '\0';

	  quote = 0;
	}
    }

  if(*cp && l4util_argc == MAXARGC-1)
    printf("WARNING: parse_args() truncated at %dth argument!\n", MAXARGC);
  
  l4util_argv[l4util_argc] = (void*) 0;
}

#if 0
static void
arg_init(char* cmdline)
{
  if (cmdline)
    {
      l4util_parse_args(cmdline);
    }
}

void 
l4util_mbi_to_argv(l4_mword_t flag, l4util_mb_info_t *mbi)
{
  if (flag == L4UTIL_MB_VALID
      && mbi && (mbi->flags & L4UTIL_MB_CMDLINE))
    arg_init((char*)(l4_addr_t)mbi->cmdline);
}
#endif
