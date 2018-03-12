#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#define LIBNAME "libB.so"
void _libC_fini(void);
void _libC_fini(void)
{
	printf("libC_fini():finish - atexit()\n");
}

void libC_fini(void);
void libC_fini(void)
{
	_libC_fini();
}

void libC_func(void);
void libC_func(void)
{
	void *libB;

	libB = dlopen(LIBNAME, RTLD_LAZY);
	if (!libB) {
		fprintf(stderr, "Could not open ./%s: %s\n", LIBNAME, dlerror());
		exit(1);
	}

	atexit(libC_fini);
}
