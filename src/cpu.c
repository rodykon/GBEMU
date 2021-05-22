#include "cpu/cpu.h"
#include "cpu/registers.h"
#include "cpu/opcodes.h"
#include "bus.h"
#include "log.h"
#include "cpu/interrupts.h"
#include "cpu/timer.h"


struct cpu_struct cpu;

static inline void log_registers(struct registers *regs)
{
    log("DEBUG: AF=%04x, BC=%04x, DE=%04x, HL=%04x, SP=%04x, PC=%04x",
        regs->af, regs->bc, regs->de, regs->hl, regs->sp, regs->pc);
}

static inline int cpu_init()
{
    init_registers(&cpu.regs);
    if (irq_init())
        return -1;
    register_opcodes();
    timer_init(&cpu.timer_regs);
}

void cpu_loop()
{
    struct opcode *opcode;
    uint8_t current_opcode, cycles = 0, disable_irq = 0, enable_irq = 0;
    
    cpu_init();

    while(1)
    {
        if (cycles == 0)
        {
            if (handle_interrups(&cycles, &enable_irq, &disable_irq))
            {
                goto end;
            }

            if (cpu.state == STATE_NORMAL)
            {
                // Read next opcode (fetch)
                if (bus_read(&current_opcode, cpu.regs.pc))
                {
                    log("ERROR: Failed to read opcode!");
                    goto end;
                }

                // Extract from opcode table (decode)
                opcode = &opcodes[current_opcode];

                // Call opcode handler (execute)
                log_registers(&cpu.regs);
                if (opcode->func(&cpu.regs, &cpu.state, &enable_irq, &disable_irq))
                {
                    log("ERROR: Opcode handler failed!");
                    goto end;
                }

                cycles = opcode->cycles;
                cpu.regs.pc += opcode->size;
            }
        }

        timer_update();
        cycles--;
    }

end:
    irq_end();
    timer_end();
}

