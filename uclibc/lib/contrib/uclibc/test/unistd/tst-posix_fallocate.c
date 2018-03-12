#include <fcntl.h>
#include <sys/stat.h>

#ifndef TST_POSIX_FALLOCATE64
# define stat64 stat
# define fstat64 fstat
# else
# ifndef O_LARGEFILE
#  error no O_LARGEFILE but you want to test with LFS enabled
# endif
#endif

static void do_prepare (void);
#define PREPARE(argc, argv) do_prepare ()
static int do_test (void);
#define TEST_FUNCTION do_test ()
#include <test-skeleton.c>

static int fd;
static void
do_prepare (void)
{
  fd = create_temp_file ("tst-posix_fallocate.", NULL);
  if (fd == -1)
    {
      printf ("cannot create temporary file: %m\n");
      exit (1);
    }
}


static int
do_test (void)
{
  struct stat64 st;

  if (fstat64 (fd, &st) != 0)
    {
      puts ("1st fstat failed");
      return 1;
    }

  if (st.st_size != 0)
    {
      puts ("file not created with size 0");
      return 1;
    }

  if (posix_fallocate (fd, 512, 768) != 0)
    {
      puts ("1st posix_fallocate call failed");
      return 1;
    }

  if (fstat64 (fd, &st) != 0)
    {
      puts ("2nd fstat failed");
      return 1;
    }

  if (st.st_size != 512 + 768)
    {
      printf ("file size after 1st posix_fallocate call is %llu, expected %u\n",
	      (unsigned long long int) st.st_size, 512u + 768u);
      return 1;
    }

  if (posix_fallocate (fd, 0, 1024) != 0)
    {
      puts ("2nd posix_fallocate call failed");
      return 1;
    }

  if (fstat64 (fd, &st) != 0)
    {
      puts ("3rd fstat failed");
      return 1;
    }

  if (st.st_size != 512 + 768)
    {
      puts ("file size changed in 2nd posix_fallocate");
      return 1;
    }

  if (posix_fallocate (fd, 2048, 64) != 0)
    {
      puts ("3rd posix_fallocate call failed");
      return 1;
    }

  if (fstat64 (fd, &st) != 0)
    {
      puts ("4th fstat failed");
      return 1;
    }

  if (st.st_size != 2048 + 64)
    {
      printf ("file size after 3rd posix_fallocate call is %llu, expected %u\n",
	      (unsigned long long int) st.st_size, 2048u + 64u);
      return 1;
    }
#ifdef TST_POSIX_FALLOCATE64
  if (posix_fallocate64 (fd, 4097ULL, 4294967295ULL + 2ULL) != 0)
    {
      puts ("4th posix_fallocate call failed");
      return 1;
    }

  if (fstat64 (fd, &st) != 0)
    {
      puts ("5th fstat failed");
      return 1;
    }

  if (st.st_size != 4097ULL + 4294967295ULL + 2ULL)
    {
      printf ("file size after 4th posix_fallocate call is %llu, expected %llu\n",
	      (unsigned long long int) st.st_size, 4097ULL + 4294967295ULL + 2ULL);
      return 1;
    }
#endif
  close (fd);

  return 0;
}
