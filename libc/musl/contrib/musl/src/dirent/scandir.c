#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <stddef.h>

int scandir(const char *path, struct dirent ***res,
	int (*sel)(const struct dirent *),
	int (*cmp)(const struct dirent **, const struct dirent **))
{
	DIR *d = opendir(path);
	struct dirent *de, **names=0, **tmp;
	size_t cnt=0, len=0;
	int old_errno = errno;

	if (!d) return -1;

	while ((errno=0), (de = readdir(d))) {
		if (sel) {
			/* sel() must not observe that errno was set to 0. */
			errno = old_errno;
			if (!sel(de)) continue;
		}
		if (cnt >= len) {
			len = 2*len+1;
			if (len > SIZE_MAX/sizeof *names) break;
			tmp = realloc(names, len * sizeof *names);
			if (!tmp) break;
			names = tmp;
		}
		names[cnt] = malloc(de->d_reclen);
		if (!names[cnt]) break;
		memcpy(names[cnt++], de, de->d_reclen);
	}
	/* closedir() might set errno via __aio_close(). It might also "fail"
	 * (return -1 and set errno). But even then, the file descriptor is
	 * closed and memory is freed, so there is no reason to report the
	 * "failure" of closedir() as a failure of scandir(). */
	int err = errno;

	closedir(d);

	if (err) {
		if (names) while (cnt-->0) free(names[cnt]);
		free(names);
		errno = err;
		return -1;
	}
	/* cmp() and caller must not observe that errno was set to 0. */
	errno = old_errno;

	if (cmp) qsort(names, cnt, sizeof *names, (int (*)(const void *, const void *))cmp);
	*res = names;
	return cnt;
}
