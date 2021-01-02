#include "cpu/cpu.h"
#include "cpu/registers.h"
#include "cpu/opcodes.h"
#include "bus.h"
#include "log.h"


void cpu_loop()
{
    struct registers regs;
    struct opcode *opcode;
    uint8_t current_opcode, cycles = 0;

    init_registers(&regs);
    register_opcodes();

    while(1)
    {
        if (cycles == 0)
        {
            // Read next opcode
            if (bus_read(&current_opcode, regs.pc))
            {
                log("ERROR: Failed to read opcode!");
                return;
            }
            opcode = &opcodes[current_opcode];

            // Call opcode handler
            log("INFO: Executing handler for opcode %02x", current_opcode);
            if (opcode->func(&regs))
            {
                log("ERROR: Opcode handler failed!");
                return;
            }

            cycles = opcode->cycles;
            regs.pc += opcode->size;
        }

        cycles--;
    }
}

