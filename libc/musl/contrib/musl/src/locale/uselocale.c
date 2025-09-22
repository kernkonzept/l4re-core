#include "locale_impl.h"
#include "pthread_impl.h"
#include "libc.h"

locale_t __uselocale(locale_t new)
{
	pthread_libc_data_t *self = __pthread_libc_data(__pthread_self());
	locale_t old = self->locale;
	locale_t global = &libc.global_locale;

	if (new) self->locale = new == LC_GLOBAL_LOCALE ? global : new;

	return old == global ? LC_GLOBAL_LOCALE : old;
}

weak_alias(__uselocale, uselocale);
