extern void libC_func(void);

void libB_func(void);
void libB_func(void)
{
	libC_func();
}
