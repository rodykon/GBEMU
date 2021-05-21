#ifndef OPCODES__
#define OPCODES__

#include "cpu/registers.h"
#include "cpu/cpu.h"

#define NUM_OPCODES 0x100
 
typedef int(*opcode_func_t)(struct registers *regs, enum cpu_state *state, uint8_t *enable_irq, uint8_t *disable_irq);
 
struct opcode {
    opcode_func_t func;
    uint8_t size;
    uint8_t cycles;
};

#define OPCODE(name) int name(struct registers *regs, enum cpu_state *state, uint8_t *enable_irq, uint8_t *disable_irq)
#define ADD_OPCODE(_opcode, _size, _cycles, _func) do { \
                                                   opcodes[_opcode].func = _func; \
                                                   opcodes[_opcode].size = _size; \
                                                   opcodes[_opcode].cycles = _cycles; } while (0)

OPCODE(INVAL);

extern struct opcode opcodes[NUM_OPCODES];

void register_opcodes();

#endif

