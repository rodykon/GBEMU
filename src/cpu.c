#include "cpu/cpu.h"

inline void init_registers(struct registers *regs)
{
	regs->fl.nu = 0;
	regs->af = 0x01B0;
	regs->bc = 0x0013;
	regs->de = 0x00D8;
	regs->hl = 0x014D;
	regs->sp = 0xFFFE;
	regs->pc = 0x0100;
}
