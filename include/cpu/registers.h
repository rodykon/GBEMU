#ifndef REGISTERS__
#define REGISTERS__

#include <inttypes.h>

#define __REG(n1, n2) \
    union { struct { uint8_t n2; uint8_t n1; }; uint16_t n1 ## n2; };

struct flag_reg {
	uint8_t nu : 4; // Not used
	uint8_t c : 1;  // Carry Flag
	uint8_t h : 1;  // Half Carry Flag
	uint8_t n : 1; // Add/Sub Flag
	uint8_t z : 1;  // Zero Flag
};

struct registers {
    union
    {
        struct
        {
            struct flag_reg f;
            uint8_t a;
        };
        uint16_t af;
    };
    __REG(b, c);
    __REG(d, e);
    __REG(h, l);
	uint16_t sp;
	uint16_t pc;
};

#define GET_MSB(reg) (uint8_t)(reg >> 8)
#define GET_LSB(reg) (uint8_t)(reg & 0xff)
#define SET_MSB(reg, val) reg = (reg & 0x00ff) + (val << 8)
#define SET_LSB(reg, val) reg = (reg & 0xff00) + val

static inline void init_registers(struct registers *regs)
{
    regs->f.nu = 0;
    regs->f.c = 0;
    regs->f.h = 0;
    regs->f.n = 0;
    regs->f.z = 0;
    regs->af = 0x01B0;
    regs->bc = 0x0013;
    regs->de = 0x00D8;
    regs->hl = 0x014D;
    regs->sp = 0xFFFE;
    regs->pc = 0x0100;
}


#endif

