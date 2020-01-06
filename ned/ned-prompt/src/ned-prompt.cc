// SPDX-License-Identifier: GPL-2.0-only or License-Ref-kk-custom
/*
 * Copyright (C) 2020 Kernkonzept GmbH.
 * Author(s): Adam Lackorzynski <adam.lackorzynski@kernkonzept.com>
 *            Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/**
 *
 * A command line prompt using ned's server interface.
 *
 * Usage: ned-prompt [<prompt>]
 *
 * The capability to the ned server is expected under the name srv.
 * Optionally a different prompt than 'Cmd>' may be set via the command
 * line.
 */

#include <cstdio>
#include <cstdlib>

#include <readline/history.h>
#include <readline/readline.h>

#include <l4/ned/cmd_control>
#include <l4/re/env>
#include <l4/re/error_helper>

/**
 * Add a line to the readline history.
 *
 * This function mimicks the 'ignoreboth' HISTCONTROL setting.
 */
static void add_to_history(char *line)
{
  if (line[0] == ' ')
    return;

  HIST_ENTRY **hist_list = history_list();
  if (hist_list)
    {
      HIST_ENTRY *hist = hist_list[history_length - 1];
      if (hist && strcmp(line, hist->line) == 0)
        return;
    }

  add_history(line);
}

static void run_cmds(char const* prompt)
{
  char fullprompt[64];

  snprintf(fullprompt, sizeof(fullprompt), "%s ", prompt);

  using L4Re::Ned::Cmd_control;
  auto srv = L4Re::chkcap(L4Re::Env::env()->get_cap<Cmd_control>("svr"),
                          "Searching ned command capability 'svr'");

  for (;;)
    {
      char *cmd;
      do
        {
         cmd = readline(fullprompt);
        }
      while (!cmd);

      if (*cmd)
        {
          char buffer[L4_UTCB_GENERIC_DATA_SIZE * sizeof(l4_umword_t)];
          L4::Ipc::String<> c(cmd);
          L4::Ipc::String<char> result(sizeof(buffer), buffer);
          int e = srv->execute(c, &result);
          if (e < 0)
            printf("Error calling ned: %d\n", e);
          else if (result.length != 3 || strncmp("nil", result.data, 3) != 0)
            printf("%.*s\n", (int)result.length, result.data);
          add_to_history(cmd);
        }

      free(cmd);
    }
}

int main(int argc, char *argv[])
{
  printf("Welcome to ned prompt.\n");
  try
    {
      run_cmds(argc < 2 ? "Cmd>" : argv[1]);
    }
  catch (L4::Runtime_error &e)
    {
      printf("%s: %s\n", e.str(), e.extra_str());
      return 1;
    }

  return 0;
}
