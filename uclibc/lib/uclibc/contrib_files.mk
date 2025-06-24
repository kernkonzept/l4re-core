LIBC_BUILD_MINIMAL = $(filter minimal,$(LIBC_BUILD_MODE))

define SRC_libc/sysdeps/linux
  setjmp
  bsd-setjmp
  bsd-_setjmp
  creat
  longjmp
  __longjmp
  getdents
  getsid
  common/context
  common/cmsg_nxthdr
  common/flock
  common/getdomainname
  common/gethostname
  common/getdirname
  common/getpagesize
  common/getpgid
  common/makedev
  common/sethostname
  common/setgroups
  common/sysinfo
endef

define SRC_libc/sysdeps/linux_large_file
  creat64
endef

SRC_libc/sysdeps/linux_arm += aeabi_atexit
SRC_libc/sysdeps/linux_arm += find_exidx

SRC_libc/sysdeps/linux_mips += setjmp_aux
SRC_libc/sysdeps/linux_mips += _test_and_set

ifeq ($(GCCIS_sparc_leon),)
  define SRC_libc/sysdeps/linux_sparc
    udiv
    umul
    urem
  endef
endif

define SRC_libc/termios
  isatty
  speed
  tcdrain
  tcflow
  tcflush
  tcgetattr
  tcgetpgrp
  tcgetsid
  tcsetattr
  tcsetpgrp
  ttyname
endef

define SRC_libc/stdlib
  __cxa_atexit
  __exit_handler
  $(if $(BID_VARIANT_FLAG_NOFPU),,__strtofpmax)
  __uc_malloc
  _stdlib_strto_l
  _stdlib_strto_ll
  abort
  abs
  atoi
  atol
  atoll
  bsearch
  div
  exit
  $(if $(BID_VARIANT_FLAG_NOFPU),,gcvt)
  getenv
  $(if $(BID_VARIANT_FLAG_NOFPU),,jrand48)
  $(if $(BID_VARIANT_FLAG_NOFPU),,jrand48_r)
  labs
  ldiv
  llabs
  lldiv
  $(if $(BID_VARIANT_FLAG_NOFPU),,lrand48)
  $(if $(BID_VARIANT_FLAG_NOFPU),,lrand48_r)
  mkdtemp
  mkostemp
  mkstemp
  mkstemp64
  mktemp
  $(if $(BID_VARIANT_FLAG_NOFPU),,nrand48)
  $(if $(BID_VARIANT_FLAG_NOFPU),,nrand48_r)
  on_exit
  posix_memalign
  qsort
  qsort_r
  rand
  rand_r
  random
  random_r
  realpath
  setenv
  $(if $(BID_VARIANT_FLAG_NOFPU),,srand48)
  $(if $(BID_VARIANT_FLAG_NOFPU),,srand48_r)
  stdlib
  strtol
  strtoll
  strtoul
  strtoull
endef

define SRC_libc/stdlib_locale
  _stdlib_strto_l_l
  _stdlib_strto_ll_l
  $(if $(BID_VARIANT_FLAG_NOFPU),,__strtofpmax_l)
  $(if $(BID_VARIANT_FLAG_NOFPU),,__wcstofpmax_l)
  strtol_l
  strtoll_l
  strtoul_l
  strtoull_l
endef

define SRC_libc/stdlib_large_file
  mkostemp64
endef

define SRC_libc/stdlib_fp
  __fp_range_check
  atof
  drand48-iter
  drand48
  drand48_r
  erand48
  erand48_r
  strtod
  $(if $(LIBC_BUILD_MINIMAL),,strtod_l)
  strtof
  $(if $(LIBC_BUILD_MINIMAL),,strtof_l)
  strtold
  $(if $(LIBC_BUILD_MINIMAL),,strtold_l)
endef

define SRC_libc/stdlib/malloc
  calloc
  free
  malloc
  memalign
  realloc
  heap_alloc
  heap_alloc_at
  heap_free
endef

define SRC_libc/stdlib/malloc-standard
  calloc
  free
  mallinfo
  malloc
	malloc_usable_size
  mallopt
  memalign
  realloc
endef

define SRC_libc/stdlib/malloc-simple
  alloc
  calloc
  free
  malloc
  memalign
  realloc
endef

define SRC_libc/stdlib_wchar
  $(if $(BID_VARIANT_FLAG_NOFPU),,__wcstofpmax)
  $(if $(BID_VARIANT_FLAG_NOFPU),,wcstod)
  $(if $(BID_VARIANT_FLAG_NOFPU),,wcstof)
  $(if $(BID_VARIANT_FLAG_NOFPU),,wcstold)
  _stdlib_mb_cur_max
  _stdlib_wcsto_l
  _stdlib_wcsto_ll
  mblen
  wcstombs
  mbstowcs
  wcstol
  wcstoll
  wcstoul
  wcstoull
  wctomb
  mbtowc
endef

define SRC_libc/stdlib_wchar_locale
  $(if $(BID_VARIANT_FLAG_NOFPU),,wcstod_l)
  $(if $(BID_VARIANT_FLAG_NOFPU),,wcstof_l)
  $(if $(BID_VARIANT_FLAG_NOFPU),,wcstold_l)
  _stdlib_wcsto_l_l
  _stdlib_wcsto_ll_l
  wcstoll_l
  wcstoul_l
  wcstoull_l
endef

define SRC_libc/string
  basename
  dirname
  ffs
  memccpy
  stpcpy
  stpncpy
  strcasecmp
  strcasestr
  strdup
  strerror
  __xpg_strerror_r
  _string_syserrmsgs
  __glibc_strerror_r
  _string_syssigmsgs
  sys_siglist
  strsignal
  psignal
  __xpg_basename
  strlcat
  strlcpy
  sys_errlist
  strndup
  strncasecmp
  strtok
  bcopy
  bzero
  memchr
  memcpy
  memmove
  memset
  strcat
  strchr
  strcmp
  strcpy
  strlen
  strncat
  strncmp
  strncpy
  strnlen
  strrchr
  memcmp
  memmem
  mempcpy
  memrchr
  rawmemchr
  strchrnul
  strcspn
  strsep
  strspn
  strstr
  strtok_r
  strpbrk
endef

define SRC_libc/string_locale
  strcasecmp_l
  strncasecmp_l
endef

define SRC_libc/string_wchar
  strxfrm
  wcscmp
  wcsnlen
  wcslen
  wcsncpy
  wmemcpy
  wcsstr
  wcscasecmp
  wcscpy
  wcsncasecmp
  wcstok
  wcscat
  wcscspn
  wcspbrk
  wcschr
  wcsdup
  wcsncat
  wcsrchr
  wcsxfrm
  wcschrnul
  wcslcpy
  wcsncmp
  wcsspn
  wmemchr
  wmemcmp
  wmemmove
  wmempcpy
  wmemset
endef

define SRC_libc/string_wchar_locale
  strxfrm_l
  wcscasecmp_l
  wcsncasecmp_l
  wcsxfrm_l
endef

SRC_libc/string_arm := _memcpy

define SRC_libc/misc
  assert/__assert
  auxvt/getauxval
  ctype/ctype
  ctype/isalnum
  ctype/isalpha
  ctype/isascii
  ctype/isblank
  ctype/iscntrl
  ctype/isdigit
  ctype/isgraph
  ctype/islower
  ctype/isprint
  ctype/ispunct
  ctype/isspace
  ctype/isupper
  ctype/isxdigit
  ctype/tolower
  ctype/toascii
  ctype/toupper
  ctype/__C_ctype_b
  ctype/__C_ctype_tolower
  ctype/__C_ctype_toupper
  ctype/__ctype_b_loc
  ctype/__ctype_tolower_loc
  ctype/__ctype_toupper_loc
  ctype/__ctype_assert
  ctype/isctype
  dirent/alphasort
  dirent/closedir
  dirent/dirfd
  dirent/opendir
  dirent/readdir
  dirent/readdir_r
  dirent/rewinddir
  dirent/scandir
  dirent/seekdir
  dirent/telldir
  error/err
  fnmatch/fnmatch
  fts/fts
  glob/glob
  internals/errno
  internals/h_errno
  internals/__errno_location
  internals/__h_errno_location
  internals/__uClibc_main
  internals/parse_config
  internals/tempname
  internals/version
  mntent/mntent
  regex/regex
  search/hcreate_r
  search/hdestroy_r
  search/hsearch
  search/hsearch_r
  search/insque
  search/insremque
  search/lfind
  search/lsearch
  search/remque
  search/tdelete
  search/tdestroy
  search/tfind
  search/tsearch
  search/twalk
  syslog/syslog
  time/time
  time/asctime
  time/asctime_r
  time/ctime
  time/ctime_r
  time/ftime
  time/gmtime
  time/gmtime_r
  time/localtime
  time/localtime_r
  time/mktime
  time/strftime
  time/strptime
  time/tzset
  time/_time_t2tm
  time/__time_tm
  time/_time_mktime
  time/dysize
  time/timegm
  time/_time_mktime_tzi
  time/_time_localtime_tzi
  ttyent/getttyent
  $(if $(LIBC_BUILD_MINIMAL),,utmp/utent)
  elf/dl-iterate-phdr
endef

define SRC_libc/misc_locale
  ctype/isalnum_l
  ctype/isalpha_l
  ctype/isascii_l
  ctype/isblank_l
  ctype/iscntrl_l
  ctype/isdigit_l
  ctype/isgraph_l
  ctype/islower_l
  ctype/isprint_l
  ctype/ispunct_l
  ctype/isspace_l
  ctype/isupper_l
  ctype/isxdigit_l
  ctype/tolower_l
  ctype/toascii_l
  ctype/toupper_l
  locale/freelocale
  locale/locale
  locale/localeconv
  locale/newlocale
  locale/nl_langinfo
  locale/setlocale
  locale/uselocale
  locale/_locale_init
  locale/__curlocale
  locale/__locale_mbrtowc_l
  locale/nl_langinfo_l
  time/strftime_l
  time/strptime_l
endef

define SRC_libc/misc_fp
  $(if $(BID_VARIANT_FLAG_NOFPU),,time/difftime)
endef

define SRC_libc/misc_large_file
  dirent/alphasort64
  dirent/readdir64
  dirent/readdir64_r
  dirent/scandir64
  glob/glob64
endef

define SRC_libc/misc_libc.a
  elf/dl-support
  elf/dl-core
  $(if $(CONFIG_BID_PIE),internals/reloc_static_pie)
endef

SRC_libc/misc_libc_minimal.a = $(SRC_libc/misc_libc.a)

define SRC_libc/misc_wchar
  time/wcsftime
  wchar/btowc
  wchar/mbsinit
  wchar/mbrlen
  wchar/mbrtowc
  wchar/mbsnrtowcs
  wchar/mbsrtowcs
  wchar/wchar
  wchar/wctob
  wchar/wcrtomb
  wchar/wcsnrtombs
  wchar/wcsrtombs
  wchar/wcwidth
  wchar/wcswidth
  wchar/_wchar_utf8sntowcs
  wchar/_wchar_wcsntoutf8s
  wctype/iswctype
  wctype/wctype
  wctype/towlower
  wctype/towupper
  wctype/towctrans
  wctype/wctrans
  wctype/iswxdigit
  wctype/iswupper
  wctype/iswlower
  wctype/iswspace
  wctype/iswpunct
  wctype/iswprint
  wctype/iswgraph
  wctype/iswdigit
  wctype/iswcntrl
  wctype/iswblank
  wctype/iswalpha
  wctype/iswalnum
endef

define SRC_libc/misc_wchar_locale
  time/wcsftime_l
  wctype/iswctype_l
  wctype/wctype_l
  wctype/towlower_l
  wctype/towupper_l
  wctype/towctrans_l
  wctype/wctrans_l
  wctype/iswxdigit_l
  wctype/iswupper_l
  wctype/iswlower_l
  wctype/iswspace_l
  wctype/iswpunct_l
  wctype/iswprint_l
  wctype/iswgraph_l
  wctype/iswdigit_l
  wctype/iswcntrl_l
  wctype/iswblank_l
  wctype/iswalpha_l
  wctype/iswalnum_l
endef

define SRC_libc/stdio
  __fsetlocking
  _adjust_pos
  _cs_funcs
  _fopen
  $(if $(BID_VARIANT_FLAG_NOFPU),,_fpmaxtostr)
  _fwrite
  _READ
  _WRITE
  _load_inttype
  _rfill
  _stdio
  _store_inttype
  _trans2r
  _trans2w
  _uintmaxtostr
  _wcommit
  asprintf
  clearerr
  clearerr.__DO_UNLOCKED
  dprintf
  fclose
  fcloseall
  fdopen
  feof
  feof.__DO_UNLOCKED
  ferror
  ferror.__DO_UNLOCKED
  fflush
  fflush.__DO_UNLOCKED
  fgetc
  fgetc.__DO_UNLOCKED
  fgetpos
  fgets
  fgets.__DO_UNLOCKED
  fileno
  fileno.__DO_UNLOCKED
  flockfile
  fopen
  freopen
  fputs
  fputs.__DO_UNLOCKED
  fputc
  fputc.__DO_UNLOCKED
  fprintf
  fread
  fread.__DO_UNLOCKED
  fseeko
  fsetpos
  ftello
  funlockfile
  fwrite
  fwrite.__DO_UNLOCKED
  getchar
  getchar.__DO_UNLOCKED
  getdelim
  getline
  gets
  perror
  printf
  putchar
  putchar.__DO_UNLOCKED
  puts
  remove
  rewind
  _scanf
  vfscanf
  __scan_cookie
  __psfs_parse_spec
  __psfs_do_numeric
  scanf
  sscanf
  fscanf
  vscanf
  vsscanf
  setvbuf
  setbuf
  setbuffer
  setlinebuf
  snprintf
  sprintf
  tmpfile
  ungetc
  vasprintf
  vdprintf
  _vfprintf
  _vfprintf_internal
  vfprintf
  _ppfs_init
  _ppfs_prepargs
  _ppfs_setargs
  _ppfs_parsespec
  vsnprintf
  vsprintf
  vprintf
endef

define SRC_libc/stdio_large_file
  fgetpos64
  fopen64
  freopen64
  fseeko64
  fsetpos64
  ftello64
endef

define SRC_libc/stdio_wchar
  _wfwrite
  fputwc
  fputwc.__DO_UNLOCKED
  fgetwc
  fgetwc.__DO_UNLOCKED
  fgetws
  fgetws.__DO_UNLOCKED
  fputws
  fputws.__DO_UNLOCKED
  fwide
  fwprintf
  fwscanf
  ungetwc
  getwchar
  getwchar.__DO_UNLOCKED
  putwchar
  putwchar.__DO_UNLOCKED
  swprintf
  swscanf
  _vfwprintf_internal
  vfwprintf
  vfwscanf
  vswprintf
  vswscanf
  vwprintf
  vwscanf
  wprintf
  wscanf
endef

define SRC_libc/inet
  _res_state
  addr
  closenameservers
  decodea
  decodeh
  decodep
  decodeq
  dnslookup
  encodea
  encodeh
  encodep
  encodeq
  ether_addr
  formquery
  gai_strerror
  get_hosts_byaddr_r
  get_hosts_byname_r
  getaddrinfo
  gethostbyaddr
  gethostbyaddr_r
  gethostbyname
  gethostbyname2
  gethostbyname2_r
  gethostbyname_r
  gethostent
  gethostent_r
  getnameinfo
  getnet
  getproto
  getservice
  herror
  if_index
  ifaddrs
  inet_addr
  inet_aton
  inet_lnaof
  inet_makeaddr
  inet_net
  inet_netof
  inet_ntoa
  in6_addr
  lengthd
  lengthq
  ns_name
  ntohl
  ntop
  opennameservers
  opensock
  read_etc_hosts_r
  res_comp
  res_init
  res_query
  resolv
endef

define SRC_libc/pwd_grp
  getgrent
  getgrent_r
  getgrgid
  getgrgid_r
  getgrnam
  getgrnam_r
  getpwuid
  getpwuid_r
  getpwnam
  getpwnam_r
  getpwent
  getpwent_r
  initgroups
  __getgrouplist_internal
  __parsepwent
  __parsegrent
  __pgsreader
endef

define SRC_libc/unistd
  confstr
  getlogin
  getopt
  sleep
endef

define SRC_libc/signal
  allocrtsig
  raise
  sigaddset
  sigandset
  sigdelset
  sigorset
  sigempty
  sigisempty
  sigfillset
  sigismem
  sigsetops
  sigjmp
endef

define SRC_libuargp
  argp-ba
  argp-eexst
  argp-fmtstream
  argp-fs-xinl
  argp-help
  argp-parse
  argp-pv
  argp-pvh
  argp-xinl
endef

define SRC_libcrypt
  crypt
  des
  md5
  sha256
  sha256-crypt
  sha512
  sha512-crypt
endef

define SRC_libiconv
  iconv
endef

define _MATH_FUNCTIONS
  acos
  acosh
  asin
  atan2
  atanh
  cosh
  exp
  exp10
  fmod
  hypot
  j0
  j1
  jn
  log
  log2
  log10
  pow
  remainder
  scalb
  sinh
  sqrt
endef

define SRC_libm
  carg
  cexp
  e_lgamma_r
  e_rem_pio2
  k_cos
  k_rem_pio2
  k_sin
  k_tan
  s_asinh
  s_atan
  s_cbrt
  s_ceil
  s_copysign
  s_cos
  s_erf
  s_expm1
  s_fabs
  s_fpclassify
  s_fdim
  s_finite
  s_finitef
  s_floor
  s_fma
  s_fmax
  s_fmin
  s_fpclassifyf
  s_frexp
  s_ilogb
  s_isnan
  s_isnanf
  s_isinf
  s_isinff
  s_ldexp
  s_lib_version
  s_llrint
  s_llround
  s_log1p
  s_logb
  s_lrint
  s_lround
  s_modf
  s_nextafter
  s_nextafterf
  s_remquo
  s_rint
  s_round
  s_scalbn
  s_signbit
  s_signbitf
  s_signgam
  s_significand
  s_sin
  s_tan
  s_tanh
  s_trunc
  w_cabs
  nan
  $(foreach f,$(_MATH_FUNCTIONS),e_$(f) w_$(f) w_$(f)f w_$(f)l)
  w_exp2
  w_exp2f
  w_exp2l
  w_lgamma_r
  w_lgammaf_r
  w_lgammal_r
  w_tgamma
  w_tgammaf
  w_tgammal
endef

define SRC_libm-amd64
  x86_64/fclrexcpt
  x86_64/fedisblxcpt
  x86_64/feenablxcpt
  x86_64/fegetenv
  x86_64/fegetexcept
  x86_64/fegetmode
  x86_64/fegetround
  x86_64/feholdexcpt
  x86_64/fesetenv
  x86_64/fesetexcept
  x86_64/fesetmode
  x86_64/fesetround
  x86_64/feupdateenv
  x86_64/fgetexcptflg
  x86_64/fraiseexcpt
  x86_64/fsetexcptflg
  x86_64/ftestexcept
endef

define SRC_libm-x86
  i386/fclrexcpt
  i386/fedisblxcpt
  i386/feenablxcpt
  i386/fegetenv
  i386/fegetexcept
  i386/fegetround
  i386/feholdexcpt
  i386/fesetenv
  i386/fesetround
  i386/feupdateenv
  i386/fgetexcptflg
  i386/fraiseexcpt
  i386/fsetexcptflg
  i386/ftestexcept
endef

SRC_libm += $(SRC_libm-$(BUILD_ARCH))

SRC_libm_float_src = float_wrappers.c
define SRC_libm_float
  acos
  acosf
  acoshf
  asinf
  asinhf
  atan2f
  atanf
  atanhf
  cbrtf
  ceilf
  copysignf
  cosf
  coshf
  erfcf
  erff
  exp2f
  expf
  expm1f
  fabsf
  fdimf
  floorf
  fmaf
  fmaxf
  fminf
  fmodf
  frexpf
  hypotf
  ilogbf
  ldexpf
  lgammaf
  llrintf
  log10f
  log1pf
  log2f
  logbf
  logf
  lrintf
  lroundf
  modff
  nearbyintf
  nextafterf
  powf
  remainderf
  remquof
  rintf
  roundf
  scalblnf
  scalbnf
  sinf
  sinhf
  sqrtf
  tanf
  tanhf
  tgammaf
  truncf
endef

SRC_libm_double_src = ldouble_wrappers.c
define SRC_libm_double
  acoshl
  acosl
  asinhl
  asinl
  atan2l
  atanhl
  atanl
  cargl
  cbrtl
  ceill
  copysignl
  coshl
  cosl
  erfcl
  erfl
  expl
  expm1l
  fabsl
  fdiml
  floorl
  fmal
  fmaxl
  fminl
  fmodl
  frexpl
  gammal
  hypotl
  ilogbl
  ldexpl
  lgammal
  llrintl
  llroundl
  log10l
  log1pl
  log2l
  logbl
  logl
  lrintl
  lroundl
  modfl
  nearbyintl
  nextafterl
  powl
  remainderl
  remquol
  rintl
  roundl
  scalblnl
  scalbnl
  sinhl
  sinl
  sqrtl
  tanhl
  tanl
  tgammal
  truncl
  significandl
endef
