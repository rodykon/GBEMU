#include "cpu/timer.h"
#include "cpu/cpu.h"
#include "bus.h"


int timer_read(uint8_t *result, uint16_t addr)
{
    uint16_t abs_addr = DIV_ADDR + addr;

    switch (abs_addr)
    {
    case DIV_ADDR:
        *result = (uint8_t)(cpu.timer_regs.div >> 8);
        break;
    case TIMA_ADDR:
        *result = cpu.timer_regs.tima;
        break;
    case TMA_ADDR:
        *result = cpu.timer_regs.tma;
        break;
    case TAC_ADDR:
        *result = cpu.timer_regs.tac & 7;
        break;
    default:
        return -1;
    }
    return 0;
}

int timer_write(uint8_t val, uint16_t addr)
{
    uint16_t abs_addr = DIV_ADDR + addr;

    switch (abs_addr)
    {
    case DIV_ADDR:
        cpu.timer_regs.div = 0;
        break;
    case TIMA_ADDR:
        cpu.timer_regs.tima = val;
        break;
    case TMA_ADDR:
        cpu.timer_regs.tma = val;
        break;
    case TAC_ADDR:
        cpu.timer_regs.tac = val & 7;
        break;
    default:
        return -1;
    }
    return 0;
}

int timer_init(struct timer_regs *timer_regs)
{
    timer_regs->div = 0xABCC;
    timer_regs->tima = 0;
    timer_regs->tma = 0;
    timer_regs->tac = 0;
    timer_regs->overflow_counter = 0;
    timer_regs->prev_div_bit = 0;

    return add_bus_connection(DIV_ADDR, 4, timer_read, timer_write);
}

int timer_end()
{
    return remove_bus_connection(DIV_ADDR);
}

// Maps value of TAC.freq to bit of DIV that needs to overflow for timer increment.
static uint8_t freq_to_div_bit[4] = {9, 3, 5, 7};

void timer_update()
{
    uint8_t div_bit;

    cpu.timer_regs.div++;

    if (cpu.timer_regs.overflow_counter > 0)
    {
        if (cpu.timer_regs.tima) // TIMA has been written to
        {
            cpu.timer_regs.overflow_counter = 0;
            return;
        }
        cpu.timer_regs.overflow_counter++;
        if (cpu.timer_regs.overflow_counter >= 4)
        {
            cpu.timer_regs.overflow_counter = 0;
            cpu.timer_regs.tima = cpu.timer_regs.tma;
            cpu.if_flags.timer_irq = 1;
        }
        return;
    }

    div_bit = (cpu.timer_regs.div >> freq_to_div_bit[TAC_FREQ(cpu.timer_regs.tac)]) & TAC_ENABLE(cpu.timer_regs.tac);
    if (div_bit && !cpu.timer_regs.prev_div_bit)
    {
        cpu.timer_regs.tima++;
        if (cpu.timer_regs.tima == 0) // Timer overflow
        {
            cpu.timer_regs.overflow_counter++;
        }
    }
    cpu.timer_regs.prev_div_bit = div_bit;
}