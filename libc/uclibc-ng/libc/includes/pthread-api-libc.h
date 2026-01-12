#pragma once

// This is not used by uclibc-ng, but we still declare it here, see musl version
// for details.
L4_HIDDEN void __pthread_l4_initialize_dynlink_thread(pthread_descr descr);
