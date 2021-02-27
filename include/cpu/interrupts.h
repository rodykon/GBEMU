#ifndef INTERRUPTS__
#define INTERRUPTS

#include <inttypes.h>
#include "cpu/registers.h"
#include "cpu/cpu.h"

#define NUM_INTERRUPTS 5

#define VB_IRQ     0x0040 // Vertical Blanking Interrupt
#define LCD_IRQ    0x0048 // LCD STAT Interrupt
#define TIMER_IRQ  0x0050 // Timer Interrupt
#define SERIAL_IRQ 0x0058 // Serial Interrupt
#define JP_IRQ     0x0060 // Joypad Interrupt

#define IF_FLAGS_ADDR 0xFF0F
#define IE_FLAGS_ADDR 0xFFFF


struct irq_register
{
    uint8_t vb_irq : 1;
    uint8_t lcd_irq : 1;
    uint8_t timer_irq : 1;
    uint8_t serial_irq : 1;
    uint8_t joypad_irq : 1;
    uint8_t unused : 3;
};

// Interrupt Master Enable flag
uint8_t ime;

// Interrupt Flags
struct irq_register if_flags;

// Interrupt Enable
struct irq_register ie_flags;

// Bus handlers
int irq_if_read(uint8_t *result, uint16_t addr);
int irq_if_write(uint8_t val, uint16_t addr);
int irq_ie_read(uint8_t *result, uint16_t addr);
int irq_ie_write(uint8_t val, uint16_t addr);

int irq_init();
int irq_end();

int handle_interrups(struct registers *regs, enum cpu_state *state, uint8_t *cycles);

#endif