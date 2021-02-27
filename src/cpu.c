#include "cpu/cpu.h"
#include "cpu/registers.h"
#include "cpu/opcodes.h"
#include "bus.h"
#include "log.h"
#include "cpu/interrupts.h"


static inline void log_registers(struct registers *regs)
{
    log("DEBUG: AF=%04x, BC=%04x, DE=%04x, HL=%04x, SP=%04x, PC=%04x",
        regs->af, regs->bc, regs->de, regs->hl, regs->sp, regs->pc);
}

void cpu_loop()
{
    struct registers regs;
    struct opcode *opcode;
    uint8_t current_opcode, cycles = 0;
    enum cpu_state state = STATE_NORMAL;

    init_registers(&regs);
    if (irq_init())
    {
        return;
    }
    register_opcodes();
    
    while(1)
    {
        if (cycles == 0)
        {
            if (handle_interrups(&regs, &state, &cycles))
            {
                goto end;
            }

            if (state == STATE_NORMAL)
            {
                // Read next opcode
                if (bus_read(&current_opcode, regs.pc))
                {
                    log("ERROR: Failed to read opcode!");
                    return;
                }
                opcode = &opcodes[current_opcode];

                // Call opcode handler
                log_registers(&regs);
                if (opcode->func(&regs, &state))
                {
                    log("ERROR: Opcode handler failed!");
                    return;
                }

                cycles = opcode->cycles;
                regs.pc += opcode->size;
            }
        }

        cycles--;
    }

end:
    irq_end();
}

