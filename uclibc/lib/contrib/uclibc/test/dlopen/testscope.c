#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#define LIBNAME "libA.so"
int main(int argc, char **argv)
{
	void *libA;
	void (*libAfn)(void);
	char *error;

	libA = dlopen(LIBNAME, RTLD_LAZY);
	if (!libA) {
		fprintf(stderr, "Could not open ./%s: %s\n", LIBNAME, dlerror());
		exit(1);
	}

	libAfn = dlsym(libA, "libA_func");
	if ((error = dlerror()) != NULL)  {
		fprintf(stderr, "Could not locate symbol 'libA_func': %s\n", error);
		exit(1);
	}

	libAfn();

	dlclose(libA);

	return EXIT_SUCCESS;
}
