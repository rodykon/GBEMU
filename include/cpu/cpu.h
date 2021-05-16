#ifndef CPU__
#define CPU__

#include "cpu/interrupts.h"

enum cpu_state
{
    STATE_NORMAL,
    STATE_HALT,
    STATE_STOP
};


struct cpu_struct
{
    struct registers regs;
    enum cpu_state state;
    uint8_t ime; // Interrupt Master Enable flag
    struct irq_register if_flags; // Interrupt Flags
    struct irq_register ie_flags; // Interrupt Enable
};

// Global CPU.
extern struct cpu_struct cpu;

void cpu_loop();

#endif

