// If our libc has largefile support enabled:
#if 0
#ifdef __SIZEOF_LONG__ == 8
#define _FILE_OFFSET_BITS 64
#endif
#endif
