/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/stat.h>

static bool verbose;

struct String
{
  String(const char *s, const char *e) : _s(s), _e(e), _p(s) {}
  const char *s() const { return _s; }
  const char *e() const { return _e; }
  const char *p() const { return _p; }
  unsigned l() const { return _e - _s; }

  const char *next_space()
  {
    const char *i = _p;
    while (i < _e && !isspace(*i))
      i++;
    _p = i;
    return _p;
  }
  void eat_space()
  {
    while (_p < e() && isspace(*_p))
      _p++;
  }

  bool empty() const { return !(_e - _s); }
  char *dupnultermstr() const
  { return strndup(s(), l()); }

  const char *_s, *_e, *_p;
};

static void parse_fstab_line(const char *fn, int line_nr,
                             const char *s, const char *end)
{
  const char *i = s;
  while (i < end && *i != '#')
    ++i;
  end = i;

  if (s == end)
    return;

  String line(s, end);
  String from(line.s(), line.next_space());

  line.next_space();
  line.eat_space();

  const char *x = line.p();
  String mountpoint(x, line.next_space());

  line.next_space();
  line.eat_space();

  x = line.p();
  String fsname(x, line.next_space());

  line.next_space();
  line.eat_space();

  x = line.p();
  String data(x, line.next_space());

  if (from.empty() || mountpoint.empty() || fsname.empty())
    {
      if (verbose)
        printf("libmount: Invalid line: %s.%d: %.*s\n",
               fn, line_nr, line.l(), line.s());
      return;
    }

  char *s1 = from.dupnultermstr();
  char *s2 = mountpoint.dupnultermstr();
  char *s3 = fsname.dupnultermstr();
  char *s5 = data.dupnultermstr();
  if (s1 && s2 && s3 && s5)
    {
      int r = mount(s1, *s2 == '/' ? s2 + 1 : s2, s3, 0, s5);
      if (r < 0)
        fprintf(stderr,
                "libmount: %s.%d: mount(\"%s\", \"%s\", \"%s\", %d, \"%s\"): %s\n",
                fn, line_nr, s1, s2, s3, 0, s5, strerror(errno));
      else if (verbose)
        printf("libmount: Mounted '%s' to '%s' with file-system '%s'\n",
               s1, s2, s3);
    }
  else
    fprintf(stderr, "libmount: %s.%d: memory allocation error\n", fn, line_nr);
  free(s5);
  free(s3);
  free(s2);
  free(s1);
}

static void parse_fstab(const char *fn, char *fstab, size_t sz)
{
  char *s = fstab;
  char *end = fstab + sz;
  int line_nr = 1;

  if (verbose)
    printf("libmount: Parsing '%s'\n", fn);

  while (s != end)
    {
      while (s < end && *s != '\n')
        s++;
      parse_fstab_line(fn, line_nr, fstab, s);
      if (s == end)
        break;
      fstab = ++s;
      line_nr++;
    }
}

static void libmount_init(void) __attribute__((constructor));
static void libmount_init()
{
  verbose = getenv("FSTAB_DEBUG");

  const char *fstab_path = getenv("FSTAB_FILE");
  if (!fstab_path)
    fstab_path = "/etc/fstab";

  int fd = open(fstab_path, O_RDONLY);
  if (fd < 0)
    {
      if (verbose)
        printf("libmount: Could not open '%s': %s.\n", fstab_path, strerror(errno));
      return;
    }

  struct stat st;
  if (fstat(fd, &st) < 0)
    {
      if (verbose)
        printf("libmount: Could not stat '%s'.\n", fstab_path);
      close(fd);
      return;
    }

  void *fstab_mem = mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (fstab_mem == MAP_FAILED)
    {
      if (verbose)
        printf("libmount: Could not mmap '%s'.\n", fstab_path);
      close(fd);
      return;
    }

  char *fstab = reinterpret_cast<char *>(fstab_mem);

  parse_fstab(fstab_path, fstab, st.st_size);

  munmap(fstab, st.st_size);
  close(fd);
}
