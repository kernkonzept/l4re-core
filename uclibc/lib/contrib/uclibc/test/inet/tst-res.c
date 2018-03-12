#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <netdb.h>

int main(int argc, char **argv)
{
    int r;
    struct __res_state state;

    r = res_ninit(&state);
    if (r) {
        herror("ninit");
		abort();
	}
    r = res_init();
    if (r) {
        herror("init");
		abort();
	}

#ifdef __UCLIBC_HAS_BSD_RES_CLOSE__
    res_close();
#endif
#ifdef __UCLIBC__
	/* assume there is at least one resolver configured */
	assert (state._u._ext.nscount > 0);
#else
	assert (state._u._ext.nscount == 0);
#endif
	assert (state.options & RES_INIT);
    res_nclose(&state);
#ifdef __UCLIBC__
	/* We wipe the whole thing */
	assert ((state.options & RES_INIT) == 0);
#endif
	assert (state._u._ext.nscount == 0);

    return 0;
}

