#ifndef OPCODES__
#define OPCODES__

#include "cpu/registers.h"

#define NUM_OPCODES 0x100
 
typedef int(*opcode_func_t)(struct registers *regs);
 
struct opcode {
    opcode_func_t func;
    uint8_t size;
    uint8_t cycles;
};

struct opcode opcodes[NUM_OPCODES];

#define OPCODE(name) int name(struct registers *regs)
#define ADD_OPCODE(_opcode, _size, _cycles, _func) opcodes[_opcode].func = _func; \
                                                   opcodes[_opcode].size = _size; \
                                                   opcodes[_opcode].cycles = _cycles;



void register_opcodes();

#endif

