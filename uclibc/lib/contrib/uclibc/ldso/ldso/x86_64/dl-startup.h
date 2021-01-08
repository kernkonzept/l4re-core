/* vi: set sw=4 ts=4: */
/*
 * Architecture specific code used by dl-startup.c
 * Copyright (C) 2000-2005 by Erik Andersen <andersen@codepoet.org>
 * Copyright (C) 2005 by Mike Frysinger <vapier@gentoo.org>
 *
 * Parts taken from glibc/sysdeps/x86_64/dl-machine.h
 */
__asm__ (
	"	.text\n"
	"	.global _start\n"
	"	.type   _start,%function\n"
	"	.hidden _start\n"
	"_start:\n"
	"	movq %rsp, %rdi\n"
	"	call _dl_start\n"
	"	# Save the user entry point address in %r12.\n"
	"	movq %rax, %r12\n"
	"	# See if we were run as a command with the executable file\n"
	"	# name as an extra leading argument.\n"
	"	movl _dl_skip_args(%rip), %eax\n"
	"	# Pop the original argument count.\n"
	"	popq %rdx\n"
	"	# Adjust the stack pointer to skip _dl_skip_args words.\n"
	"	leaq (%rsp,%rax,8), %rsp\n"
	"	# Subtract _dl_skip_args from argc.\n"
	"	subl %eax, %edx\n"
	"	# Push argc back on the stack.\n"
	"	pushq %rdx\n"
	"	# Pass our finalizer function to the user in %rdx, as per ELF ABI.\n"
	"	leaq _dl_fini(%rip), %rdx\n"
	"	# Jump to the user's entry point.\n"
	"	jmp *%r12\n"
	"	.size	_start,.-_start\n"
	"	.previous\n"
);

/* Get a pointer to the argv array.  On many platforms this can be just
 * the address of the first argument, on other platforms we need to
 * do something a little more subtle here.  */
#define GET_ARGV(ARGVP, ARGS) ARGVP = (((unsigned long*) ARGS)+1)

#include "dl-bootstrap.h"
