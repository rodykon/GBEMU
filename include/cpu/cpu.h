#ifndef CPU__
#define CPU

#include <inttypes.h>


/*    Registers    */
struct flag_reg {
	uint8_t nu : 4;
	uint8_t c : 1;
	uint8_t h : 1;
	uint8_t n : 1;
	uint8_t z : 1;
}

struct registers {
	struct flag_reg fl;
	uint16_t af;
	uint16_t bc;
	uint16_t de;
	uint16_t hl;
	uint16_t sp;
	uint16_t pc;
}

#define GET_MSB(reg) (uint8_t)(reg >> 4)
#define GET_LSB(reg) (uint8_t)(reg & 0xff)
#define SET_MSB(reg, val) reg = reg ^ 0xff00 + val
#define SET_LSB(reg, val) reg = reg ^ 0x00ff + val << 4


inline void init_registers(struct registers *regs);


/*    Opcodes    */
#define NUM_OPCODES 0x100

typedef int(*opcode_func_t)(struct registers *regs, uint16_t address)

struct opcode {
	opcode_func_t func;
	uint8_t size;
	uint8_t cycles;
}

struct opcode opcodes[NUM_OPCODES];

#define ADD_OPCODE(opcode, size, cycles, func) opcodes[opcode] = { func, size, cycles };


#endif

