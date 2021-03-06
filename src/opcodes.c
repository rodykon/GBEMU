#include "cpu/opcodes.h"
#include "bus.h"
#include "log.h"
#include "mem_utils.h"

struct opcode opcodes[NUM_OPCODES] = {INVAL};

/* ----------- Utils ----------- */

#define ZERO(a, b) ((a) + (b) == 0) ? 1 : 0
#define SUB_ZERO(a, b) ((a) - (b) == 0) ? 1 : 0
#define CARRY(a, b) ((a) + (b) < (a)) ? 1 : 0
#define SUB_CARRY(a, b) ((b) > (a)) ? 1 : 0
#define HALF_CARRY(a, b) ((((a) & 0xF) + ((b) & 0xF)) & 0x10) ? 1 : 0
#define SUB_HALF_CARRY(a, b) (((a) & 0xF) < ((b) & 0xF)) ? 1 : 0

static void inc_byte(uint8_t *val, struct registers *regs)
{
    *val++;

    regs->f.z = ZERO(*val, 1);
    regs->f.h = HALF_CARRY(*val, 1);
    regs->f.n = 0;
}

static void dec_byte(uint8_t *val, struct registers *regs)
{
    *val--;

    regs->f.z = SUB_ZERO(*val, 1);
    regs->f.h = SUB_HALF_CARRY(*val, 1);
    regs->f.n = 1;
}

static uint8_t swap(uint8_t to_swap, struct registers *regs)
{
    uint8_t result = ((to_swap << 4) & 0xF0) | ((to_swap >> 4) & 0x0F);
    regs->f.z = result ? regs->f.z : 1;
    regs->f.n = 0;
    regs->f.h = 0;
    regs->f.c = 0;
    return result;
}

static void rlc(uint8_t *to_rot, struct registers *regs)
{
    regs->f.n = 0;
    regs->f.h = 0;
    regs->f.c = *to_rot >> 7;

    *to_rot = (*to_rot << 1) | (*to_rot >> 7);

    regs->f.z = *to_rot == 0 ? 1 : 0;
}

static void rl(uint8_t *to_rot, struct registers *regs)
{
    uint8_t new_c = *to_rot >> 7;

    regs->f.n = 0;
    regs->f.h = 0;
    *to_rot = (*to_rot << 1) | regs->f.c;
    regs->c = new_c & 1;

    regs->f.z = *to_rot == 0 ? 1 : 0;
}

static void rrc(uint8_t *to_rot, struct registers *regs)
{
    regs->f.n = 0;
    regs->f.h = 0;
    regs->f.c = *to_rot & 1;

    *to_rot = (*to_rot >> 1) | (*to_rot << 7);

    regs->f.z = *to_rot == 0 ? 1 : 0;
}

static void rr(uint8_t *to_rot, struct registers *regs)
{
    uint8_t new_c = *to_rot & 1;

    regs->f.n = 0;
    regs->f.h = 0;
    *to_rot = (*to_rot >> 1) | (regs->f.c << 7);
    regs->c = new_c & 1;

    regs->f.z = *to_rot == 0 ? 1 : 0;
}

static uint8_t shift_left(uint8_t to_shift, struct registers *regs)
{
    regs->f.n = 0;
    regs->f.h = 0;
    regs->f.c = to_shift >> 7;
    regs->f.z = ((to_shift << 1) & 0xFF) == 0 ? 1 : 0;
    return (to_shift << 1) & 0xFF;
}

static uint8_t shift_right(uint8_t to_shift, struct registers *regs)
{
    regs->f.n = 0;
    regs->f.h = 0;
    regs->f.c = to_shift & 1;
    regs->f.z = (to_shift >> 1) == 0 ? 1 : 0;
    return to_shift >> 1;
}

static int test_bit(struct registers *regs, uint8_t to_test)
{
    uint8_t n;

    // Read n
    regs->pc++;
    if (bus_read(&n, regs->pc + 1))
    {
        return -1;
    }

    if (n > 7)
    {
        log(LERR "Invalid 'n' argument to BIT operation.");
        return -1;
    }

    // Test bit
    regs->f.z = (to_test >> n) & 1;
    regs->f.n = 0;
    regs->f.h = 1;
    return n;
}

static int set_bit(struct registers *regs, uint8_t *to_set)
{
    uint8_t n;

    // Read n
    regs->pc++;
    if (bus_read(&n, regs->pc + 1))
    {
        return -1;
    }

    if (n > 7)
    {
        log(LERR "Invalid 'n' argument to SET operation.");
        return -1;
    }

    // Set bit
    *to_set = *to_set | (1 << n);
    return n;
}

static int reset_bit(struct registers *regs, uint8_t *to_set)
{
    uint8_t n;

    // Read n
    regs->pc++;
    if (bus_read(&n, regs->pc + 1))
    {
        return -1;
    }

    if (n > 7)
    {
        log(LERR "Invalid 'n' argument to RES operation.");
        return -1;
    }

    // Set bit
    *to_set = *to_set ^ (1 << n);
    return n;
}

/* ----------- Misc. ----------- */

OPCODE(INVAL)
{
    log(LERR "Invalid opcode.");
    return -1;
}

OPCODE(NOP)
{
    log(LDEBUG "NOP");
    return 0;
}

OPCODE(CB)
{
    uint8_t type, value, ret;

    if (bus_read(&type, regs->pc + 1))
    {
        return -1;
    }

    switch (type)
    {
        case 0x07:
            rlc(&regs->a, regs);
            log(LDEBUG "RLC A");
            break;
        case 0x00:
            rlc(&regs->b, regs);
            log(LDEBUG "RLC B");
            break;
        case 0x01:
            rlc(&regs->c, regs);
            log(LDEBUG "RLC C");
            break;
        case 0x02:
            rlc(&regs->d, regs);
            log(LDEBUG "RLC D");
            break;
        case 0x03:
            rlc(&regs->e, regs);
            log(LDEBUG "RLC E");
            break;
        case 0x04:
            rlc(&regs->h, regs);
            log(LDEBUG "RLC H");
            break;
        case 0x05:
            rlc(&regs->l, regs);
            log(LDEBUG "RLC L");
            break;
        case 0x06:
            if (bus_read(&value, regs->hl))
            {
                return -1;
            }
            rlc(&value, regs);
            if (bus_write(value, regs->hl))
            {
                return -1;
            }
            log(LDEBUG "RLC (HL)");
            break;
        case 0x17:
            rl(&regs->a, regs);
            log(LDEBUG "RL A");
            break;
        case 0x10:
            rl(&regs->b, regs);
            log(LDEBUG "RL B");
            break;
        case 0x11:
            rl(&regs->c, regs);
            log(LDEBUG "RL C");
            break;
        case 0x12:
            rl(&regs->d, regs);
            log(LDEBUG "RL D");
            break;
        case 0x13:
            rl(&regs->e, regs);
            log(LDEBUG "RL E");
            break;
        case 0x14:
            rl(&regs->h, regs);
            log(LDEBUG "RL H");
            break;
        case 0x15:
            rl(&regs->l, regs);
            log(LDEBUG "RL L");
            break;
        case 0x16:
            if (bus_read(&value, regs->hl))
            {
                return -1;
            }
            rl(&value, regs);
            if (bus_write(value, regs->hl))
            {
                return -1;
            }
            log(LDEBUG "RL (HL)");
            break;
        case 0x0F:
            rrc(&regs->a, regs);
            log(LDEBUG "RRC A");
            break;
        case 0x08:
            rrc(&regs->b, regs);
            log(LDEBUG "RRC B");
            break;
        case 0x09:
            rrc(&regs->c, regs);
            log(LDEBUG "RRC C");
            break;
        case 0x0A:
            rrc(&regs->d, regs);
            log(LDEBUG "RRC D");
            break;
        case 0x0B:
            rrc(&regs->e, regs);
            log(LDEBUG "RRC E");
            break;
        case 0x0C:
            rrc(&regs->h, regs);
            log(LDEBUG "RRC H");
            break;
        case 0x0D:
            rrc(&regs->l, regs);
            log(LDEBUG "RRC L");
            break;
        case 0x0E:
            if (bus_read(&value, regs->hl))
            {
                return -1;
            }
            rrc(&value, regs);
            if (bus_write(value, regs->hl))
            {
                return -1;
            }
            log(LDEBUG "RRC (HL)");
            break;
        case 0x1F:
            rr(&regs->a, regs);
            log(LDEBUG "RR A");
            break;
        case 0x18:
            rr(&regs->b, regs);
            log(LDEBUG "RR B");
            break;
        case 0x19:
            rr(&regs->c, regs);
            log(LDEBUG "RR C");
            break;
        case 0x1A:
            rr(&regs->d, regs);
            log(LDEBUG "RR D");
            break;
        case 0x1B:
            rr(&regs->e, regs);
            log(LDEBUG "RR E");
            break;
        case 0x1C:
            rr(&regs->h, regs);
            log(LDEBUG "RR H");
            break;
        case 0x1D:
            rr(&regs->l, regs);
            log(LDEBUG "RR L");
            break;
        case 0x1E:
            if (bus_read(&value, regs->hl))
            {
                return -1;
            }
            rr(&value, regs);
            if (bus_write(value, regs->hl))
            {
                return -1;
            }
            log(LDEBUG "RR (HL)");
            break;
        case 0x27:
            regs->a = shift_left(regs->a, regs);
            log(LDEBUG "SLA A");
            break;
        case 0x20:
            regs->b = shift_left(regs->b, regs);
            log(LDEBUG "SLA B");
            break;
        case 0x21:
            regs->c = shift_left(regs->c, regs);
            log(LDEBUG "SLA C");
            break;
        case 0x22:
            regs->d = shift_left(regs->d, regs);
            log(LDEBUG "SLA D");
            break;
        case 0x23:
            regs->e = shift_left(regs->e, regs);
            log(LDEBUG "SLA E");
            break;
        case 0x24:
            regs->h = shift_left(regs->h, regs);
            log(LDEBUG "SLA H");
            break;
        case 0x25:
            regs->l = shift_left(regs->l, regs);
            log(LDEBUG "SLA L");
            break;
        case 0x26:
            if (bus_read(&value, regs->hl) || bus_write(shift_left(value, regs), regs->hl))
            {
                return -1;
            }
            log(LDEBUG "SLA (HL)");
            break;
        case 0x2F:
            regs->a = shift_right(regs->a, regs) | (regs->a & 0x80);
            log(LDEBUG "SRA A");
            break;
        case 0x28:
            regs->b = shift_right(regs->b, regs) | (regs->b & 0x80);
            log(LDEBUG "SRA B");
            break;
        case 0x29:
            regs->c = shift_right(regs->c, regs) | (regs->c & 0x80);
            log(LDEBUG "SRA C");
            break;
        case 0x2A:
            regs->d = shift_right(regs->d, regs) | (regs->d & 0x80);
            log(LDEBUG "SRA D");
            break;
        case 0x2B:
            regs->e = shift_right(regs->e, regs) | (regs->e & 0x80);
            log(LDEBUG "SRA E");
            break;
        case 0x2C:
            regs->h = shift_right(regs->h, regs) | (regs->h & 0x80);
            log(LDEBUG "SRA H");
            break;
        case 0x2D:
            regs->l = shift_right(regs->l, regs) | (regs->l & 0x80);
            log(LDEBUG "SRA L");
            break;
        case 0x2E:
            if (bus_read(&value, regs->hl) || bus_write(shift_right(value, regs) | (value & 0x80), regs->hl))
            {
                return -1;
            }
            log(LDEBUG "SRA (HL)");
            break;
        case 0x3F:
            regs->a = shift_right(regs->a, regs);
            log(LDEBUG "SRL A");
            break;
        case 0x38:
            regs->b = shift_right(regs->b, regs);
            log(LDEBUG "SRL B");
            break;
        case 0x39:
            regs->c = shift_right(regs->c, regs);
            log(LDEBUG "SRL C");
            break;
        case 0x3A:
            regs->d = shift_right(regs->d, regs);
            log(LDEBUG "SRL D");
            break;
        case 0x3B:
            regs->e = shift_right(regs->e, regs);
            log(LDEBUG "SRL E");
            break;
        case 0x3C:
            regs->h = shift_right(regs->h, regs);
            log(LDEBUG "SRL H");
            break;
        case 0x3D:
            regs->l = shift_right(regs->l, regs);
            log(LDEBUG "SRL L");
            break;
        case 0x3E:
            if (bus_read(&value, regs->hl) || bus_write(shift_right(value, regs), regs->hl))
            {
                return -1;
            }
            log(LDEBUG "SRL (HL)");
            break;
        case 0x37:
            regs->a = swap(regs->a, regs);
            log(LDEBUG "SWAP A");
            break;
        case 0x30:
            regs->b = swap(regs->b, regs);
            log(LDEBUG "SWAP B");
            break;
        case 0x31:
            regs->c = swap(regs->c, regs);
            log(LDEBUG "SWAP C");
            break;
        case 0x32:
            regs->d = swap(regs->d, regs);
            log(LDEBUG "SWAP D");
            break;
        case 0x33:
            regs->e = swap(regs->e, regs);
            log(LDEBUG "SWAP E");
            break;
        case 0x34:
            regs->h = swap(regs->h, regs);
            log(LDEBUG "SWAP H");
            break;
        case 0x35:
            regs->l = swap(regs->l, regs);
            log(LDEBUG "SWAP L");
            break;
        case 0x36:
            if (bus_read(&value, regs->hl) || bus_write(swap(value, regs), regs->hl))
            {
                return -1;
            }
            log(LDEBUG "SWAP (HL)");
            break;
        case 0x47:
            value = test_bit(regs, regs->a);
            if (value < 0)
            {
                return -1;
            }
            log(LDEBUG "BIT %d, A", value);
            break;
        case 0x40:
            value = test_bit(regs, regs->b);
            if (value < 0)
            {
                return -1;
            }
            log(LDEBUG "BIT %d, B", value);
            break;
        case 0x41:
            value = test_bit(regs, regs->c);
            if (value < 0)
            {
                return -1;
            }
            log(LDEBUG "BIT %d, C", value);
            break;
        case 0x42:
            value = test_bit(regs, regs->d);
            if (value < 0)
            {
                return -1;
            }
            log(LDEBUG "BIT %d, D", value);
            break;
        case 0x43:
            value = test_bit(regs, regs->e);
            if (value < 0)
            {
                return -1;
            }
            log(LDEBUG "BIT %d, E", value);
            break;
        case 0x44:
            value = test_bit(regs, regs->h);
            if (value < 0)
            {
                return -1;
            }
            log(LDEBUG "BIT %d, H", value);
            break;
        case 0x45:
            value = test_bit(regs, regs->l);
            if (value < 0)
            {
                return -1;
            }
            log(LDEBUG "BIT %d, L", value);
            break;
        case 0x46:
            if (bus_read(&value, regs->hl))
            {
                return -1;
            }
            value = test_bit(regs, value);
            if (value < 0)
            {
                return -1;
            }
            log(LDEBUG "BIT %d, (HL)", value);
            break;
        case 0xC7:
            value = set_bit(regs, &regs->a);
            if (value < 0)
            {
                return -1;
            }
            log(LDEBUG "SET %d, A", value);
            break;
        case 0xC0:
            value = set_bit(regs, &regs->b);
            if (value < 0)
            {
                return -1;
            }
            log(LDEBUG "SET %d, B", value);
            break;
        case 0xC1:
            value = set_bit(regs, &regs->c);
            if (value < 0)
            {
                return -1;
            }
            log(LDEBUG "SET %d, C", value);
            break;
        case 0xC2:
            value = set_bit(regs, &regs->d);
            if (value < 0)
            {
                return -1;
            }
            log(LDEBUG "SET %d, D", value);
            break;
        case 0xC3:
            value = set_bit(regs, &regs->e);
            if (value < 0)
            {
                return -1;
            }
            log(LDEBUG "SET %d, E", value);
            break;
        case 0xC4:
            value = set_bit(regs, &regs->h);
            if (value < 0)
            {
                return -1;
            }
            log(LDEBUG "SET %d, H", value);
            break;
        case 0xC5:
            value = set_bit(regs, &regs->l);
            if (value < 0)
            {
                return -1;
            }
            log(LDEBUG "SET %d, L", value);
            break;
        case 0xC6:
            if (bus_read(&value, regs->hl))
            {
                return -1;
            }
            ret = set_bit(regs, &value);
            if (ret < 0)
            {
                return -1;
            }
            if (bus_write(value, regs->hl))
            {
                return -1;
            }
            log(LDEBUG "SET %d, (HL)", ret);
            break;
        case 0x87:
            value = reset_bit(regs, &regs->a);
            if (value < 0)
            {
                return -1;
            }
            log(LDEBUG "RES %d, A", value);
            break;
        case 0x80:
            value = reset_bit(regs, &regs->b);
            if (value < 0)
            {
                return -1;
            }
            log(LDEBUG "RES %d, B", value);
            break;
        case 0x81:
            value = reset_bit(regs, &regs->c);
            if (value < 0)
            {
                return -1;
            }
            log(LDEBUG "RES %d, C", value);
            break;
        case 0x82:
            value = reset_bit(regs, &regs->d);
            if (value < 0)
            {
                return -1;
            }
            log(LDEBUG "RES %d, D", value);
            break;
        case 0x83:
            value = reset_bit(regs, &regs->e);
            if (value < 0)
            {
                return -1;
            }
            log(LDEBUG "RES %d, E", value);
            break;
        case 0x84:
            value = reset_bit(regs, &regs->h);
            if (value < 0)
            {
                return -1;
            }
            log(LDEBUG "RES %d, H", value);
            break;
        case 0x85:
            value = reset_bit(regs, &regs->l);
            if (value < 0)
            {
                return -1;
            }
            log(LDEBUG "RES %d, L", value);
            break;
        case 0x86:
            if (bus_read(&value, regs->hl))
            {
                return -1;
            }
            ret = reset_bit(regs, &value);
            if (ret < 0)
            {
                return -1;
            }
            if (bus_write(value, regs->hl))
            {
                return -1;
            }
            log(LDEBUG "RES %d, (HL)", ret);
            break;
    }

    return 0;
}

OPCODE(DAA)
{
    if (!regs->f.n) {  // Addition
        if (regs->f.c || regs->a > 0x99) { regs->a += 0x60; regs->f.c = 1; } // Carry or top nibble overflow.
        if (regs->f.h || (regs->a & 0x0f) > 0x09) { regs->a += 0x6; } // Half-carry or low nibble overflow.
    } else {  // Subtraction
        if (regs->f.c) { regs->a -= 0x60; } // Carry
        if (regs->f.h) { regs->a -= 0x6; } // Half-carry
    }

    regs->f.z = (regs->a == 0);
    regs->f.h = 0;

    log(LDEBUG "DAA");
    return 0;
}

OPCODE(CPL)
{
    regs->a = ~regs->a;

    regs->f.n = 1;
    regs->f.h = 1;

    log(LDEBUG "CPL");
    return 0;
}

OPCODE(CCF)
{
    regs->f.c = ~regs->f.c;

    regs->f.n = 0;
    regs->f.h = 0;
    
    log(LDEBUG "CCF");
    return 0;
}

OPCODE(SCF)
{
    regs->f.c = 1;

    regs->f.n = 0;
    regs->f.h = 0;
    
    log(LDEBUG "SCF");
    return 0;
}

OPCODE(HALT)
{
    *state = STATE_HALT;
    log(LDEBUG "HALT");
    return 0;
}

OPCODE(STOP)
{
    *state = STATE_STOP;
    log(LDEBUG "STOP");
    return 0;
}

OPCODE(DI)
{
    *disable_irq++;
    log(LDEBUG "DI");
    return 0;
}

OPCODE(EI)
{
    *enable_irq++;
    log(LDEBUG "EI");
    return 0;
}

OPCODE(RLCA)
{
    rlc(&regs->a, regs);
    log(LDEBUG "RLCA");
    return 0; 
}

OPCODE(RLA)
{
    rl(&regs->a, regs);
    log(LDEBUG "RLA");
    return 0; 
}

OPCODE(RRCA)
{
    rrc(&regs->a, regs);
    log(LDEBUG "RRCA");
    return 0; 
}

OPCODE(RRA)
{
    rr(&regs->a, regs);
    log(LDEBUG "RRA");
    return 0; 
}

/* -------- 8-Bit Loads -------- */

// LD reg8, imm8

OPCODE(LD_B_n)
{
    uint8_t arg;
    if (bus_read(&arg, regs->pc + 1))
    {
        return -1;
    }
    regs->b = arg;
    log(LDEBUG "LD B, 0x%02x", arg);
    return 0;
}

OPCODE(LD_C_n)
{
    uint8_t arg;
    if (bus_read(&arg, regs->pc + 1))
    {
        return -1;
    }
    regs->c = arg;
    log(LDEBUG "LD C, 0x%02x", arg);
    return 0;
}

OPCODE(LD_D_n)
{
    uint8_t arg;
    if (bus_read(&arg, regs->pc + 1))
    {
        return -1;
    }
    regs->d = arg;
    log(LDEBUG "LD D, 0x%02x", arg);
    return 0;
}

OPCODE(LD_E_n)
{
    uint8_t arg;
    if (bus_read(&arg, regs->pc + 1))
    {
        return -1;
    }
    regs->e = arg;
    log(LDEBUG "LD E, 0x%02x", arg);
    return 0;
}

OPCODE(LD_H_n)
{
    uint8_t arg;
    if (bus_read(&arg, regs->pc + 1))
    {
        return -1;
    }
    regs->h = arg;
    log(LDEBUG "LD H, 0x%02x", arg);
    return 0;
}

OPCODE(LD_L_n)
{
    uint8_t arg;
    if (bus_read(&arg, regs->pc + 1))
    {
        return -1;
    }
    regs->l = arg;
    log(LDEBUG "LD L, 0x%02x", arg);
    return 0;
}

// LD reg8, reg8

OPCODE(LD_A_A)
{
    log(LDEBUG "LD A, A");
    return 0;
}


OPCODE(LD_A_B)
{
    regs->a = regs->b;
    log(LDEBUG "LD A, B");
    return 0;
}


OPCODE(LD_A_C)
{
    regs->a = regs->c;
    log(LDEBUG "LD A, C");
    return 0;
}


OPCODE(LD_A_D)
{
    regs->a = regs->d;
    log(LDEBUG "LD A, D");
    return 0;
}


OPCODE(LD_A_E)
{
    regs->a = regs->e;
    log(LDEBUG "LD A, E");
    return 0;
}


OPCODE(LD_A_H)
{
    regs->a = regs->h;
    log(LDEBUG "LD A, H");
    return 0;
}


OPCODE(LD_A_L)
{
    regs->a = regs->l;
    log(LDEBUG "LD A, L");
    return 0;
}


OPCODE(LD_B_A)
{
    regs->b = regs->a;
    log(LDEBUG "LD B, A");
    return 0;
}

OPCODE(LD_B_B)
{
    log(LDEBUG "LD B, B");
    return 0;
}


OPCODE(LD_B_C)
{
    regs->b = regs->c;
    log(LDEBUG "LD B, C");
    return 0;
}


OPCODE(LD_B_D)
{
    regs->b = regs->d;
    log(LDEBUG "LD B, D");
    return 0;
}


OPCODE(LD_B_E)
{
    regs->b = regs->e;
    log(LDEBUG "LD B, E");
    return 0;
}


OPCODE(LD_B_H)
{
    regs->b = regs->h;
    log(LDEBUG "LD B, H");
    return 0;
}


OPCODE(LD_B_L)
{
    regs->b = regs->l;
    log(LDEBUG "LD B, L");
    return 0;
}


OPCODE(LD_C_A)
{
    regs->c = regs->a;
    log(LDEBUG "LD C, A");
    return 0;
}


OPCODE(LD_C_B)
{
    regs->c = regs->b;
    log(LDEBUG "LD C, B");
    return 0;
}

OPCODE(LD_C_C)
{
    log(LDEBUG "LD C, C");
    return 0;
}


OPCODE(LD_C_D)
{
    regs->c = regs->d;
    log(LDEBUG "LD C, D");
    return 0;
}


OPCODE(LD_C_E)
{
    regs->c = regs->e;
    log(LDEBUG "LD C, E");
    return 0;
}


OPCODE(LD_C_H)
{
    regs->c = regs->h;
    log(LDEBUG "LD C, H");
    return 0;
}


OPCODE(LD_C_L)
{
    regs->c = regs->l;
    log(LDEBUG "LD C, L");
    return 0;
}


OPCODE(LD_D_A)
{
    regs->d = regs->a;
    log(LDEBUG "LD D, A");
    return 0;
}


OPCODE(LD_D_B)
{
    regs->d = regs->b;
    log(LDEBUG "LD D, B");
    return 0;
}


OPCODE(LD_D_C)
{
    regs->d = regs->c;
    log(LDEBUG "LD D, C");
    return 0;
}

OPCODE(LD_D_D)
{
    log(LDEBUG "LD D, D");
    return 0;
}


OPCODE(LD_D_E)
{
    regs->d = regs->e;
    log(LDEBUG "LD D, E");
    return 0;
}


OPCODE(LD_D_H)
{
    regs->d = regs->h;
    log(LDEBUG "LD D, H");
    return 0;
}


OPCODE(LD_D_L)
{
    regs->d = regs->l;
    log(LDEBUG "LD D, L");
    return 0;
}


OPCODE(LD_E_A)
{
    regs->e = regs->a;
    log(LDEBUG "LD E, A");
    return 0;
}


OPCODE(LD_E_B)
{
    regs->e = regs->b;
    log(LDEBUG "LD E, B");
    return 0;
}


OPCODE(LD_E_C)
{
    regs->e = regs->c;
    log(LDEBUG "LD E, C");
    return 0;
}


OPCODE(LD_E_D)
{
    regs->e = regs->d;
    log(LDEBUG "LD E, D");
    return 0;
}

OPCODE(LD_E_E)
{
    log(LDEBUG "LD E, E");
    return 0;
}


OPCODE(LD_E_H)
{
    regs->e = regs->h;
    log(LDEBUG "LD E, H");
    return 0;
}


OPCODE(LD_E_L)
{
    regs->e = regs->l;
    log(LDEBUG "LD E, L");
    return 0;
}


OPCODE(LD_H_A)
{
    regs->h = regs->a;
    log(LDEBUG "LD H, A");
    return 0;
}


OPCODE(LD_H_B)
{
    regs->h = regs->b;
    log(LDEBUG "LD H, B");
    return 0;
}


OPCODE(LD_H_C)
{
    regs->h = regs->c;
    log(LDEBUG "LD H, C");
    return 0;
}


OPCODE(LD_H_D)
{
    regs->h = regs->d;
    log(LDEBUG "LD H, D");
    return 0;
}


OPCODE(LD_H_E)
{
    regs->h = regs->e;
    log(LDEBUG "LD H, E");
    return 0;
}

OPCODE(LD_H_H)
{
    log(LDEBUG "LD H, H");
    return 0;
}


OPCODE(LD_H_L)
{
    regs->h = regs->l;
    log(LDEBUG "LD H, L");
    return 0;
}


OPCODE(LD_L_A)
{
    regs->l = regs->a;
    log(LDEBUG "LD L, A");
    return 0;
}


OPCODE(LD_L_B)
{
    regs->l = regs->b;
    log(LDEBUG "LD L, B");
    return 0;
}


OPCODE(LD_L_C)
{
    regs->l = regs->c;
    log(LDEBUG "LD L, C");
    return 0;
}


OPCODE(LD_L_D)
{
    regs->l = regs->d;
    log(LDEBUG "LD L, D");
    return 0;
}


OPCODE(LD_L_E)
{
    regs->l = regs->e;
    log(LDEBUG "LD L, E");
    return 0;
}


OPCODE(LD_L_H)
{
    regs->l = regs->h;
    log(LDEBUG "LD L, H");
    return 0;
}

OPCODE(LD_L_L)
{
    log(LDEBUG "LD L, L");
    return 0;
}

// LD reg8, (reg16)

OPCODE(LD_A_BC)
{
    uint8_t new_val;

    if (bus_read(&new_val, regs->bc))
    {
        return -1;
    }
    regs->a = new_val;
    log(LDEBUG "LD A, (BC)");
    return 0;
}

OPCODE(LD_A_DE)
{
    uint8_t new_val;

    if (bus_read(&new_val, regs->de))
    {
        return -1;
    }
    regs->a = new_val;
    log(LDEBUG "LD A, (DE)");
    return 0;
}

OPCODE(LD_A_HL)
{
    uint8_t new_val;

    if (bus_read(&new_val, regs->hl))
    {
        return -1;
    }
    regs->a = new_val;
    log(LDEBUG "LD A, (HL)");
    return 0;
}

OPCODE(LD_B_HL)
{
    uint8_t new_val;

    if (bus_read(&new_val, regs->hl))
    {
        return -1;
    }
    regs->b = new_val;
    log(LDEBUG "LD B, (HL)");
    return 0;
}

OPCODE(LD_C_HL)
{
    uint8_t new_val;

    if (bus_read(&new_val, regs->hl))
    {
        return -1;
    }
    regs->c = new_val;
    log(LDEBUG "LD C, (HL)");
    return 0;
}

OPCODE(LD_D_HL)
{
    uint8_t new_val;

    if (bus_read(&new_val, regs->hl))
    {
        return -1;
    }
    regs->d = new_val;
    log(LDEBUG "LD D, (HL)");
    return 0;
}

OPCODE(LD_E_HL)
{
    uint8_t new_val;

    if (bus_read(&new_val, regs->hl))
    {
        return -1;
    }
    regs->e = new_val;
    log(LDEBUG "LD E, (HL)");
    return 0;
}

OPCODE(LD_H_HL)
{
    uint8_t new_val;

    if (bus_read(&new_val, regs->hl))
    {
        return -1;
    }
    regs->h = new_val;
    log(LDEBUG "LD H, (HL)");
    return 0;
}

OPCODE(LD_L_HL)
{
    uint8_t new_val;

    if (bus_read(&new_val, regs->hl))
    {
        return -1;
    }
    regs->l = new_val;
    log(LDEBUG "LD L, (HL)");
    return 0;
}

// LD (reg16), A

OPCODE(LD_BC_A)
{
    if (bus_write(regs->a, regs->bc))
    {
        return -1;
    }
    log(LDEBUG "LD (BC), A");
    return 0;
}

OPCODE(LD_DE_A)
{
    if (bus_write(regs->a, regs->de))
    {
        return -1;
    }
    log(LDEBUG "LD (DE), A");
    return 0;
}

OPCODE(LD_HL_A)
{
    if (bus_write(regs->a, regs->hl))
    {
        return -1;
    }
    log(LDEBUG "LD (HL), A");
    return 0;
}

OPCODE(LD_HL_B)
{
    if (bus_write(regs->b, regs->hl))
    {
        return -1;
    }
    log(LDEBUG "LD (HL), B");
    return 0;
}

OPCODE(LD_HL_C)
{
    if (bus_write(regs->c, regs->hl))
    {
        return -1;
    }
    log(LDEBUG "LD (HL), C");
    return 0;
}

OPCODE(LD_HL_D)
{
    if (bus_write(regs->d, regs->hl))
    {
        return -1;
    }
    log(LDEBUG "LD (HL), D");
    return 0;
}

OPCODE(LD_HL_E)
{
    if (bus_write(regs->e, regs->hl))
    {
        return -1;
    }
    log(LDEBUG "LD (HL), E");
    return 0;
}

OPCODE(LD_HL_H)
{
    if (bus_write(regs->h, regs->hl))
    {
        return -1;
    }
    log(LDEBUG "LD (HL), H");
    return 0;
}

OPCODE(LD_HL_L)
{
    if (bus_write(regs->l, regs->hl))
    {
        return -1;
    }
    log(LDEBUG "LD (HL), L");
    return 0;
}

// LD (reg16), imm8

OPCODE(LD_HL_n)
{
    uint8_t imm8;

    if (bus_read(&imm8, regs->pc + 1))
    {
        return -1;
    }

    if (bus_write(imm8, regs->hl))
    {
        return -1;
    }
    log(LDEBUG "LD (HL), 0x%02x", imm8);
    return 0;
}

// LD reg8, (imm16)

OPCODE(LD_A_nn)
{
    uint16_t imm16;
    uint8_t val;

    if (read_word(&imm16, regs->pc + 1))
    {
        return -1;
    }

    if (bus_read(&val, imm16))
    {
        return -1;
    }
    regs->a = val;
    log(LDEBUG "LD A, (0x%02x)", imm16);
    return 0;
}

// LD (imm16), reg8

OPCODE(LD_nn_A)
{
    uint16_t imm16;

    if (read_word(&imm16, regs->pc + 1))
    {
        return -1;
    }

    if (bus_write(regs->a, imm16))
    {
        return -1;
    }

    log(LDEBUG "LD (0x%02x), A", imm16);
    return 0;
}

// LD A, (C)

OPCODE(LD_A_C2)
{
    uint8_t val;

    if (bus_read(&val, 0xFF00 + regs->c))
    {
        return -1;
    }
    regs->a = val;

    log(LDEBUG "LD A, (C)");
    return 0;
}

// LD (C), A

OPCODE(LD_C_A2)
{
    if (bus_write(regs->a, 0xFF00 + regs->c))
    {
        return -1;
    }

    log(LDEBUG "LD (C), A");
    return 0;
}

// LDD A, (HL)

OPCODE(LDD_A_HL)
{
    uint8_t val;

    if (bus_read(&val, regs->hl))
    {
        return -1;
    }
    regs->a = val;
    regs->hl--;

    log(LDEBUG "LDD A, (HL)");
    return 0;
}

// LDD (HL), A

OPCODE(LDD_HL_A)
{
    if (bus_write(regs->a, regs->hl))
    {
        return -1;
    }
    regs->hl--;

    log(LDEBUG "LDD (HL), A");
    return 0;
}

// LDI A, (HL)

OPCODE(LDI_A_HL)
{
    uint8_t val;

    if (bus_read(&val, regs->hl))
    {
        return -1;
    }
    regs->a = val;
    regs->hl++;

    log(LDEBUG "LDI A, (HL)");
    return 0;
}

// LDI (HL), A

OPCODE(LDI_HL_A)
{
    if (bus_write(regs->a, regs->hl))
    {
        return -1;
    }
    regs->hl++;

    log(LDEBUG "LDI (HL), A");
    return 0;
}

// LDH (n), A

OPCODE(LDH_n_A)
{
    uint8_t imm8;

    if (bus_read(&imm8, regs->pc + 1))
    {
        return -1;
    }
    if (bus_write(regs->a, 0xFF00 + imm8))
    {
        return -1;
    }

    log(LDEBUG "LDH (0x%02x), A", imm8);
    return 0;
}

// LDH A, (n)

OPCODE(LDH_A_n)
{
    uint8_t imm8, val;

    if (bus_read(&imm8, regs->pc + 1))
    {
        return -1;
    }
    if (bus_read(&val, 0xFF00 + imm8))
    {
        return -1;
    }
    regs->a = val;

    log(LDEBUG "LDH A, (0x%02x)", imm8);
    return 0;
}

/* -------- 16-Bit Loads ------- */

// LD reg16, imm16

OPCODE(LD_BC_nn)
{
    uint16_t imm16;

    if (read_word(&imm16, regs->pc + 1))
    {
        return -1;
    }
    regs->bc = imm16;

    log(LDEBUG "LD BC, 0x%04x", imm16);
    return 0;
}

OPCODE(LD_DE_nn)
{
    uint16_t imm16;

    if (read_word(&imm16, regs->pc + 1))
    {
        return -1;
    }
    regs->de = imm16;

    log(LDEBUG "LD DE, 0x%04x", imm16);
    return 0;
}

OPCODE(LD_HL_nn)
{
    uint16_t imm16;

    if (read_word(&imm16, regs->pc + 1))
    {
        return -1;
    }
    regs->hl = imm16;

    log(LDEBUG "LD HL, 0x%04x", imm16);
    return 0;
}

OPCODE(LD_SP_nn)
{
    uint16_t imm16;

    if (read_word(&imm16, regs->pc + 1))
    {
        return -1;
    }
    regs->sp = imm16;

    log(LDEBUG "LD SP, 0x%04x", imm16);
    return 0;
}

// LD reg16, reg16

OPCODE(LD_SP_HL)
{
    regs->sp = regs->hl;

    log(LDEBUG "LD SP, HL");
    return 0;
}

// LDHL SP, n

OPCODE(LDHL_SP_n)
{
    uint8_t imm8;

    if (bus_read(&imm8, regs->pc + 1))
    {
        return -1;
    }


    regs->f.z = 0;
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->sp & 0xFF, imm8);
    regs->f.c = CARRY(regs->sp & 0xFF, imm8);

    regs->hl = regs->sp + imm8;
    log(LDEBUG "LDHL SP, 0x%02x", imm8);
    return 0;
}

// LD (nn), SP

OPCODE(LD_nn_SP)
{
    uint16_t imm16;

    if (read_word(&imm16, regs->pc + 1))
    {
        return -1;
    }

    if (write_word(regs->sp, imm16))
    {
        return -1;
    }

    log(LDEBUG "LD (0x%04x), SP", imm16);
    return 0;
}

// PUSH reg16

OPCODE(PUSH_AF)
{
    regs->sp -= 2;

    if (write_word(regs->af, regs->sp))
    {
        return -1;
    }

    log(LDEBUG "PUSH AF");
    return 0;
}

OPCODE(PUSH_BC)
{
    regs->sp -= 2;

    if (write_word(regs->bc, regs->sp))
    {
        return -1;
    }

    log(LDEBUG "PUSH BC");
    return 0;
}

OPCODE(PUSH_DE)
{
    regs->sp -= 2;

    if (write_word(regs->de, regs->sp))
    {
        return -1;
    }

    log(LDEBUG "PUSH DE");
    return 0;
}

OPCODE(PUSH_HL)
{
    regs->sp -= 2;

    if (write_word(regs->hl, regs->sp))
    {
        return -1;
    }

    log(LDEBUG "PUSH HL");
    return 0;
}

// POP reg16

OPCODE(POP_AF)
{
    if (read_word(&regs->af, regs->sp))
    {
        return -1;
    }
    regs->sp += 2;

    log(LDEBUG "POP AF");
    return 0;
}

OPCODE(POP_BC)
{
    if (read_word(&regs->bc, regs->sp))
    {
        return -1;
    }
    regs->sp += 2;

    log(LDEBUG "POP BC");
    return 0;
}

OPCODE(POP_DE)
{
    if (read_word(&regs->de, regs->sp))
    {
        return -1;
    }
    regs->sp += 2;

    log(LDEBUG "POP DE");
    return 0;
}

OPCODE(POP_HL)
{
    if (read_word(&regs->hl, regs->sp))
    {
        return -1;
    }
    regs->sp += 2;

    log(LDEBUG "POP HL");
    return 0;
}

/* ---------- 8-Bit ALU -------- */

// ADD A, reg8

OPCODE(ADD_A_A)
{
    regs->f.z = ZERO(regs->a, regs->a);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, regs->a);
    regs->f.c = CARRY(regs->a, regs->a);

    regs->a += regs->a;
    log(LDEBUG "ADD A, A");
    return 0;
}

OPCODE(ADD_A_B)
{
    regs->f.z = ZERO(regs->a, regs->b);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, regs->b);
    regs->f.c = CARRY(regs->a, regs->b);

    regs->a += regs->b;
    log(LDEBUG "ADD A, B");
    return 0;
}

OPCODE(ADD_A_C)
{
    regs->f.z = ZERO(regs->a, regs->c);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, regs->c);
    regs->f.c = CARRY(regs->a, regs->c);

    regs->a += regs->c;
    log(LDEBUG "ADD A, C");
    return 0;
}

OPCODE(ADD_A_D)
{
    regs->f.z = ZERO(regs->a, regs->d);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, regs->d);
    regs->f.c = CARRY(regs->a, regs->d);

    regs->a += regs->d;
    log(LDEBUG "ADD A, D");
    return 0;
}

OPCODE(ADD_A_E)
{
    regs->f.z = ZERO(regs->a, regs->e);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, regs->e);
    regs->f.c = CARRY(regs->a, regs->e);

    regs->a += regs->e;
    log(LDEBUG "ADD A, E");
    return 0;
}

OPCODE(ADD_A_H)
{
    regs->f.z = ZERO(regs->a, regs->h);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, regs->h);
    regs->f.c = CARRY(regs->a, regs->h);

    regs->a += regs->h;
    log(LDEBUG "ADD A, H");
    return 0;
}

OPCODE(ADD_A_L)
{
    regs->f.z = ZERO(regs->a, regs->l);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, regs->l);
    regs->f.c = CARRY(regs->a, regs->l);

    regs->a += regs->l;
    log(LDEBUG "ADD A, L");
    return 0;
}

// ADD A, (HL)

OPCODE(ADD_A_HL)
{
    uint8_t val;

    if (bus_read(&val, regs->hl))
    {
        return -1;
    }

    regs->f.z = ZERO(regs->a, val);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, val);
    regs->f.c = CARRY(regs->a, val);

    regs->a += val;
    log(LDEBUG "ADD A, (HL)");
    return 0;
}

// ADD A, imm8

OPCODE(ADD_A_n)
{
    uint8_t imm8;

    if (bus_read(&imm8, regs->pc + 1))
    {
        return -1;
    }

    regs->f.z = ZERO(regs->a, imm8);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, imm8);
    regs->f.c = CARRY(regs->a, imm8);

    regs->a += imm8;
    log(LDEBUG "ADD A, 0x%02x", imm8);
    return 0;
}

// ADC A, reg8

OPCODE(ADC_A_A)
{
    regs->f.z = ZERO(regs->a, regs->a + regs->f.c);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, regs->a + regs->f.c);
    regs->f.c = CARRY(regs->a, regs->a + regs->f.c);

    regs->a += regs->a + regs->f.c;
    log(LDEBUG "ADC A, A");
    return 0;
}

OPCODE(ADC_A_B)
{
    regs->f.z = ZERO(regs->a, regs->b + regs->f.c);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, regs->b + regs->f.c);
    regs->f.c = CARRY(regs->a, regs->b + regs->f.c);

    regs->a += regs->b + regs->f.c;
    log(LDEBUG "ADC A, B");
    return 0;
}

OPCODE(ADC_A_C)
{
    regs->f.z = ZERO(regs->a, regs->c + regs->f.c);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, regs->c + regs->f.c);
    regs->f.c = CARRY(regs->a, regs->c + regs->f.c);

    regs->a += regs->c + regs->f.c;
    log(LDEBUG "ADC A, C");
    return 0;
}

OPCODE(ADC_A_D)
{
    regs->f.z = ZERO(regs->a, regs->d + regs->f.c);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, regs->d + regs->f.c);
    regs->f.c = CARRY(regs->a, regs->d + regs->f.c);

    regs->a += regs->d + regs->f.c;
    log(LDEBUG "ADC A, D");
    return 0;
}

OPCODE(ADC_A_E)
{
    regs->f.z = ZERO(regs->a, regs->e + regs->f.c);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, regs->e + regs->f.c);
    regs->f.c = CARRY(regs->a, regs->e + regs->f.c);

    regs->a += regs->e + regs->f.c;
    log(LDEBUG "ADC A, E");
    return 0;
}

OPCODE(ADC_A_H)
{
    regs->f.z = ZERO(regs->a, regs->h + regs->f.c);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, regs->h + regs->f.c);
    regs->f.c = CARRY(regs->a, regs->h + regs->f.c);

    regs->a += regs->h + regs->f.c;
    log(LDEBUG "ADC A, H");
    return 0;
}

OPCODE(ADC_A_L)
{
    regs->f.z = ZERO(regs->a, regs->l + regs->f.c);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, regs->l + regs->f.c);
    regs->f.c = CARRY(regs->a, regs->l + regs->f.c);

    regs->a += regs->l;
    log(LDEBUG "ADC A, L");
    return 0;
}

// ADC A, (HL)

OPCODE(ADC_A_HL)
{
    uint8_t val;

    if (bus_read(&val, regs->hl))
    {
        return -1;
    }

    regs->f.z = ZERO(regs->a, val + regs->f.c);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, val + regs->f.c);
    regs->f.c = CARRY(regs->a, val + regs->f.c);

    regs->a += val + regs->f.c;
    log(LDEBUG "ADC A, (HL)");
    return 0;
}

// ADC A, imm8

OPCODE(ADC_A_n)
{
    uint8_t imm8;

    if (bus_read(&imm8, regs->pc + 1))
    {
        return -1;
    }

    regs->f.z = ZERO(regs->a, imm8 + regs->f.c);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, imm8 + regs->f.c);
    regs->f.c = CARRY(regs->a, imm8 + regs->f.c);

    regs->a += imm8 + regs->f.c;
    log(LDEBUG "ADC A, 0x%02x", imm8);
    return 0;
}

// SUB A, reg8

OPCODE(SUB_A_A)
{
    regs->f.z = SUB_ZERO(regs->a, regs->a);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->a, regs->a);
    regs->f.c = SUB_CARRY(regs->a, regs->a);

    regs->a -= regs->a;
    log(LDEBUG "SUB A, A");
    return 0;
}

OPCODE(SUB_A_B)
{
    regs->f.z = SUB_ZERO(regs->a, regs->b);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->a, regs->b);
    regs->f.c = SUB_CARRY(regs->a, regs->b);

    regs->a -= regs->b;
    log(LDEBUG "SUB A, B");
    return 0;
}

OPCODE(SUB_A_C)
{
    regs->f.z = SUB_ZERO(regs->a, regs->c);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->a, regs->c);
    regs->f.c = SUB_CARRY(regs->a, regs->c);

    regs->a -= regs->c;
    log(LDEBUG "SUB A, C");
    return 0;
}

OPCODE(SUB_A_D)
{
    regs->f.z = SUB_ZERO(regs->a, regs->d);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->a, regs->d);
    regs->f.c = SUB_CARRY(regs->a, regs->d);

    regs->a -= regs->d;
    log(LDEBUG "SUB A, D");
    return 0;
}

OPCODE(SUB_A_E)
{
    regs->f.z = SUB_ZERO(regs->a, regs->e);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->a, regs->e);
    regs->f.c = SUB_CARRY(regs->a, regs->e);

    regs->a -= regs->e;
    log(LDEBUG "SUB A, E");
    return 0;
}

OPCODE(SUB_A_H)
{
    regs->f.z = SUB_ZERO(regs->a, regs->h);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->a, regs->h);
    regs->f.c = SUB_CARRY(regs->a, regs->h);

    regs->a -= regs->h;
    log(LDEBUG "SUB A, H");
    return 0;
}

OPCODE(SUB_A_L)
{
    regs->f.z = SUB_ZERO(regs->a, regs->l);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->a, regs->l);
    regs->f.c = SUB_CARRY(regs->a, regs->l);

    regs->a -= regs->l;
    log(LDEBUG "SUB A, L");
    return 0;
}

// SUB A, (HL)

OPCODE(SUB_A_HL)
{
    uint8_t val;

    if (bus_read(&val, regs->hl))
    {
        return -1;
    }

    regs->f.z = SUB_ZERO(regs->a, val);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->a, val);
    regs->f.c = SUB_CARRY(regs->a, val);

    regs->a -= val;
    log(LDEBUG "SUB A, (HL)");
    return 0;
}

// SUB A, imm8

OPCODE(SUB_A_n)
{
    uint8_t imm8;

    if (bus_read(&imm8, regs->pc + 1))
    {
        return -1;
    }

    regs->f.z = SUB_ZERO(regs->a, imm8);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->a, imm8);
    regs->f.c = SUB_CARRY(regs->a, imm8);

    regs->a -= imm8;
    log(LDEBUG "SUB A, 0x%02x", imm8);
    return 0;
}

// SBC A, reg8

OPCODE(SBC_A_A)
{
    regs->f.z = SUB_ZERO(regs->a, regs->a + regs->f.c);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->a, regs->a + regs->f.c);
    regs->f.c = SUB_CARRY(regs->a, regs->a + regs->f.c);

    regs->a -= regs->a + regs->f.c;
    log(LDEBUG "SBC A, A");
    return 0;
}

OPCODE(SBC_A_B)
{
    regs->f.z = SUB_ZERO(regs->a, regs->b + regs->f.c);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->a, regs->b + regs->f.c);
    regs->f.c = SUB_CARRY(regs->a, regs->b + regs->f.c);

    regs->a -= regs->b + regs->f.c;
    log(LDEBUG "SBC A, B");
    return 0;
}

OPCODE(SBC_A_C)
{
    regs->f.z = SUB_ZERO(regs->a, regs->c + regs->f.c);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->a, regs->c + regs->f.c);
    regs->f.c = SUB_CARRY(regs->a, regs->c + regs->f.c);

    regs->a -= regs->c + regs->f.c;
    log(LDEBUG "SBC A, C");
    return 0;
}

OPCODE(SBC_A_D)
{
    regs->f.z = SUB_ZERO(regs->a, regs->d + regs->f.c);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->a, regs->d + regs->f.c);
    regs->f.c = SUB_CARRY(regs->a, regs->d + regs->f.c);

    regs->a -= regs->d + regs->f.c;
    log(LDEBUG "SBC A, D");
    return 0;
}

OPCODE(SBC_A_E)
{
    regs->f.z = SUB_ZERO(regs->a, regs->e + regs->f.c);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->a, regs->e + regs->f.c);
    regs->f.c = SUB_CARRY(regs->a, regs->e + regs->f.c);

    regs->a -= regs->e + regs->f.c;
    log(LDEBUG "SBC A, E");
    return 0;
}

OPCODE(SBC_A_H)
{
    regs->f.z = SUB_ZERO(regs->a, regs->h + regs->f.c);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->a, regs->h + regs->f.c);
    regs->f.c = SUB_CARRY(regs->a, regs->h + regs->f.c);

    regs->a -= regs->h + regs->f.c;
    log(LDEBUG "SBC A, H");
    return 0;
}

OPCODE(SBC_A_L)
{
    regs->f.z = SUB_ZERO(regs->a, regs->l + regs->f.c);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->a, regs->l + regs->f.c);
    regs->f.c = SUB_CARRY(regs->a, regs->l + regs->f.c);

    regs->a -= regs->l + regs->f.c;
    log(LDEBUG "SBC A, L");
    return 0;
}

// SBC A, (HL)

OPCODE(SBC_A_HL)
{
    uint8_t val;

    if (bus_read(&val, regs->hl))
    {
        return -1;
    }

    regs->f.z = SUB_ZERO(regs->a, val + regs->f.c);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->a, val + regs->f.c);
    regs->f.c = SUB_CARRY(regs->a, val + regs->f.c);

    regs->a -= val + regs->f.c;
    log(LDEBUG "SBC A, (HL)");
    return 0;
}

// SBC A, imm8

OPCODE(SBC_A_n)
{
    uint8_t imm8;

    if (bus_read(&imm8, regs->pc + 1))
    {
        return -1;
    }

    regs->f.z = SUB_ZERO(regs->a, imm8 + regs->f.c);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->a, imm8 + regs->f.c);
    regs->f.c = SUB_CARRY(regs->a, imm8 + regs->f.c);

    regs->a -= imm8 + regs->f.c;
    log(LDEBUG "SBC A, 0x%02x", imm8);
    return 0;
}

// AND A, reg8

OPCODE(AND_A_A)
{
    regs->a &= regs->a;

    regs->f.z = regs->a == 0 ? 1 : 0;
    regs->f.n = 0;
    regs->f.h = 1;
    regs->f.c = 0;
    log(LDEBUG "AND A, A");
    return 0;
}

OPCODE(AND_A_B)
{
    regs->a &= regs->b;
    
    regs->f.z = regs->a == 0 ? 1 : 0;
    regs->f.n = 0;
    regs->f.h = 1;
    regs->f.c = 0;
    log(LDEBUG "AND A, B");
    return 0;
}

OPCODE(AND_A_C)
{
    regs->a &= regs->c;
    
    regs->f.z = regs->a == 0 ? 1 : 0;
    regs->f.n = 0;
    regs->f.h = 1;
    regs->f.c = 0;
    log(LDEBUG "AND A, C");
    return 0;
}

OPCODE(AND_A_D)
{
    regs->a &= regs->d;
    
    regs->f.z = regs->a == 0 ? 1 : 0;
    regs->f.n = 0;
    regs->f.h = 1;
    regs->f.c = 0;
    log(LDEBUG "AND A, D");
    return 0;
}

OPCODE(AND_A_E)
{
    regs->a &= regs->e;
    
    regs->f.z = regs->a == 0 ? 1 : 0;
    regs->f.n = 0;
    regs->f.h = 1;
    regs->f.c = 0;
    log(LDEBUG "AND A, E");
    return 0;
}

OPCODE(AND_A_H)
{
    regs->a &= regs->a;
    
    regs->f.z = regs->a == 0 ? 1 : 0;
    regs->f.n = 0;
    regs->f.h = 1;
    regs->f.c = 0;
    log(LDEBUG "AND A, H");
    return 0;
}

OPCODE(AND_A_L)
{
    regs->a &= regs->l;
    
    regs->f.z = regs->a == 0 ? 1 : 0;
    regs->f.n = 0;
    regs->f.h = 1;
    regs->f.c = 0;
    log(LDEBUG "AND A, L");
    return 0;
}

// AND A, (HL)

OPCODE(AND_A_HL)
{
    uint8_t val;

    if (bus_read(&val, regs->hl))
    {
        return -1;
    }

    regs->a &= val;
    
    regs->f.z = regs->a == 0 ? 1 : 0;
    regs->f.n = 0;
    regs->f.h = 1;
    regs->f.c = 0;
    log(LDEBUG "AND A, (HL)");
    return 0;
}

// AND A, imm8

OPCODE(AND_A_n)
{
    uint8_t imm8;

    if (bus_read(&imm8, regs->pc + 1))
    {
        return -1;
    }

    regs->a &= imm8;
    
    regs->f.z = regs->a == 0 ? 1 : 0;
    regs->f.n = 0;
    regs->f.h = 1;
    regs->f.c = 0;
    log(LDEBUG "AND A, 0x%02x", imm8);
    return 0;
}

// OR A, reg8

OPCODE(OR_A_A)
{
    regs->a |= regs->a;

    regs->f.z = regs->a == 0 ? 1 : 0;
    regs->f.n = 0;
    regs->f.h = 0;
    regs->f.c = 0;
    log(LDEBUG "OR A, A");
    return 0;
}

OPCODE(OR_A_B)
{
    regs->a |= regs->b;
    
    regs->f.z = regs->a == 0 ? 1 : 0;
    regs->f.n = 0;
    regs->f.h = 0;
    regs->f.c = 0;
    log(LDEBUG "OR A, B");
    return 0;
}

OPCODE(OR_A_C)
{
    regs->a |= regs->c;
    
    regs->f.z = regs->a == 0 ? 1 : 0;
    regs->f.n = 0;
    regs->f.h = 0;
    regs->f.c = 0;
    log(LDEBUG "OR A, C");
    return 0;
}

OPCODE(OR_A_D)
{
    regs->a |= regs->d;
    
    regs->f.z = regs->a == 0 ? 1 : 0;
    regs->f.n = 0;
    regs->f.h = 0;
    regs->f.c = 0;
    log(LDEBUG "OR A, D");
    return 0;
}

OPCODE(OR_A_E)
{
    regs->a |= regs->e;
    
    regs->f.z = regs->a == 0 ? 1 : 0;
    regs->f.n = 0;
    regs->f.h = 0;
    regs->f.c = 0;
    log(LDEBUG "OR A, E");
    return 0;
}

OPCODE(OR_A_H)
{
    regs->a |= regs->a;
    
    regs->f.z = regs->a == 0 ? 1 : 0;
    regs->f.n = 0;
    regs->f.h = 0;
    regs->f.c = 0;
    log(LDEBUG "OR A, H");
    return 0;
}

OPCODE(OR_A_L)
{
    regs->a |= regs->l;
    
    regs->f.z = regs->a == 0 ? 1 : 0;
    regs->f.n = 0;
    regs->f.h = 0;
    regs->f.c = 0;
    log(LDEBUG "OR A, L");
    return 0;
}

// OR A, (HL)

OPCODE(OR_A_HL)
{
    uint8_t val;

    if (bus_read(&val, regs->hl))
    {
        return -1;
    }

    regs->a |= val;
    
    regs->f.z = regs->a == 0 ? 1 : 0;
    regs->f.n = 0;
    regs->f.h = 0;
    regs->f.c = 0;
    log(LDEBUG "OR A, (HL)");
    return 0;
}

// OR A, imm8

OPCODE(OR_A_n)
{
    uint8_t imm8;

    if (bus_read(&imm8, regs->pc + 1))
    {
        return -1;
    }

    regs->a |= imm8;
    
    regs->f.z = regs->a == 0 ? 1 : 0;
    regs->f.n = 0;
    regs->f.h = 0;
    regs->f.c = 0;
    log(LDEBUG "OR A, 0x%02x", imm8);
    return 0;
}

// OR A, reg8

OPCODE(XOR_A_A)
{
    regs->a ^= regs->a;

    regs->f.z = regs->a == 0 ? 1 : 0;
    regs->f.n = 0;
    regs->f.h = 0;
    regs->f.c = 0;
    log(LDEBUG "XOR A, A");
    return 0;
}

OPCODE(XOR_A_B)
{
    regs->a ^= regs->b;
    
    regs->f.z = regs->a == 0 ? 1 : 0;
    regs->f.n = 0;
    regs->f.h = 0;
    regs->f.c = 0;
    log(LDEBUG "XOR A, B");
    return 0;
}

OPCODE(XOR_A_C)
{
    regs->a ^= regs->c;
    
    regs->f.z = regs->a == 0 ? 1 : 0;
    regs->f.n = 0;
    regs->f.h = 0;
    regs->f.c = 0;
    log(LDEBUG "XOR A, C");
    return 0;
}

OPCODE(XOR_A_D)
{
    regs->a ^= regs->d;
    
    regs->f.z = regs->a == 0 ? 1 : 0;
    regs->f.n = 0;
    regs->f.h = 0;
    regs->f.c = 0;
    log(LDEBUG "XOR A, D");
    return 0;
}

OPCODE(XOR_A_E)
{
    regs->a ^= regs->e;
    
    regs->f.z = regs->a == 0 ? 1 : 0;
    regs->f.n = 0;
    regs->f.h = 0;
    regs->f.c = 0;
    log(LDEBUG "XOR A, E");
    return 0;
}

OPCODE(XOR_A_H)
{
    regs->a ^= regs->a;
    
    regs->f.z = regs->a == 0 ? 1 : 0;
    regs->f.n = 0;
    regs->f.h = 0;
    regs->f.c = 0;
    log(LDEBUG "XOR A, H");
    return 0;
}

OPCODE(XOR_A_L)
{
    regs->a ^= regs->l;
    
    regs->f.z = regs->a == 0 ? 1 : 0;
    regs->f.n = 0;
    regs->f.h = 0;
    regs->f.c = 0;
    log(LDEBUG "XOR A, L");
    return 0;
}

// XOR A, (HL)

OPCODE(XOR_A_HL)
{
    uint8_t val;

    if (bus_read(&val, regs->hl))
    {
        return -1;
    }

    regs->a ^= val;
    
    regs->f.z = regs->a == 0 ? 1 : 0;
    regs->f.n = 0;
    regs->f.h = 0;
    regs->f.c = 0;
    log(LDEBUG "XOR A, (HL)");
    return 0;
}

// XOR A, imm8

OPCODE(XOR_A_n)
{
    uint8_t imm8;

    if (bus_read(&imm8, regs->pc + 1))
    {
        return -1;
    }

    regs->a ^= imm8;
    
    regs->f.z = regs->a == 0 ? 1 : 0;
    regs->f.n = 0;
    regs->f.h = 0;
    regs->f.c = 0;
    log(LDEBUG "XOR A, 0x%02x", imm8);
    return 0;
}

// CP A, reg8

OPCODE(CP_A_A)
{
    regs->f.z = SUB_ZERO(regs->a, regs->a);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->a, regs->a);
    regs->f.c = SUB_CARRY(regs->a, regs->a);

    log(LDEBUG "CP A, A");
    return 0;
}

OPCODE(CP_A_B)
{
    regs->f.z = SUB_ZERO(regs->a, regs->b);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->a, regs->b);
    regs->f.c = SUB_CARRY(regs->a, regs->b);

    log(LDEBUG "CP A, B");
    return 0;
}

OPCODE(CP_A_C)
{
    regs->f.z = SUB_ZERO(regs->a, regs->c);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->a, regs->c);
    regs->f.c = SUB_CARRY(regs->a, regs->c);

    log(LDEBUG "CP A, C");
    return 0;
}

OPCODE(CP_A_D)
{
    regs->f.z = SUB_ZERO(regs->a, regs->d);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->a, regs->d);
    regs->f.c = SUB_CARRY(regs->a, regs->d);

    log(LDEBUG "CP A, D");
    return 0;
}

OPCODE(CP_A_E)
{
    regs->f.z = SUB_ZERO(regs->a, regs->e);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->a, regs->e);
    regs->f.c = SUB_CARRY(regs->a, regs->e);

    log(LDEBUG "CP A, E");
    return 0;
}

OPCODE(CP_A_H)
{
    regs->f.z = SUB_ZERO(regs->a, regs->h);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->a, regs->h);
    regs->f.c = SUB_CARRY(regs->a, regs->h);

    log(LDEBUG "CP A, H");
    return 0;
}

OPCODE(CP_A_L)
{
    regs->f.z = SUB_ZERO(regs->a, regs->l);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->a, regs->l);
    regs->f.c = SUB_CARRY(regs->a, regs->l);

    log(LDEBUG "CP A, L");
    return 0;
}

// CP A, (HL)

OPCODE(CP_A_HL)
{
    uint8_t val;

    if (bus_read(&val, regs->hl))
    {
        return -1;
    }

    regs->f.z = SUB_ZERO(regs->a, val);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->a, val);
    regs->f.c = SUB_CARRY(regs->a, val);

    log(LDEBUG "CP A, (HL)");
    return 0;
}

// CP A, imm8

OPCODE(CP_A_n)
{
    uint8_t imm8;

    if (bus_read(&imm8, regs->pc + 1))
    {
        return -1;
    }

    regs->f.z = SUB_ZERO(regs->a, imm8);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->a, imm8);
    regs->f.c = SUB_CARRY(regs->a, imm8);

    log(LDEBUG "CP A, 0x%02x", imm8);
    return 0;
}

// INC reg8

OPCODE(INC_A)
{
    regs->f.z = ZERO(regs->a, 1);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, 1);

    regs->a++;
    log(LDEBUG "INC A");
    return 0;
}

OPCODE(INC_B)
{
    regs->f.z = ZERO(regs->b, 1);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->b, 1);

    regs->b++;
    log(LDEBUG "INC B");
    return 0;
}

OPCODE(INC_C)
{
    regs->f.z = ZERO(regs->c, 1);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->c, 1);

    regs->c++;
    log(LDEBUG "INC C");
    return 0;
}

OPCODE(INC_D)
{
    regs->f.z = ZERO(regs->d, 1);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->d, 1);

    regs->d++;
    log(LDEBUG "INC D");
    return 0;
}

OPCODE(INC_E)
{
    regs->f.z = ZERO(regs->e, 1);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->e, 1);

    regs->e++;
    log(LDEBUG "INC E");
    return 0;
}

OPCODE(INC_H)
{
    regs->f.z = ZERO(regs->h, 1);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->h, 1);

    regs->h++;
    log(LDEBUG "INC H");
    return 0;
}

OPCODE(INC_L)
{
    regs->f.z = ZERO(regs->l, 1);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->l, 1);

    regs->l++;
    log(LDEBUG "INC L");
    return 0;
}

// INC (HL)

OPCODE(INC_HL)
{
    uint8_t val;

    if (bus_read(&val, regs->hl))
    {
        return -1;
    }

    regs->f.z = ZERO(val, 1);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(val, 1);

    val++;

    if (bus_write(val, regs->hl))
    {
        return -1;
    }
    log(LDEBUG "INC (HL)");
    return 0;
}

// DEC reg8

OPCODE(DEC_A)
{
    regs->f.z = SUB_ZERO(regs->a, 1);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->a, 1);

    regs->a--;
    log(LDEBUG "DEC A");
    return 0;
}

OPCODE(DEC_B)
{
    regs->f.z = SUB_ZERO(regs->b, 1);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->b, 1);

    regs->b--;
    log(LDEBUG "DEC B");
    return 0;
}

OPCODE(DEC_C)
{
    regs->f.z = SUB_ZERO(regs->c, 1);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->c, 1);

    regs->c--;
    log(LDEBUG "DEC C");
    return 0;
}

OPCODE(DEC_D)
{
    regs->f.z = SUB_ZERO(regs->d, 1);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->d, 1);

    regs->d--;
    log(LDEBUG "DEC D");
    return 0;
}

OPCODE(DEC_E)
{
    regs->f.z = SUB_ZERO(regs->e, 1);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->e, 1);

    regs->e--;
    log(LDEBUG "DEC E");
    return 0;
}

OPCODE(DEC_H)
{
    regs->f.z = SUB_ZERO(regs->h, 1);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->h, 1);

    regs->h--;
    log(LDEBUG "DEC H");
    return 0;
}

OPCODE(DEC_L)
{
    regs->f.z = SUB_ZERO(regs->l, 1);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(regs->l, 1);

    regs->l--;
    log(LDEBUG "DEC L");
    return 0;
}

// DEC (HL)

OPCODE(DEC_HL)
{
    uint8_t val;

    if (bus_read(&val, regs->hl))
    {
        return -1;
    }

    regs->f.z = SUB_ZERO(val, 1);
    regs->f.n = 1;
    regs->f.h = SUB_HALF_CARRY(val, 1);

    val--;

    if (bus_write(val, regs->hl))
    {
        return -1;
    }
    log(LDEBUG "DEC (HL)");
    return 0;
}

// ADD HL, reg16

OPCODE(ADD_HL_BC)
{
    regs->f.n = 0;
    regs->f.h = (((regs->hl & 0xFFF) + (regs->bc & 0xFFF)) & 0x1000) ? 1 : 0;
    regs->f.c = CARRY(regs->hl, regs->bc);

    regs->hl += regs->bc;
    log(LDEBUG "ADD HL, BC");
    return 0;
}

OPCODE(ADD_HL_DE)
{
    regs->f.n = 0;
    regs->f.h = (((regs->hl & 0xFFF) + (regs->de & 0xFFF)) & 0x1000) ? 1 : 0;
    regs->f.c = CARRY(regs->hl, regs->de);

    regs->hl += regs->de;
    log(LDEBUG "ADD HL, DE");
    return 0;
}

OPCODE(ADD_HL_HL)
{
    regs->f.n = 0;
    regs->f.h = (((regs->hl & 0xFFF) + (regs->hl & 0xFFF)) & 0x1000) ? 1 : 0;
    regs->f.c = CARRY(regs->hl, regs->hl);

    regs->hl += regs->hl;
    log(LDEBUG "ADD HL, HL");
    return 0;
}

OPCODE(ADD_HL_SP)
{
    regs->f.n = 0;
    regs->f.h = (((regs->hl & 0xFFF) + (regs->sp & 0xFFF)) & 0x1000) ? 1 : 0;
    regs->f.c = CARRY(regs->hl, regs->sp);

    regs->hl += regs->sp;
    log(LDEBUG "ADD HL, SP");
    return 0;
}

// ADD SP, imm8

OPCODE(ADD_SP_n)
{
    uint8_t imm8;

    if (bus_read(&imm8, regs->pc + 1))
    {
        return -1;
    }

    regs->f.z = 0;
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->sp, imm8);
    regs->f.c = CARRY(regs->sp, imm8);

    regs->sp += imm8;
    log(LDEBUG "ADD SP, 0x%02x", imm8);
    return 0;
}

// INC reg16

OPCODE(INC_BC)
{
    regs->bc++;
    log(LDEBUG "INC BC");
    return 0;
}

OPCODE(INC_DE)
{
    regs->de++;
    log(LDEBUG "INC DE");
    return 0;
}

OPCODE(INC_HL_2)
{
    regs->hl++;
    log(LDEBUG "INC HL");
    return 0;
}

OPCODE(INC_SP)
{
    regs->sp++;
    log(LDEBUG "INC SP");
    return 0;
}

// DEC reg16

OPCODE(DEC_BC)
{
    regs->bc--;
    log(LDEBUG "DEC BC");
    return 0;
}

OPCODE(DEC_DE)
{
    regs->de--;
    log(LDEBUG "DEC DE");
    return 0;
}

OPCODE(DEC_HL_2)
{
    regs->hl--;
    log(LDEBUG "DEC HL");
    return 0;
}

OPCODE(DEC_SP)
{
    regs->sp--;
    log(LDEBUG "DEC SP");
    return 0;
}

/* ------------- Jumps ---------- */

OPCODE(JP)
{
    uint16_t addr;

    if (read_word(&addr, regs->pc + 1))
    {
        return -1;
    }

    regs->pc = addr;

    log(LDEBUG "JP 0x%04x", addr);
    return 0;
}

OPCODE(JP_NZ)
{
    uint16_t addr;

#ifdef DEBUG
    if (read_word(&addr, regs->pc + 1))
    {
        return -1;
    }

    log(LDEBUG "JP NZ 0x%04x", addr);

    if (regs->f.z)
    {
        return 0;
    }
#else
    if (regs->f.z) // It's more efficient to check the condition first.
    {
        return 0;
    }

    if (read_word(&addr, regs->pc + 1))
    {
        return -1;
    }
#endif

    regs->pc = addr;
    return 0;
}

OPCODE(JP_Z)
{
    uint16_t addr;

#ifdef DEBUG
    if (read_word(&addr, regs->pc + 1))
    {
        return -1;
    }

    log(LDEBUG "JP Z 0x%04x", addr);

    if (!regs->f.z)
    {
        return 0;
    }
#else
    if (!regs->f.z) // It's more efficient to check the condition first.
    {
        return 0;
    }

    if (read_word(&addr, regs->pc + 1))
    {
        return -1;
    }
#endif

    regs->pc = addr;
    return 0;
}

OPCODE(JP_NC)
{
    uint16_t addr;

#ifdef DEBUG
    if (read_word(&addr, regs->pc + 1))
    {
        return -1;
    }

    log(LDEBUG "JP NC 0x%04x", addr);

    if (regs->f.c)
    {
        return 0;
    }
#else
    if (regs->f.c) // It's more efficient to check the condition first.
    {
        return 0;
    }

    if (read_word(&addr, regs->pc + 1))
    {
        return -1;
    }
#endif

    regs->pc = addr;
    return 0;
}

OPCODE(JP_C)
{
    uint16_t addr;

#ifdef DEBUG
    if (read_word(&addr, regs->pc + 1))
    {
        return -1;
    }

    log(LDEBUG "JP C 0x%04x", addr);

    if (!regs->f.c)
    {
        return 0;
    }
#else
    if (!regs->f.c) // It's more efficient to check the condition first.
    {
        return 0;
    }

    if (read_word(&addr, regs->pc + 1))
    {
        return -1;
    }
#endif

    regs->pc = addr;
    return 0;
}

OPCODE(JP_HL)
{
    uint16_t address;

    if (read_word(&address, regs->hl))
    {
        return -1;
    }

    regs->pc = address;
    log(LDEBUG "JP (HL)");
    return 0;
}

OPCODE(JR)
{
    uint8_t offset;

    if (bus_read(&offset, regs->pc + 1))
    {
        return -1;
    }

    regs->pc += offset;
    log(LDEBUG "JR 0x%02x", offset);
    return 0;
}

OPCODE(JR_NZ)
{
    uint8_t offset;

#ifdef DEBUG
    if (bus_read(&offset, regs->pc + 1))
    {
        return -1;
    }

    log(LDEBUG "JR NZ 0x%02x", offset);

    if (regs->f.z)
    {
        return 0;
    }
#else
    if (regs->f.z) // It's more efficient to check the condition first.
    {
        return 0;
    }

    if (bus_read(&offset, regs->pc + 1))
    {
        return -1;
    }
#endif

    regs->pc += offset;
    return 0;
}

OPCODE(JR_Z)
{
    uint8_t offset;

#ifdef DEBUG
    if (bus_read(&offset, regs->pc + 1))
    {
        return -1;
    }

    log(LDEBUG "JR Z 0x%02x", offset);

    if (!regs->f.z)
    {
        return 0;
    }
#else
    if (!regs->f.z) // It's more efficient to check the condition first.
    {
        return 0;
    }

    if (bus_read(&offset, regs->pc + 1))
    {
        return -1;
    }
#endif

    regs->pc += offset;
    return 0;
}

OPCODE(JR_NC)
{
    uint8_t offset;

#ifdef DEBUG
    if (bus_read(&offset, regs->pc + 1))
    {
        return -1;
    }

    log(LDEBUG "JR NC 0x%02x", offset);

    if (regs->f.c)
    {
        return 0;
    }
#else
    if (regs->f.c) // It's more efficient to check the condition first.
    {
        return 0;
    }

    if (bus_read(&offset, regs->pc + 1))
    {
        return -1;
    }
#endif

    regs->pc += offset;
    return 0;
}

OPCODE(JR_C)
{
    uint8_t offset;

#ifdef DEBUG
    if (bus_read(&offset, regs->pc + 1))
    {
        return -1;
    }

    log(LDEBUG "JR C 0x%02x", offset);

    if (!regs->f.c)
    {
        return 0;
    }
#else
    if (!regs->f.c) // It's more efficient to check the condition first.
    {
        return 0;
    }

    if (bus_read(&offset, regs->pc + 1))
    {
        return -1;
    }
#endif

    regs->pc += offset;
    return 0;
}

/* ------------- Calls ---------- */

OPCODE(CALL)
{
    uint16_t address;

    regs->sp -= 2;
    if (write_word(regs->pc + 3, regs->sp))
    {
        return -1;
    }

    if (read_word(&address, regs->pc + 1))
    {
        return -1;
    }
    regs->pc = address;
    log(LDEBUG "CALL 0x%04x", address);
    return 0;
}

OPCODE(CALL_NZ)
{
    uint16_t address;

#ifdef DEBUG

    if (read_word(&address, regs->pc + 1))
    {
        return -1;
    }
    log(LDEBUG "CALL NZ 0x%04x", address);

    if (regs->f.z)
    {
        return 0;
    }
#else
    if (regs->f.z)
    {
        return 0;
    }

    if (read_word(&address, regs->pc + 1))
    {
        return -1;
    }
#endif
    regs->sp -= 2;
    if (write_word(regs->pc + 3, regs->sp))
    {
        return -1;
    }

    regs->pc = address;
    return 0;
}

OPCODE(CALL_Z)
{
    uint16_t address;

#ifdef DEBUG

    if (read_word(&address, regs->pc + 1))
    {
        return -1;
    }
    log(LDEBUG "CALL Z 0x%04x", address);

    if (!regs->f.z)
    {
        return 0;
    }
#else
    if (!regs->f.z)
    {
        return 0;
    }

    if (read_word(&address, regs->pc + 1))
    {
        return -1;
    }
#endif
    regs->sp -= 2;
    if (write_word(regs->pc + 3, regs->sp))
    {
        return -1;
    }

    regs->pc = address;
    return 0;
}

OPCODE(CALL_NC)
{
    uint16_t address;

#ifdef DEBUG

    if (read_word(&address, regs->pc + 1))
    {
        return -1;
    }
    log(LDEBUG "CALL NC 0x%04x", address);

    if (regs->f.c)
    {
        return 0;
    }
#else
    if (regs->f.c)
    {
        return 0;
    }

    if (read_word(&address, regs->pc + 1))
    {
        return -1;
    }
#endif
    regs->sp -= 2;
    if (write_word(regs->pc + 3, regs->sp))
    {
        return -1;
    }

    regs->pc = address;
    return 0;
}

OPCODE(CALL_C)
{
    uint16_t address;

#ifdef DEBUG

    if (read_word(&address, regs->pc + 1))
    {
        return -1;
    }
    log(LDEBUG "CALL C 0x%04x", address);

    if (!regs->f.c)
    {
        return 0;
    }
#else
    if (!regs->f.c)
    {
        return 0;
    }

    if (read_word(&address, regs->pc + 1))
    {
        return -1;
    }
#endif
    regs->sp -= 2;
    if (write_word(regs->pc + 3, regs->sp))
    {
        return -1;
    }

    regs->pc = address;
    return 0;
}

/* ----------- Restarts --------- */

OPCODE(RST_00)
{
    regs->sp -= 2;
    if (write_word(regs->pc, regs->sp))
    {
        return -1;
    }

    regs->pc = 0x0000;
    log(LDEBUG "RST $00");
    return 0;
}

OPCODE(RST_08)
{
    regs->sp -= 2;
    if (write_word(regs->pc, regs->sp))
    {
        return -1;
    }

    regs->pc = 0x0008;
    log(LDEBUG "RST $08");
    return 0;
}

OPCODE(RST_10)
{
    regs->sp -= 2;
    if (write_word(regs->pc, regs->sp))
    {
        return -1;
    }

    regs->pc = 0x0010;
    log(LDEBUG "RST $10");
    return 0;
}

OPCODE(RST_18)
{
    regs->sp -= 2;
    if (write_word(regs->pc, regs->sp))
    {
        return -1;
    }

    regs->pc = 0x0018;
    log(LDEBUG "RST $18");
    return 0;
}

OPCODE(RST_20)
{
    regs->sp -= 2;
    if (write_word(regs->pc, regs->sp))
    {
        return -1;
    }

    regs->pc = 0x0020;
    log(LDEBUG "RST $20");
    return 0;
}

OPCODE(RST_28)
{
    regs->sp -= 2;
    if (write_word(regs->pc, regs->sp))
    {
        return -1;
    }

    regs->pc = 0x0028;
    log(LDEBUG "RST $28");
    return 0;
}

OPCODE(RST_30)
{
    regs->sp -= 2;
    if (write_word(regs->pc, regs->sp))
    {
        return -1;
    }

    regs->pc = 0x0030;
    log(LDEBUG "RST $30");
    return 0;
}

OPCODE(RST_38)
{
    regs->sp -= 2;
    if (write_word(regs->pc, regs->sp))
    {
        return -1;
    }

    regs->pc = 0x0038;
    log(LDEBUG "RST $38");
    return 0;
}

/* ------------ Returns --------- */

OPCODE(RET)
{
    uint16_t address;

    if (read_word(&address, regs->sp))
    {
        return -1;
    }
    regs->sp += 2;

    regs->pc = address;
    log(LDEBUG "RET");
    return 0;
}

OPCODE(RET_NZ)
{
    uint16_t address;
    log(LDEBUG "RET NZ");

    if (regs->f.z)
    {
        return 0;
    }

    if (read_word(&address, regs->sp))
    {
        return -1;
    }
    regs->sp += 2;

    regs->pc = address;
    return 0;
}

OPCODE(RET_Z)
{
    uint16_t address;
    log(LDEBUG "RET Z");

    if (!regs->f.z)
    {
        return 0;
    }
    
    if (read_word(&address, regs->sp))
    {
        return -1;
    }
    regs->sp += 2;

    regs->pc = address;
    return 0;
}

OPCODE(RET_NC)
{
    uint16_t address;
    log(LDEBUG "RET NC");

    if (regs->f.c)
    {
        return 0;
    }
    
    if (read_word(&address, regs->sp))
    {
        return -1;
    }
    regs->sp += 2;

    regs->pc = address;
    return 0;
}

OPCODE(RET_C)
{
    uint16_t address;
    log(LDEBUG "RET C");

    if (!regs->f.c)
    {
        return 0;
    }
    
    if (read_word(&address, regs->sp))
    {
        return -1;
    }
    regs->sp += 2;

    regs->pc = address;
    return 0;
}

OPCODE(RETI)
{
    uint16_t address;

    if (read_word(&address, regs->sp))
    {
        return -1;
    }
    regs->sp += 2;

    regs->pc = address;
    *enable_irq = 2;
    log(LDEBUG "RETI");
    return 0;
}

void register_opcodes()
{
    /* ----------- Misc. ----------- */
    ADD_OPCODE(0x00, 1, 1, NOP);
    ADD_OPCODE(0xCB, 2, 8, CB); // This needs to be fixed: there are CB opcodes that require more than 8 cycles.
    ADD_OPCODE(0x27, 1, 4, DAA);
    ADD_OPCODE(0x2F, 1, 4, CPL);
    ADD_OPCODE(0x3F, 1, 4, CCF);
    ADD_OPCODE(0x37, 1, 4, SCF);
    ADD_OPCODE(0x76, 1, 4, HALT);
    ADD_OPCODE(0x10, 2, 4, STOP);
    ADD_OPCODE(0xF3, 1, 4, DI);
    ADD_OPCODE(0xFB, 1, 4, EI);
    ADD_OPCODE(0x07, 1, 4, RLCA);
    ADD_OPCODE(0x17, 1, 4, RLA);

    /* -------- 8-Bit Loads -------- */
    // LD reg8, imm8
    ADD_OPCODE(0x06, 2, 8, LD_B_n);
    ADD_OPCODE(0x0E, 2, 8, LD_C_n);
    ADD_OPCODE(0x16, 2, 8, LD_D_n);
    ADD_OPCODE(0x1E, 2, 8, LD_E_n);
    ADD_OPCODE(0x26, 2, 8, LD_H_n);
    ADD_OPCODE(0x2E, 2, 8, LD_L_n);

    // LD reg8, reg8
    ADD_OPCODE(0x7F, 1, 4, LD_A_A);
    ADD_OPCODE(0x78, 1, 4, LD_A_B);
    ADD_OPCODE(0x79, 1, 4, LD_A_C);
    ADD_OPCODE(0x7A, 1, 4, LD_A_D);
    ADD_OPCODE(0x7B, 1, 4, LD_A_E);
    ADD_OPCODE(0x7C, 1, 4, LD_A_H);
    ADD_OPCODE(0x7D, 1, 4, LD_A_L);

    ADD_OPCODE(0x47, 1, 4, LD_B_A);
    ADD_OPCODE(0x40, 1, 4, LD_B_B);
    ADD_OPCODE(0x41, 1, 4, LD_B_C);
    ADD_OPCODE(0x42, 1, 4, LD_B_D);
    ADD_OPCODE(0x43, 1, 4, LD_B_E);
    ADD_OPCODE(0x44, 1, 4, LD_B_H);
    ADD_OPCODE(0x45, 1, 4, LD_B_L);

    ADD_OPCODE(0x4F, 1, 4, LD_C_A);
    ADD_OPCODE(0x48, 1, 4, LD_C_B);
    ADD_OPCODE(0x49, 1, 4, LD_C_C);
    ADD_OPCODE(0x4A, 1, 4, LD_C_D);
    ADD_OPCODE(0x4B, 1, 4, LD_C_E);
    ADD_OPCODE(0x4C, 1, 4, LD_C_H);
    ADD_OPCODE(0x4D, 1, 4, LD_C_L);

    ADD_OPCODE(0x57, 1, 4, LD_D_A);
    ADD_OPCODE(0x50, 1, 4, LD_D_B);
    ADD_OPCODE(0x51, 1, 4, LD_D_C);
    ADD_OPCODE(0x52, 1, 4, LD_D_D);
    ADD_OPCODE(0x53, 1, 4, LD_D_E);
    ADD_OPCODE(0x54, 1, 4, LD_D_H);
    ADD_OPCODE(0x55, 1, 4, LD_D_L);

    ADD_OPCODE(0x5F, 1, 4, LD_E_A);
    ADD_OPCODE(0x58, 1, 4, LD_E_B);
    ADD_OPCODE(0x59, 1, 4, LD_E_C);
    ADD_OPCODE(0x5A, 1, 4, LD_E_D);
    ADD_OPCODE(0x5B, 1, 4, LD_E_E);
    ADD_OPCODE(0x5C, 1, 4, LD_E_H);
    ADD_OPCODE(0x5D, 1, 4, LD_E_L);

    ADD_OPCODE(0x67, 1, 4, LD_H_A);
    ADD_OPCODE(0x60, 1, 4, LD_H_B);
    ADD_OPCODE(0x61, 1, 4, LD_H_C);
    ADD_OPCODE(0x62, 1, 4, LD_H_D);
    ADD_OPCODE(0x63, 1, 4, LD_H_E);
    ADD_OPCODE(0x64, 1, 4, LD_H_H);
    ADD_OPCODE(0x65, 1, 4, LD_H_L);

    ADD_OPCODE(0x6F, 1, 4, LD_L_A);
    ADD_OPCODE(0x68, 1, 4, LD_L_B);
    ADD_OPCODE(0x69, 1, 4, LD_L_C);
    ADD_OPCODE(0x6A, 1, 4, LD_L_D);
    ADD_OPCODE(0x6B, 1, 4, LD_L_E);
    ADD_OPCODE(0x6C, 1, 4, LD_L_H);
    ADD_OPCODE(0x6D, 1, 4, LD_L_L);

    // LD reg8, (reg16)
    ADD_OPCODE(0x0A, 1, 8, LD_A_BC);
    ADD_OPCODE(0x1A, 1, 8, LD_A_DE);
    ADD_OPCODE(0x7E, 1, 8, LD_A_HL);
    ADD_OPCODE(0x46, 1, 8, LD_B_HL);
    ADD_OPCODE(0x4E, 1, 8, LD_C_HL);
    ADD_OPCODE(0x56, 1, 8, LD_D_HL);
    ADD_OPCODE(0x5E, 1, 8, LD_E_HL);
    ADD_OPCODE(0x66, 1, 8, LD_H_HL);
    ADD_OPCODE(0x6E, 1, 8, LD_L_HL);

    // LD (reg16), reg8
    ADD_OPCODE(0x02, 1, 8, LD_BC_A);
    ADD_OPCODE(0x12, 1, 8, LD_DE_A);
    ADD_OPCODE(0x77, 1, 8, LD_HL_A);
    ADD_OPCODE(0x70, 1, 8, LD_HL_B);
    ADD_OPCODE(0x71, 1, 8, LD_HL_C);
    ADD_OPCODE(0x72, 1, 8, LD_HL_D);
    ADD_OPCODE(0x73, 1, 8, LD_HL_E);
    ADD_OPCODE(0x74, 1, 8, LD_HL_H);
    ADD_OPCODE(0x75, 1, 8, LD_HL_L);

    // LD (reg16), imm8
    ADD_OPCODE(0x36, 2, 12, LD_HL_n);

    // LD reg8, (imm16)
    ADD_OPCODE(0xFA, 3, 16, LD_A_nn);

    // LD (imm16), reg8
    ADD_OPCODE(0xEA, 3, 16, LD_nn_A);

    // LD A, (C)
    ADD_OPCODE(0xF2, 1, 8, LD_A_C2);

    // LD (C), A
    ADD_OPCODE(0xE2, 1, 8, LD_C_A2);

    // LDD A, (HL)
    ADD_OPCODE(0x3A, 1, 8, LDD_A_HL);

    // LDD (HL), A
    ADD_OPCODE(0x32, 1, 8, LDD_HL_A);

    // LDI A, (HL)
    ADD_OPCODE(0x2A, 1, 8, LDI_A_HL);

    // LDI A, (HL)
    ADD_OPCODE(0x22, 1, 8, LDI_HL_A);

    // LDH (n), A
    ADD_OPCODE(0xE0, 2, 12, LDH_n_A);

    // LDH A, (n)
    ADD_OPCODE(0xF0, 2, 12, LDH_A_n);

    /* -------- 16-Bit Loads ------- */

    // LD reg16, imm16
    ADD_OPCODE(0x01, 3, 12, LD_BC_nn);
    ADD_OPCODE(0x11, 3, 12, LD_DE_nn);
    ADD_OPCODE(0x21, 3, 12, LD_HL_nn);
    ADD_OPCODE(0x31, 3, 12, LD_SP_nn);

    // LD reg16, reg16
    ADD_OPCODE(0xF9, 1, 8, LD_SP_HL);

    // LDHL SP, n
    ADD_OPCODE(0xF8, 2, 12, LDHL_SP_n);

    // LD (nn), SP
    ADD_OPCODE(0x08, 3, 20, LD_nn_SP);

    // PUSH reg16
    ADD_OPCODE(0xF5, 1, 16, PUSH_AF);
    ADD_OPCODE(0xC5, 1, 16, PUSH_BC);
    ADD_OPCODE(0xD5, 1, 16, PUSH_DE);
    ADD_OPCODE(0xE5, 1, 16, PUSH_HL);

    // POP reg16
    ADD_OPCODE(0xF1, 1, 12, POP_AF);
    ADD_OPCODE(0xC1, 1, 12, POP_BC);
    ADD_OPCODE(0xD1, 1, 12, POP_DE);
    ADD_OPCODE(0xE1, 1, 12, POP_HL);

    /* ---------- 8-Bit ALU -------- */

    // ADD A, reg8
    ADD_OPCODE(0x87, 1, 4, ADD_A_A);
    ADD_OPCODE(0x80, 1, 4, ADD_A_B);
    ADD_OPCODE(0x81, 1, 4, ADD_A_C);
    ADD_OPCODE(0x82, 1, 4, ADD_A_D);
    ADD_OPCODE(0x83, 1, 4, ADD_A_E);
    ADD_OPCODE(0x84, 1, 4, ADD_A_H);
    ADD_OPCODE(0x85, 1, 4, ADD_A_L);

    // ADD A, (HL)
    ADD_OPCODE(0x86, 1, 8, ADD_A_HL);

    // ADD A, imm8
    ADD_OPCODE(0xC6, 2, 8, ADD_A_n);

    // ADC A, reg8
    ADD_OPCODE(0x8F, 1, 4, ADC_A_A);
    ADD_OPCODE(0x88, 1, 4, ADC_A_B);
    ADD_OPCODE(0x89, 1, 4, ADC_A_C);
    ADD_OPCODE(0x8A, 1, 4, ADC_A_D);
    ADD_OPCODE(0x8B, 1, 4, ADC_A_E);
    ADD_OPCODE(0x8C, 1, 4, ADC_A_H);
    ADD_OPCODE(0x8D, 1, 4, ADC_A_L);

    // ADC A, (HL)
    ADD_OPCODE(0x8E, 1, 8, ADC_A_HL);

    // ADC A, imm8
    ADD_OPCODE(0xCE, 2, 8, ADC_A_n);

    // SUB A, reg8
    ADD_OPCODE(0x97, 1, 4, SUB_A_A);
    ADD_OPCODE(0x90, 1, 4, SUB_A_B);
    ADD_OPCODE(0x91, 1, 4, SUB_A_C);
    ADD_OPCODE(0x92, 1, 4, SUB_A_D);
    ADD_OPCODE(0x93, 1, 4, SUB_A_E);
    ADD_OPCODE(0x94, 1, 4, SUB_A_H);
    ADD_OPCODE(0x95, 1, 4, SUB_A_L);

    // SUB A, (HL)
    ADD_OPCODE(0x96, 1, 8, SUB_A_HL);

    // SUB A, imm8
    ADD_OPCODE(0xD6, 2, 8, SUB_A_n);

    // SBC A, reg8
    ADD_OPCODE(0x9F, 1, 4, SBC_A_A);
    ADD_OPCODE(0x98, 1, 4, SBC_A_B);
    ADD_OPCODE(0x99, 1, 4, SBC_A_C);
    ADD_OPCODE(0x9A, 1, 4, SBC_A_D);
    ADD_OPCODE(0x9B, 1, 4, SBC_A_E);
    ADD_OPCODE(0x9C, 1, 4, SBC_A_H);
    ADD_OPCODE(0x9D, 1, 4, SBC_A_L);

    // SBC A, (HL)
    ADD_OPCODE(0x9E, 1, 8, SBC_A_HL);

    // SBC A, imm8
    ADD_OPCODE(0xDE, 2, 8, SBC_A_n);

    // AND A, reg8
    ADD_OPCODE(0xA7, 1, 4, AND_A_A);
    ADD_OPCODE(0xA0, 1, 4, AND_A_B);
    ADD_OPCODE(0xA1, 1, 4, AND_A_C);
    ADD_OPCODE(0xA2, 1, 4, AND_A_D);
    ADD_OPCODE(0xA3, 1, 4, AND_A_E);
    ADD_OPCODE(0xA4, 1, 4, AND_A_H);
    ADD_OPCODE(0xA5, 1, 4, AND_A_L);

    // AND A, (HL)
    ADD_OPCODE(0xA6, 1, 8, AND_A_HL);
    
    // AND, A, imm8
    ADD_OPCODE(0xE6, 2, 8, AND_A_n);

    // OR A, reg8
    ADD_OPCODE(0xB7, 1, 4, OR_A_A);
    ADD_OPCODE(0xB0, 1, 4, OR_A_B);
    ADD_OPCODE(0xB1, 1, 4, OR_A_C);
    ADD_OPCODE(0xB2, 1, 4, OR_A_D);
    ADD_OPCODE(0xB3, 1, 4, OR_A_E);
    ADD_OPCODE(0xB4, 1, 4, OR_A_H);
    ADD_OPCODE(0xB5, 1, 4, OR_A_L);

    // OR A, (HL)
    ADD_OPCODE(0xB6, 1, 8, OR_A_HL);
    
    // OR, A, imm8
    ADD_OPCODE(0xF6, 2, 8, OR_A_n);

    // XOR A, reg8
    ADD_OPCODE(0xAF, 1, 4, XOR_A_A);
    ADD_OPCODE(0xA8, 1, 4, XOR_A_B);
    ADD_OPCODE(0xA9, 1, 4, XOR_A_C);
    ADD_OPCODE(0xAA, 1, 4, XOR_A_D);
    ADD_OPCODE(0xAB, 1, 4, XOR_A_E);
    ADD_OPCODE(0xAC, 1, 4, XOR_A_H);
    ADD_OPCODE(0xAD, 1, 4, XOR_A_L);

    // XOR A, (HL)
    ADD_OPCODE(0xAE, 1, 8, XOR_A_HL);
    
    // XOR, A, imm8
    ADD_OPCODE(0xEE, 2, 8, XOR_A_n);

    // CP A, reg8
    ADD_OPCODE(0xBF, 1, 4, CP_A_A);
    ADD_OPCODE(0xB8, 1, 4, CP_A_B);
    ADD_OPCODE(0xB9, 1, 4, CP_A_C);
    ADD_OPCODE(0xBA, 1, 4, CP_A_D);
    ADD_OPCODE(0xBB, 1, 4, CP_A_E);
    ADD_OPCODE(0xBC, 1, 4, CP_A_H);
    ADD_OPCODE(0xBD, 1, 4, CP_A_L);

    // CP A, (HL)
    ADD_OPCODE(0xBE, 1, 8, CP_A_HL);

    // CP A, imm8
    ADD_OPCODE(0xFE, 2, 8, CP_A_n);

    // INC reg8
    ADD_OPCODE(0x3C, 1, 4, INC_A);
    ADD_OPCODE(0x04, 1, 4, INC_B);
    ADD_OPCODE(0x0C, 1, 4, INC_C);
    ADD_OPCODE(0x14, 1, 4, INC_D);
    ADD_OPCODE(0x1C, 1, 4, INC_E);
    ADD_OPCODE(0x24, 1, 4, INC_H);
    ADD_OPCODE(0x2C, 1, 4, INC_L);

    // INC (HL)
    ADD_OPCODE(0x34, 1, 12, INC_HL);

    // DEC reg8
    ADD_OPCODE(0x3D, 1, 4, DEC_A);
    ADD_OPCODE(0x05, 1, 4, DEC_B);
    ADD_OPCODE(0x0D, 1, 4, DEC_C);
    ADD_OPCODE(0x15, 1, 4, DEC_D);
    ADD_OPCODE(0x1D, 1, 4, DEC_E);
    ADD_OPCODE(0x25, 1, 4, DEC_H);
    ADD_OPCODE(0x2D, 1, 4, DEC_L);

    // DEC (HL)
    ADD_OPCODE(0x35, 1, 12, DEC_HL);

    /* ---------- 16-Bit ALU -------- */

    // ADD HL, reg16
    ADD_OPCODE(0x09, 1, 8, ADD_HL_BC);
    ADD_OPCODE(0x19, 1, 8, ADD_HL_DE);
    ADD_OPCODE(0x29, 1, 8, ADD_HL_HL);
    ADD_OPCODE(0x39, 1, 8, ADD_HL_SP);

    // ADD SP, imm8
    ADD_OPCODE(0xE8, 2, 16, ADD_SP_n);

    // INC reg16
    ADD_OPCODE(0x03, 1, 8, INC_BC);
    ADD_OPCODE(0x13, 1, 8, INC_DE);
    ADD_OPCODE(0x23, 1, 8, INC_HL_2);
    ADD_OPCODE(0x33, 1, 8, INC_SP);

    // DEC reg16
    ADD_OPCODE(0x03, 1, 8, DEC_BC);
    ADD_OPCODE(0x13, 1, 8, DEC_DE);
    ADD_OPCODE(0x23, 1, 8, DEC_HL_2);
    ADD_OPCODE(0x33, 1, 8, DEC_SP);

    /* ------------- Jumps ---------- */

    ADD_OPCODE(0xC3, 3, 12, JP);

    ADD_OPCODE(0xC2, 3, 12, JP_NZ);
    ADD_OPCODE(0xCA, 3, 12, JP_Z);
    ADD_OPCODE(0xD2, 3, 12, JP_NC);
    ADD_OPCODE(0xDA, 3, 12, JP_C);

    ADD_OPCODE(0xE9, 1, 4, JP_HL);

    ADD_OPCODE(0x18, 2, 8, JR);

    ADD_OPCODE(0x20, 2, 8, JR_NZ);
    ADD_OPCODE(0x28, 2, 8, JR_Z);
    ADD_OPCODE(0x30, 2, 8, JR_NC);
    ADD_OPCODE(0x38, 2, 8, JR_C);

    /* ------------- Calls ---------- */

    ADD_OPCODE(0xCD, 3, 12, CALL);

    ADD_OPCODE(0xC4, 3, 12, CALL_NZ);
    ADD_OPCODE(0xCC, 3, 12, CALL_Z);
    ADD_OPCODE(0xD4, 3, 12, CALL_NC);
    ADD_OPCODE(0xDC, 3, 12, CALL_C);

    /* ----------- Restarts --------- */

    ADD_OPCODE(0xC7, 1, 32, RST_00);
    ADD_OPCODE(0xCF, 1, 32, RST_08);
    ADD_OPCODE(0xD7, 1, 32, RST_10);
    ADD_OPCODE(0xDF, 1, 32, RST_18);
    ADD_OPCODE(0xE7, 1, 32, RST_20);
    ADD_OPCODE(0xEF, 1, 32, RST_28);
    ADD_OPCODE(0xF7, 1, 32, RST_30);
    ADD_OPCODE(0xFF, 1, 32, RST_38);

    /* ------------ Returns --------- */

    ADD_OPCODE(0xC9, 1, 8, RET);

    ADD_OPCODE(0xC0, 1, 8, RET_NZ);
    ADD_OPCODE(0xC8, 1, 8, RET_Z);
    ADD_OPCODE(0xD0, 1, 8, RET_NC);
    ADD_OPCODE(0xD8, 1, 8, RET_C);

    ADD_OPCODE(0xD9, 1, 8, RETI);
}
