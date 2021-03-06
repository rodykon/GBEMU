#include "cpu/interrupts.h"
#include "bus.h"
#include "log.h"
#include "cpu/opcodes.h"
#include "cpu/cpu.h"
#include "mem_utils.h"

static uint16_t irq_addresses[] = {VB_IRQ, LCD_IRQ, TIMER_IRQ, SERIAL_IRQ, JP_IRQ};

int irq_if_read(uint8_t *result, uint16_t addr)
{
    *result = *(uint8_t*)&cpu.if_flags;
    return 0;
}

int irq_if_write(uint8_t val, uint16_t addr)
{
    cpu.if_flags = *(struct irq_register*)&val;
    cpu.if_flags.unused = 3;
    return 0;
}

int irq_ie_read(uint8_t *result, uint16_t addr)
{
    *result = *(uint8_t*)&cpu.ie_flags;
    return 0;
}

int irq_ie_write(uint8_t val, uint16_t addr)
{
    cpu.ie_flags = *(struct irq_register*)&val;
    return 0;
}

int irq_init()
{
    cpu.ime = 0;
    *(uint8_t*)&cpu.if_flags = 0xE0;
    *(uint8_t*)&cpu.ie_flags = 0;
    
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

    set_interrupts = *(uint8_t*)&cpu.if_flags & *(uint8_t*)&cpu.ie_flags & 0x1F;
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

// Handle the conditions in which IME is set or reset, this is not trivial since we want to
// change IME after the instruction FOLLOWING IE or ID was executed.
static inline void set_ime(uint8_t *enable_irq, uint8_t *disable_irq)
{
    if (*enable_irq)
    {
        if (*enable_irq >= 2)
        {
            cpu.ime = 1;
            *enable_irq = 0;
        }
        else
        {
            *enable_irq++;
        }
    }

    if (*disable_irq)
    {
        if (*disable_irq >=2)
        {
            cpu.ime = 0;
            *disable_irq = 0;
        }
        else
        {
            *disable_irq++;
        }
    }
}

int handle_interrups(uint8_t *cycles, uint8_t *enable_irq, uint8_t *disable_irq)
{
    uint16_t irq_no = 0xFF;

    set_ime(enable_irq, disable_irq);

    if (cpu.ime)
    {
        irq_no = get_set_interrupt_no();
        if (irq_no < NUM_INTERRUPTS)
        {
            // Push PC to the stack.
            cpu.regs.sp -= 2;
            if (write_word(cpu.regs.pc, cpu.regs.sp))
                return -1;

            // Set PC to irq address.
            cpu.regs.pc = irq_addresses[irq_no];

            // Clear irq.
            *(uint8_t*)&cpu.if_flags ^= 1 << irq_no;

            // Turn off interrupts.
            cpu.ime = 0;

            // Set clock cycles
            *cycles = 20;
            if (cpu.state == STATE_HALT)
                *cycles += 4;

            // Make sure state is set to normal.
            cpu.state = STATE_NORMAL;
        }
    }
    else if (cpu.state == STATE_HALT && (*(uint8_t*)&cpu.if_flags & *(uint8_t*)&cpu.ie_flags & 0x1F))
    {
        cpu.state = STATE_NORMAL;
        *cycles = 4;
    }
    return 0;
}
