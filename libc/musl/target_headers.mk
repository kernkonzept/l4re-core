define GEN_common
  bits/alltypes.h
  version.h
endef

define HDR_common
  alloca.h
  arpa/inet.h
  assert.h
  bits/dirent.h
  bits/errno.h
  bits/fcntl.h
  bits/fenv.h
  bits/ioctl.h
  bits/ioctl_fix.h
  bits/ipc.h
  bits/ipcstat.h
  bits/l4-malloc.h
  bits/limits.h
  bits/link.h
  bits/mman.h
  bits/poll.h
  bits/resource.h
  bits/sem.h
  bits/setjmp.h
  bits/shm.h
  bits/signal.h
  bits/socket.h
  bits/stat.h
  bits/statfs.h
  bits/stdint.h
  bits/syscall.h
  bits/termios.h
  byteswap.h
  complex.h
  ctype.h
  dirent.h
  dlfcn.h
  elf.h
  endian.h
  err.h
  errno.h
  fcntl.h
  features.h
  fenv.h
  fnmatch.h
  getopt.h
  glob.h
  grp.h
  ifaddrs.h
  inttypes.h
  langinfo.h
  locale.h
  libgen.h
  limits.h
  link.h
  math.h
  memory.h
  net/if.h
  netdb.h
  netinet/in.h
  netinet/tcp.h
  nl_types.h
  paths.h
  poll.h
  pthread.h
  pty.h
  pwd.h
  sched.h
  regex.h
  setjmp.h
  shadow.h
  signal.h
  spawn.h
  stdio.h
  stdio_ext.h
  string.h
  strings.h
  stdint.h
  stdlib.h
  sys/ioctl.h
  sys/ipc.h
  sys/file.h
  sys/mman.h
  sys/mount.h
  sys/param.h
  sys/prctl.h
  sys/resource.h
  sys/select.h
  sys/sem.h
  sys/shm.h
  sys/socket.h
  sys/stat.h
  sys/statfs.h
  sys/statvfs.h
  sys/sysmacros.h
  sys/syscall.h
  sys/time.h
  sys/times.h
  sys/ttydefaults.h
  sys/types.h
  sys/ucontext.h
  sys/utsname.h
  sys/uio.h
  sys/un.h
  sys/vfs.h
  sys/wait.h
  sysexits.h
  syslog.h
  tar.h
  termios.h
  time.h
  ucontext.h
  unistd.h
  utime.h
  wait.h
  wchar.h
  wctype.h
endef

vpath %.h $(CONTRIB_DIR)/arch/$(LIBC_ARCH)
vpath %.h $(CONTRIB_DIR)/arch/generic
vpath %.h $(CONTRIB_DIR)/include

$(HEADER_DIR)/bits/alltypes.h: $(CONTRIB_DIR)/arch/$(LIBC_ARCH)/bits/alltypes.h.in \
                               $(CONTRIB_DIR)/include/alltypes.h.in \
                               $(CONTRIB_DIR)/tools/mkalltypes.sed
	$(VERBOSE)[ -d $(@D) ] || $(MKDIR) -p $(@D)
	$(VERBOSE) sed -f $(CONTRIB_DIR)/tools/mkalltypes.sed \
	           $(CONTRIB_DIR)/arch/$(LIBC_ARCH)/bits/alltypes.h.in \
	           $(CONTRIB_DIR)/include/alltypes.h.in >$@
$(HEADER_DIR)/version.h: $(CONTRIB_DIR)/VERSION
	$(VERBOSE)printf '#define VERSION "%s"\n' $(cat $<) >$@
