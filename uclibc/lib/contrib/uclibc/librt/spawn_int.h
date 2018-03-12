/* Data structure to contain the action information.  */
struct __spawn_action {
	enum {
		spawn_do_close,
		spawn_do_dup2,
		spawn_do_open
	} tag;

	union {
		struct {
			int fd;
		} close_action;
		struct {
			int fd;
			int newfd;
		} dup2_action;
		struct {
			int fd;
			const char *path;
			int oflag;
			mode_t mode;
		} open_action;
	} action;
};

int __posix_spawn_file_actions_realloc(posix_spawn_file_actions_t *fa);

/* handle !LFS */
#ifndef __UCLIBC_HAS_LFS__
# define rlimit64 rlimit
# define getrlimit64 getrlimit
#endif
#ifndef O_LARGEFILE
# define O_LARGEFILE 0
#endif
