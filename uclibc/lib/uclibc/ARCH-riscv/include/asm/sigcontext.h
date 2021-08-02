struct sigcontext {
  /* gregs[0] holds the program counter. */
  unsigned long gregs[32];
  unsigned long long fpregs[66] __attribute__((aligned(16)));
};
