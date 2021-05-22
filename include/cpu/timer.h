#ifndef TIMER__
#define TIMER__

#include <inttypes.h>

struct timer_regs
{
    uint16_t div; // Divider Register
    uint8_t tac; // Timer control
    uint8_t tima; // Timer Counter
    uint8_t tma; // Timer Modulo
    uint8_t prev_div_bit; // Used for internal timer logic (falling edge detection).
    uint8_t overflow_counter; // Number of clock ticks since overflow.
};


int timer_read(uint8_t *result, uint16_t addr);
int timer_write(uint8_t val, uint16_t addr);

#define DIV_ADDR 0xFF04
#define TIMA_ADDR 0xFF05
#define TMA_ADDR 0xFF06
#define TAC_ADDR 0xFF07
#define TAC_ENABLE(tac) (tac & 4)
#define TAC_FREQ(tac)   (tac & 3)

int timer_init(struct timer_regs *timer_regs);

void timer_update();

#endif
