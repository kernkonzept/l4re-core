.section ".init"

//bl __pthread_initialize_minimal@plt
// with @plt it gives asm error, now wrong??
bl __pthread_initialize_minimal

