#include "cpu/opcodes.h"
#include "bus.h"


OPCODE(NOP)
{
    return 0;
}

OPCODE(LD1)
{
    uint8_t arg = 0;
    int result = 0;

    if (bus_read(&arg, regs->pc + 1))
    {
        return -1;
    }

    SET_LSB(regs->bc, arg);
    
    if (bus_read(&arg, regs->pc + 2))
    {
        return -1;
    }

    SET_MSB(regs->bc, arg);
    return 0;
}

OPCODE(LD2)
{
    return bus_write(GET_MSB(regs->af), regs->bc);
}

void register_opcodes()
{
    ADD_OPCODE(0x00, 1, 1, NOP);
    ADD_OPCODE(0x01, 3, 3, LD1);
    ADD_OPCODE(0x02, 1, 2, LD2);
}