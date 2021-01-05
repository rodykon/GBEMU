#ifndef REGISTERS__
#define REGISTERS__

#include <inttypes.h>

struct flag_reg {
	uint8_t nu : 4; // Not used
	uint8_t c : 1;  // Carry Flag
	uint8_t h : 1;  // Half Carry Flag
	uint8_t n : 1; // Add/Sub Flag
	uint8_t z : 1;  // Zero Flag
};

struct registers {
	struct flag_reg fl;
	uint16_t af;
	uint16_t bc;
	uint16_t de;
	uint16_t hl;
	uint16_t sp;
	uint16_t pc;
};

#define GET_MSB(reg) (uint8_t)(reg >> 8)
#define GET_LSB(reg) (uint8_t)(reg & 0xff)
#define SET_MSB(reg, val) reg = (reg & 0x00ff) + (val << 8)
#define SET_LSB(reg, val) reg = (reg & 0xff00) + val

static inline void init_registers(struct registers *regs)
{
    regs->fl.nu = 0;
    regs->fl.c = 0;
    regs->fl.h = 0;
    regs->fl.n = 0;
    regs->fl.z = 0;
    regs->af = 0x01B0;
    regs->bc = 0x0013;
    regs->de = 0x00D8;
    regs->hl = 0x014D;
    regs->sp = 0xFFFE;
    regs->pc = 0x0100;
}


#endif

