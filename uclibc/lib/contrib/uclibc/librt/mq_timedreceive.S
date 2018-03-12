#include <sysdep-cancel.h>
#ifndef __NR_mq_timedreceive
#error Missing definition of NR_timedreceive needed for cancellation.
#endif
PSEUDO(mq_timedreceive, mq_timedreceive, 5)
ret_ERRVAL
PSEUDO_END(mq_timedreceive)
librt_hidden_def(mq_timedreceive)
