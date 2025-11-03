#include <errno.h>
#include <spawn.h>
#include <stdio.h>

int posix_spawn(pid_t *restrict pid, const char *restrict path,
                const posix_spawn_file_actions_t *restrict file_actions,
                const posix_spawnattr_t *restrict attrp,
                char *const argv[restrict],
                char *const envp[restrict])
{
  (void)pid;
  (void)path;
  (void)file_actions;
  (void)attrp;
  (void)argv;
  (void)envp;
  return -ENOTSUP;
}

int posix_spawnp(pid_t *restrict pid, const char *restrict file,
                 const posix_spawn_file_actions_t *restrict file_actions,
                 const posix_spawnattr_t *restrict attrp,
                 char *const argv[restrict],
                 char *const envp[restrict])
{
  (void)pid;
  (void)file;
  (void)file_actions;
  (void)attrp;
  (void)argv;
  (void)envp;
  return -ENOTSUP;
}

int posix_spawn_file_actions_addclose(posix_spawn_file_actions_t *file_actions,
                                      int fildes)
{
  (void)file_actions;
  (void)fildes;
  return -ENOTSUP;
}

int posix_spawn_file_actions_addopen(posix_spawn_file_actions_t *restrict file_actions,
                                     int fildes,
                                     const char *restrict path,
                                     int oflag,
                                     mode_t mode)
{
  (void)file_actions;
  (void)fildes;
  (void)path;
  (void)oflag;
  (void)mode;
  return -ENOTSUP;
}

int posix_spawn_file_actions_adddup2(posix_spawn_file_actions_t *file_actions,
                                     int fildes,
                                     int newfildes)
{
  (void)file_actions;
  (void)fildes;
  (void)newfildes;
  return -ENOTSUP;
}
