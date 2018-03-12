#include <sys/statvfs.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(int argc, char* argv[])
{
	struct statvfs s;
	int i;

	for (i = 1; i < argc; i++) {
		if (statvfs(argv[i], &s) != 0) {
			fprintf(stderr, "%s: %s: statvfs failed. %s\n",
				*argv, argv[i], strerror(errno));
			exit(EXIT_FAILURE);
		}
		printf("statvfs %s:\n\tblocks=%lld\n\tblkfree=%lld\n\tbsize=%d\n",
			argv[i], s.f_blocks, s.f_bfree, s.f_bsize);
#if 1 // def _STATFS_F_FRSIZE
		printf("\tfrsize=%lld\n", s.f_frsize);
#endif
	}
	exit(EXIT_SUCCESS);
}
