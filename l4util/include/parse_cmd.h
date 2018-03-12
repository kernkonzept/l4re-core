/**
 * \file
 * \brief  comfortable command-line parsing
 *
 * \date   2002
 * \author Jork Loeser <jork.loeser@inf.tu-dresden.de>
 */

/*
 * (c) 2003-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#ifndef __PARSE_CMD_H
#define __PARSE_CMD_H

#include <stdarg.h>
#include <l4/sys/compiler.h>

/**
 * \defgroup l4util_parse_cmd Comfortable Command Line Parsing
 * \ingroup l4util_api
 */
/*@{*/

/**
 * \brief Types for parsing
 */
enum parse_cmd_type {
    PARSE_CMD_INT,
    PARSE_CMD_SWITCH,
    PARSE_CMD_STRING,
    PARSE_CMD_FN,
    PARSE_CMD_FN_ARG,
    PARSE_CMD_INC,
    PARSE_CMD_DEC,
};

/**
 * \brief Function type for PARSE_CMD_FN
 */
typedef L4_CV void (*parse_cmd_fn_t)(int);

/**
 * \brief Function type for PARSE_CMD_FN_ARG
 */
typedef L4_CV void (*parse_cmd_fn_arg_t)(int, const char*, int);

EXTERN_C_BEGIN

/** Parse the command-line for specified arguments and store the values into
 * variables.
 *
 * This Functions gets the command-line, and a list of
 * command-descriptors. Then, the command-line is parsed according to
 * the given descriptors, storing strings, switches and numeric
 * arguments at given addresses, and possibly calling specified
 * functions. A default help descriptor is added. Its purpose is to
 * present a short command overview in the case the given command-line
 * does not fit to the descriptors.
 *
 * Each command-descriptor has the following form:
 *
 * \e short \e option \e char, \e long \e option \e name, 
 * \e comment, \e type, \e val, \e addr.
 *
 * The \e short \e option \e char specifies the short form of the
 * described option. The short form will be recognized after a single
 * dash, or in a group of short options preceeded by a single dash. 
 * Specify ' ' if no short form should be used.
 * 
 * The \e long \e option \e name specifies the long form of the
 * described option. The long form will be recognized after two
 * dashes. Specify 0 if no long form should be used for this option.
 *
 * The \e comment is a string that will be used when presenting the short
 * command-line help.
 *
 * The \e type specifies, if the option should be recognized as
 * - a number (\c PARSE_CMD_INT),
 * - a switch (\c PARSE_CMD_SWITCH),
 * - a string (\c PARSE_CMD_STRING),
 * - a function call (\c PARSE_CMD_FN, \c PARSE_CMD_FN_ARG),
 * - an increment/decrement operator (\c PARSE_CMD_INC, \c PARSE_CMD_DEC).
 *
 * If type is \c PARSE_CMD_INT, the option requires a second argument
 * on the command-line after the option. This argument is parsed as a
 * number. It can be preceeded by 0x to present a hex-value or by 0 to
 * present an octal form. \e addr is interpreted as an int-pointer. 
 * The scanned argument from the command-line is stored in this
 * pointer.
 *
 * If \e type is \c PARSE_CMD_SWITCH, \e addr must be a pointer to
 * int, and the value from \e val is stored at this pointer.
 *
 * With \c PARSE_CMD_STRING, an additional argument is expected at the
 * cmdline. \e addr must be a pointer to const char*, and a pointer to
 * the argument on the command line is stored at this pointer. The
 * value in \e val is a default value, which is stored at \e addr if
 * the corresponding option is not given on the command line.
 *
 * With \c PARSE_CMD_FN_ARG, \e addr is interpreted as a function
 * pointer of type #parse_cmd_fn_t. It will be called with \e val as
 * argument if the corresponding option is found.
 *
 * If \e type is \c PARSE_CMD_FN_ARG, \e addr is as a function pointer
 * of type #parse_cmd_fn_arg_t, and handled similar to 
 * \c PARSE_CMD_FN. An additional argument is expected at the command
 * line, however. It is given to the called function as 2nd argument,
 * and parsed as an integer as with \c PARSE_CMD_INT as a third
 * argument.
 *
 * If \e type is \c PARSE_CMD_INC or \c PARSE_CMD_DEC, \e addr is
 * interpreted as an int-pointer. The value of \e val is stored to
 * this pointer first. For every occurence of the option in the
 * command line, the integer referenced by \e addr is incremented or
 * decremented, respectively.
 *
 * The list of command-descriptors is terminated by specifying a binary 0 for
 * the short option char.
 *
 * Note: The short option char 'h' and the long option name "help" must not be
 * specified. They are used for the default help descriptor and produce a short
 * command-options help when specified on the command-line.
 *
 * \param argc  pointer to number of command line parameters as passed to main
 * \param argv  pointer to array of command line parameters as passed to main
 * \param arg0 format list describing the command line options to parse for
 * \return 0 if the command-line was successfully parsed, otherwise:
 * - -1 if the given descriptors are somehow wrong.
 * - -2 if not enough memory was available to hold temporary structs.
 * - -3 if the given command-line args did not meet the specified set.
 * - -4 if the help-option was given.
 *
 * Upon return, argc and argv point to a list of arguments that were not
 * scanned as arguments. See \c getoptlong for details on scanning. */
L4_CV int parse_cmdline(int *argc, const char***argv, char arg0, ...);
L4_CV int parse_cmdlinev(int *argc, const char***argv, char arg0, va_list va);
L4_CV int parse_cmdline_extra(const char*argv0, const char*line, char delim,
			      char arg0,...);

EXTERN_C_END
/*@}*/

#endif

