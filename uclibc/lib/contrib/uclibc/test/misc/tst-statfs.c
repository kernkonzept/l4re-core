#include <sys/vfs.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(int argc, char* argv[])
{
	struct statfs s;
	int ret = 0, i;

	for (i = 1; i < argc; i++) {
		if (statfs(argv[i], &s) != 0) {
			fprintf(stderr, "%s: %s: statfs failed. %s\n",
				*argv, argv[i], strerror(errno));
			exit(EXIT_FAILURE);
		}
		++ret;
		printf("statfs %s:\n\tblocks=%lld\n\tblkfree=%lld\n\tbsize=%d\n",
			argv[i], s.f_blocks, s.f_bfree, s.f_bsize);
#ifdef _STATFS_F_FRSIZE
		printf("\tfrsize=%lld\n", s.f_frsize);
#elif defined __mips__
		printf("\tfrsize=mips, unsupported?\n");
#else
# error no _STATFS_F_FRSIZE
#endif
	}
	exit(ret ? EXIT_SUCCESS : EXIT_FAILURE);
}
