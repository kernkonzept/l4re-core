#include <netdb.h>

static __thread int __h_errno __attribute__((tls_model("initial-exec")));

int *__h_errno_location(void)
{
  return &__h_errno;
}
