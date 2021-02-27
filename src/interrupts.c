#include "cpu/interrupts.h"
#include "bus.h"
#include "log.h"
#include "cpu/opcodes.h"
#include "mem_utils.h"

// Interrupt Master Enable flag
uint8_t ime = 0;

// Interrupt Flags
struct irq_register if_flags = { 0, 0, 0, 0, 0, 3};

// Interrupt Enable
struct irq_register ie_flags = { 0 };

static uint16_t irq_addresses[] = {VB_IRQ, LCD_IRQ, TIMER_IRQ, SERIAL_IRQ, JP_IRQ};

int irq_if_read(uint8_t *result, uint16_t addr)
{
    *result = *(uint8_t*)&if_flags;
    return 0;
}

int irq_if_write(uint8_t val, uint16_t addr)
{
    if_flags = *(struct irq_register*)&val;
    if_flags.unused = 3;
    return 0;
}

int irq_ie_read(uint8_t *result, uint16_t addr)
{
    *result = *(uint8_t*)&ie_flags;
    return 0;
}

int irq_ie_write(uint8_t val, uint16_t addr)
{
    ie_flags = *(struct irq_register*)&val;
    return 0;
}

int irq_init()
{
    ime = 0;
    *(uint8_t*)&if_flags = 0xE0;
    *(uint8_t*)&ie_flags = 0;
    
    // Add if and ie bus addresses.
    if (add_bus_connection(IF_FLAGS_ADDR, 1, irq_if_read, irq_if_write))
    {
        log("ERROR: Failed to initialize IRQ.");
        return -1;
    }
    
    if (add_bus_connection(IE_FLAGS_ADDR, 1, irq_if_read, irq_if_write))
    {
        log("ERROR: Failed to initialize IRQ.");
        remove_bus_connection(IF_FLAGS_ADDR);
        return -1;
    }

    return 0;
}

int irq_end()
{
    if (remove_bus_connection(IF_FLAGS_ADDR) || remove_bus_connection(IE_FLAGS_ADDR))
    {
        return -1;
    }
    return 0;
}

static uint8_t get_set_interrupt_no()
{
    uint8_t set_interrupts, interrupt_no = 0;

    set_interrupts = *(uint8_t*)&if_flags & *(uint8_t*)&ie_flags & 0x1F;
    if (set_interrupts)
    {
        while (interrupt_no < NUM_INTERRUPTS)
        {
            if (set_interrupts & 1)
            {
                return interrupt_no;
            }
            set_interrupts >>= 1;
            interrupt_no++;
        }
    } 
    return 0xFF;
}

int handle_interrups(struct registers *regs, enum cpu_state *state, uint8_t *cycles)
{
    uint16_t irq_no = 0xFF;

    if (ime)
    {
        irq_no = get_set_interrupt_no();
        if (irq_no < NUM_INTERRUPTS)
        {
            // Push PC to the stack.
            regs->sp -= 2;
            if (write_word(regs->pc, regs->sp))
            {
                return -1;
            }

            // Set PC to irq address.
            regs->pc = irq_addresses[irq_no];

            // Clear irq.
            *(uint8_t*)&if_flags ^= 1 << irq_no;

            // Turn off interrupts.
            ime = 0;

            // Set clock cycles
            *cycles = 20;
            if (*state == STATE_HALT)
            {
                *cycles += 4;
            }

            // Make sure state is set to normal.
            *state = STATE_NORMAL;
        }
        else if (*state == STATE_HALT && (*(uint8_t*)&if_flags & *(uint8_t*)&ie_flags & 0x1F))
        {
            *state = STATE_NORMAL;
            *cycles = 4;
        }
    }
    return 0;
}
