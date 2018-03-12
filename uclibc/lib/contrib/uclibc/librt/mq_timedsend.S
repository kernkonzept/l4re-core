#include <sysdep-cancel.h>
#ifndef __NR_mq_timedsend
#error Missing definition of NR_timedsend needed for cancellation.
#endif
PSEUDO(mq_timedsend, mq_timedsend, 5)
ret_ERRVAL
PSEUDO_END(mq_timedsend)
librt_hidden_def(mq_timedsend)
