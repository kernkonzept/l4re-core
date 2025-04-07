
#define PC(ctx) (ctx.arm_pc)

/*
 * Signal context structure - contains all info to do with the state
 * before the signal handler was invoked.  Note: only add new entries
 * to the end of the structure.
 */
struct sigcontext {
	unsigned long fault_address;
	unsigned long regs[31];
	unsigned long sp;
	unsigned long pc;
	unsigned long pstate;
	unsigned char __reserved[4096] __attribute__((__aligned__(16)));
};

