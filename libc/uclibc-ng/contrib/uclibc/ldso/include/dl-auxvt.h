#ifndef _DL_AUXVT_H
#define _DL_AUXVT_H

#define AUX_MAX_AT_ID 40
extern ElfW(auxv_t) _dl_auxvt[AUX_MAX_AT_ID]; /* Cache frequently accessed auxiliary vector entries */
extern ElfW(auxv_t) *_dl_auxv_start;          /* Start of the auxiliary vector */

#endif /* _DL_AUXVT_H */

