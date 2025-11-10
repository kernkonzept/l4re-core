LIBC_BUILD_MINIMAL = $(filter minimal,$(LIBC_BUILD_MODE))

define SRC_misc
  basename
  dirname
  ffs
  getopt
  getopt_long
  realpath
endef

define SRC_errno
  strerror
endef

define SRC_signal
  psignal
  sigaddset
  sigandset
  sigdelset
  sigemptyset
  sigfillset
  sigisemptyset
  sigismember
  siglongjmp
  sigorset
  sigsetjmp
  sigsetjmp_tail
endef

define SRC_ctype
  __ctype_b_loc
  __ctype_get_mb_cur_max
  __ctype_tolower_loc
  __ctype_toupper_loc
  isalnum
  isalpha
  isascii
  isblank
  iscntrl
  isdigit
  isgraph
  islower
  isprint
  ispunct
  isspace
  isupper
  isxdigit
  toascii
  tolower
  toupper
endef

define SRC_ctype_wchar
  iswalnum
  iswalpha
  iswblank
  iswcntrl
  iswctype
  iswdigit
  iswgraph
  iswlower
  iswprint
  iswpunct
  iswspace
  iswupper
  iswxdigit
  towctrans
  wcswidth
  wctrans
  wcwidth
endef

define SRC_dirent
  alphasort
  closedir
  dirfd
  fdopendir
  opendir
  readdir
  readdir_r
  rewinddir
  scandir
  seekdir
  telldir
  versionsort
endef

define SRC_internal
  defsysinfo
  $(if $(BID_VARIANT_FLAG_NOFPU),,floatscan)
  intscan
  libc
  shgetc
  syscall_ret
  version
endef

define SRC_math
  __cos
  __cosdf
  __cosl
  __expo2
  __expo2f
  __fpclassify
  __fpclassifyf
  __fpclassifyl
  __invtrigl
  __math_divzero
  __math_divzerof
  __math_invalid
  __math_invalidf
  __math_invalidl
  __math_oflow
  __math_oflowf
  __math_uflow
  __math_uflowf
  __math_xflow
  __math_xflowf
  __polevll
  __rem_pio2
  __rem_pio2_large
  __rem_pio2f
  __rem_pio2l
  __signbit
  __signbitf
  __signbitl
  __sin
  __sindf
  __sinl
  __tan
  __tandf
  __tanl
  acos
  acosf
  acosh
  acoshf
  acoshl
  acosl
  asin
  asinf
  asinh
  asinhf
  asinhl
  asinl
  atan
  atan2
  atan2f
  atan2l
  atanf
  atanh
  atanhf
  atanhl
  atanl
  cbrt
  cbrtf
  cbrtl
  ceil
  ceilf
  ceill
  copysign
  copysignf
  copysignl
  cos
  cosf
  cosh
  coshf
  coshl
  cosl
  erf
  erff
  erfl
  exp
  exp2
  exp2f
  exp2f_data
  exp2l
  exp10
  exp10f
  exp10l
  exp_data
  expf
  expl
  expm1
  expm1f
  expm1l
  fabs
  fabsf
  fabsl
  fdim
  fdimf
  fdiml
  finite
  finitef
  floor
  floorf
  floorl
  fma
  fmaf
  fmal
  fmax
  fmaxf
  fmaxl
  fmin
  fminf
  fminl
  fmod
  fmodf
  fmodl
  frexp
  frexpf
  frexpl
  hypot
  hypotf
  hypotl
  ilogb
  ilogbf
  ilogbl
  j0
  j0f
  j1
  j1f
  jn
  jnf
  ldexp
  ldexpf
  ldexpl
  lgamma
  lgamma_r
  lgammaf
  lgammaf_r
  lgammal
  llrint
  llrintf
  llrintl
  llround
  llroundf
  llroundl
  log
  log1p
  log1pf
  log1pl
  log2
  log2_data
  log2f
  log2f_data
  log2l
  log10
  log10f
  log10l
  log_data
  logb
  logbf
  logbl
  logf
  logf_data
  logl
  lrint
  lrintf
  lrintl
  lround
  lroundf
  lroundl
  modf
  modff
  modfl
  nan
  nanf
  nanl
  nearbyint
  nearbyintf
  nearbyintl
  nextafter
  nextafterf
  nextafterl
  nexttoward
  nexttowardf
  nexttowardl
  pow
  pow_data
  powf
  powf_data
  powl
  remainder
  remainderf
  remainderl
  remquo
  remquof
  remquol
  rint
  rintf
  rintl
  round
  roundf
  roundl
  scalb
  scalbf
  scalbln
  scalblnf
  scalblnl
  scalbn
  scalbnf
  scalbnl
  signgam
  significand
  significandf
  sin
  sincos
  sincosf
  sincosl
  sinf
  sinh
  sinhf
  sinhl
  sinl
  sqrt
  sqrt_data
  sqrtf
  sqrtl
  tan
  tanf
  tanh
  tanhf
  tanhl
  tanl
  tgamma
  tgammaf
  tgammal
  trunc
  truncf
  truncl
endef

define SRC_fenv
  __flt_rounds
  fegetexceptflag
  feholdexcept
  fenv
  fesetexceptflag
  fesetround
  feupdateenv
endef

define SRC_errno
  __errno_location
  strerror
endef

define SRC_ldso
  dl_iterate_phdr
endef

define SRC_ldso_libc.so
  __dlsym
  dladdr
  dlclose
  dlerror
  dlinfo
  dlopen
  dlstart
  dlsym
  dynlink
  tlsdesc
endef

# Only not minimal: _Exit?
define SRC_exit
  abort
  assert
  atexit
  exit
endef

define SRC_multibyte
  btowc
  c16rtomb
  c32rtomb
  internal
  mblen
  mbrlen
  mbrtoc16
  mbrtoc32
  mbrtowc
  mbsinit
  mbsnrtowcs
  mbsrtowcs
  mbstowcs
  mbtowc
  wcrtomb
  wcsnrtombs
  wcsrtombs
  wcstombs
  wctob
  wctomb
endef

define SRC_locale
  __lctrans
  __mo_lookup
  c_locale
  freelocale
  langinfo
  locale_map
  localeconv
  newlocale
  setlocale
  strcoll
  strxfrm
  uselocale
endef

define SRC_locale_fp
  strtod_l
endef

define SRC_locale_wchar
  wcscoll
  wcsxfrm
endef

define SRC_malloc
  calloc
  free
  libc_calloc
  lite_malloc
  mallocng/aligned_alloc
  mallocng/donate
  mallocng/free
  mallocng/malloc
  mallocng/realloc
  memalign
  posix_memalign
  realloc
  replaced
endef

define SRC_mman
  mmap
  mremap
  munmap
endef

# TODO:
# - __init_ssp
# - secure_getenv: Already defined in libc_be_misc
define SRC_env
  __environ
  $(if $(LIBC_BUILD_MINIMAL),,__init_tls)
  __libc_start_main
  clearenv
  getenv
  putenv
  setenv
  unsetenv
endef

define SRC_passwd
  getgr_a
  getgrent
  getgrent_a
  getpw_a
  getpw_r
  getpwent
  getpwent_a
  nscd_query
endef

define SRC_prng
  $(if $(BID_VARIANT_FLAG_NOFPU),,__rand48_step)
  $(if $(BID_VARIANT_FLAG_NOFPU),,__seed48)
  $(if $(BID_VARIANT_FLAG_NOFPU),,drand48)
  rand
  rand_r
  random
endef

define SRC_thread
  default_attr
  lock_ptc
endef

DEFINES___lock.c += -D_GNU_SOURCE

define SRC_unistd
  __dup3
  isatty
  sleep
endef

define SRC_regex
  fnmatch
  glob
  regcomp
  regerror
  regexec
  tre-mem
endef

define SRC_syscalls
  __SYS_writev
endef

# TODO:
# - tmpnam (already provided by libc_be_fs_noop)
# - flockfile
# - ftrylockfile
# - funlockfile
define SRC_stdio
  __fdopen
  __fmodeflags
  $(if $(LIBC_BUILD_MINIMAL),,__lockfile)
  __overflow
  $(if $(LIBC_BUILD_MINIMAL),,__stdio_close)
  $(if $(LIBC_BUILD_MINIMAL),,__stdio_exit)
  __stdio_read
  __stdio_seek
  __stdio_write
  __stdout_write
  __toread
  __towrite
  __uflow
  asprintf
  clearerr
  dprintf
  ext
  ext2
  $(if $(LIBC_BUILD_MINIMAL),,fclose)
  feof
  ferror
  fflush
  fgetc
  fgetln
  fgetpos
  fgets
  fileno
  fmemopen
  fopen
  fopencookie
  fprintf
  fputc
  fputs
  fread
  freopen
  fscanf
  fseek
  fsetpos
  ftell
  fwrite
  getc
  getc_unlocked
  getchar
  getchar_unlocked
  getdelim
  getline
  gets
  ofl
  ofl_add
  open_memstream
  perror
  printf
  putc
  putc_unlocked
  putchar
  putchar_unlocked
  puts
  remove
  rewind
  scanf
  setbuf
  setbuffer
  setlinebuf
  setvbuf
  snprintf
  sprintf
  sscanf
  stderr
  stdin
  stdout
  tmpfile
  tempnam
  ungetc
  vasprintf
  vdprintf
  vfprintf
  vfscanf
  vprintf
  vscanf
  vsnprintf
  vsprintf
  vsscanf
endef

define SRC_stdio_wchar
  fgetwc
  fgetws
  fputwc
  fputws
  fwide
  fwprintf
  fwscanf
  getw
  getwc
  getwchar
  open_wmemstream
  putw
  putwc
  putwchar
  swprintf
  swscanf
  ungetwc
  vfwprintf
  vfwscanf
  vswprintf
  vswscanf
  vwprintf
  vwscanf
  wprintf
  wscanf
endef

define SRC_setjmp
  longjmp
  setjmp
endef

define SRC_stdlib
  abs
  atoi
  atol
  atoll
  bsearch
  div
  imaxabs
  imaxdiv
  labs
  ldiv
  llabs
  lldiv
  qsort
  qsort_nr
  $(if $(BID_VARIANT_FLAG_NOFPU),,strtod)
  strtol
endef

define SRC_stdlib_wchar
  wcstol
  $(if $(BID_VARIANT_FLAG_NOFPU),,wcstod)
endef

define SRC_stdlib_fp
  atof
  ecvt
  fcvt
  gcvt
endef

# NOTE: Regarding memset and memcpy
# On arm64 these optimized implementations of memcpy and memset expect
# that unaligned memory accesses are allowed, which unfortunately is
# incompatible with bootstrap on arm64.
# Since these are hand-crafted in assembly, the -mstrict-align option that
# we set when building libc-minimal does not apply. As a workaround we use
# the generic C fallback implementations.
# Once we upgraded bootstrap so that it enables the caches, we can switch
# back to the optimized implementations.
define SRC_string
  bcmp
  bcopy
  bzero
  explicit_bzero
  index
  memccpy
  memchr
  memcmp
  $(if $(and $(LIBC_BUILD_MINIMAL), $(filter arm64, $(BUILD_ARCH)))$(BID_VARIANT_FLAG_NOFPU), memcpy.c, memcpy)
  memmem
  memmove
  mempcpy
  memrchr
  $(if $(and $(LIBC_BUILD_MINIMAL), $(filter arm64, $(BUILD_ARCH)))$(BID_VARIANT_FLAG_NOFPU), memset.c, memset)
  stpcpy
  stpncpy
  strcasecmp
  strcasestr
  strcat
  strchr
  strchrnul
  strcmp
  strcpy
  rindex
  strcspn
  strdup
  strerror_r
  strlcat
  strlcpy
  strlen
  strncasecmp
  strncat
  strncmp
  strncpy
  strndup
  strnlen
  strpbrk
  strrchr
  strsep
  strsignal
  strspn
  strstr
  strtok
  strtok_r
  strverscmp
  swab
endef

define SRC_string_wchar
  wcpcpy
  wcpncpy
  wcscasecmp
  wcscasecmp_l
  wcscat
  wcschr
  wcscmp
  wcscpy
  wcscspn
  wcsdup
  wcslen
  wcsncasecmp
  wcsncasecmp_l
  wcsncat
  wcsncmp
  wcsncpy
  wcsnlen
  wcspbrk
  wcsrchr
  wcsspn
  wcsstr
  wcstok
  wcswcs
  wmemchr
  wmemcmp
  wmemcpy
  wmemmove
  wmemset
endef

define SRC_temp
  __randname
  mkdtemp
  mkostemp
  mkostemps
  mkstemp
  mkstemps
  mktemp
endef

# TODO:
# - cfmakeraw: Already stubbed in libc_be_misc
# - tcsendbreak: Already stubbed in libc_be_misc
define SRC_termios
  cfgetospeed
  cfsetospeed
  cfsetspeed
  tcdrain
  tcflow
  tcflush
  tcgetattr
  tcgetsid
  tcgetwinsize
  tcsetattr
  tcsetwinsize
endef

define SRC_time
  __map_file
  __month_to_secs
  __secs_to_tm
  __tm_to_secs
  __tz
  __utc
  __year_to_secs
  asctime
  asctime_r
  ctime
  gmtime
  gmtime_r
  localtime
  localtime_r
  mktime
  strftime
  strptime
  timegm
endef

define SRC_time_fp
  difftime
endef

define SRC_time_wchar
  wcsftime
endef


define SRC_legacy
  err
  getpagesize
endef
