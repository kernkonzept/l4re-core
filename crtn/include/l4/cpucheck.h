/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#ifndef __CRTX_CPUCHECK_H
#define __CRTX_CPUCHECK_H

#ifndef __ASSEMBLER__
#define CPU_MODEL(model)	".section\t.cpucheck,\"a\",@nobits\n\t"	\
				/*"__cpu_model_" #model ":\n\t"	*/	\
				".comm __cpu_model_" #model ", 4\n\t"	\
				".text\n\t"

#if CPU==486

  asm(CPU_MODEL(4));

#elif CPU==586

  asm(CPU_MODEL(5));

#elif CPU==686
  
  asm(CPU_MODEL(6));

#endif

#undef CPU_SECTION

#endif /* __ASSEMBLER__ */

#endif /* __CRTX_CPUCHECK_H */

