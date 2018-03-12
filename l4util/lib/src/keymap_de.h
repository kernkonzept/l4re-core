/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische UniversitÃ¤t Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#ifndef __L4UTIL__INCLUDE__KEYMAP_DE_H__
#define __L4UTIL__INCLUDE__KEYMAP_DE_H__

#define SHIFT -1

static char keymap[128][2] =
{
	{0},			/* 0 */
	{27,	27},		/* 1 - ESC */
	{'1',	'!'},		/* 2 */
	{'2',	'"'},
	{'3',	'§'},
	{'4',	'$'},
	{'5',	'%'},
	{'6',	'&'},
	{'7',	'/'},
	{'8',	'('},
	{'9',	')'},
	{'0',	'='},
	{'ß',	'?'},
	{'\'',	'`'},
	{8,	8},		/* 14 - Backspace */
	{'\t',	'\t'},		/* 15 */
	{'q',	'Q'},
	{'w',	'W'},
	{'e',	'E'},
	{'r',	'R'},
	{'t',	'T'},
	{'z',	'Z'},
	{'u',	'U'},
	{'i',	'I'},
	{'o',	'O'},
	{'p',	'P'},
	{'ü',	'Ü'},
	{'+',	'*'},		/* 27 */
	{'\r',	'\r'},		/* 28 - Enter */
	{0,	0},		/* 29 - Ctrl */
	{'a',	'A'},		/* 30 */
	{'s',	'S'},
	{'d',	'D'},
	{'f',	'F'},
	{'g',	'G'},
	{'h',	'H'},
	{'j',	'J'},
	{'k',	'K'},
	{'l',	'L'},
	{'ö',	'Ö'},
	{'ä',	'Ä'},		/* 40 */
	{'^',	'°'},		/* 41 */
	{SHIFT,	SHIFT},		/* 42 - Left Shift */
	{'#',	'\''},		/* 43 */
	{'y',	'Y'},		/* 44 */
	{'x',	'X'},
	{'c',	'C'},
	{'v',	'V'},
	{'b',	'B'},
	{'n',	'N'},
	{'m',	'M'},
	{',',	';'},
	{'.',	':'},
	{'-',	'_'},		/* 53 */
	{SHIFT,	SHIFT},		/* 54 - Right Shift */
	{0,	0},		/* 55 - Print Screen */
	{0,	0},		/* 56 - Alt */
	{' ',	' '},		/* 57 - Space bar */
	{0,	0},		/* 58 - Caps Lock */
	{0,	0},		/* 59 - F1 */
	{0,	0},		/* 60 - F2 */
	{0,	0},		/* 61 - F3 */
	{0,	0},		/* 62 - F4 */
	{0,	0},		/* 63 - F5 */
	{0,	0},		/* 64 - F6 */
	{0,	0},		/* 65 - F7 */
	{0,	0},		/* 66 - F8 */
	{0,	0},		/* 67 - F9 */
	{0,	0},		/* 68 - F10 */
	{0,	0},		/* 69 - Num Lock */
	{0,	0},		/* 70 - Scroll Lock */
	{'7',	'7'},		/* 71 - Numeric keypad 7 */
	{'8',	'8'},		/* 72 - Numeric keypad 8 */
	{'9',	'9'},		/* 73 - Numeric keypad 9 */
	{'-',	'-'},		/* 74 - Numeric keypad '-' */
	{'4',	'4'},		/* 75 - Numeric keypad 4 */
	{'5',	'5'},		/* 76 - Numeric keypad 5 */
	{'6',	'6'},		/* 77 - Numeric keypad 6 */
	{'+',	'+'},		/* 78 - Numeric keypad '+' */
	{'1',	'1'},		/* 79 - Numeric keypad 1 */
	{'2',	'2'},		/* 80 - Numeric keypad 2 */
	{'3',	'3'},		/* 81 - Numeric keypad 3 */
	{'0',	'0'},		/* 82 - Numeric keypad 0 */
	{'.',	'.'},		/* 83 - Numeric keypad '.' */
	{0,	0},
	{0,	0},
	{'<',   '>'},
	{0,	0},
	{0,	0},
	{0,	0},
	{0,	0},
	{0,	0},
	{0,	0},
	{0,	0},
	{0,	0},
	{0,	0},
	{0,	0},
	{0,	0},
	{0,	0},
	{0,	0},
	{0,	0},
	{0,	0},
	{0,	0},
	{0,	0},		/* 103 - arrow up */
	{0,	0},
	{0,	0},		/* 105 - arrow left */
	{0,	0},		/* 106 - arrow right */
	{0,	0},
	{0,	0},		/* 108 - arrow down */
	{0,	0},
	{0,	0},
	{0,	0},
	{0,	0},
	{0,	0},
	{0,	0},
	{0,	0},
	{0,	0},
	{0,	0},
	{0,	0},
	{0,	0},
	{0,	0},
	{0,	0},
	{0,	0},
	{0,	0},
	{0,	0},
	{0,	0},
	{0,	0},
	{0,	0},
};

#endif
