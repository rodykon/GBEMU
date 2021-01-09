#include "cpu/opcodes.h"
#include "bus.h"
#include "log.h"

/* ----------- Utils ----------- */

static int read_word(uint16_t *out, uint16_t address)
{
    uint8_t lsb, msb;

    if (bus_read(&lsb, address))
    {
        return -1;
    }

    if (bus_read(&msb, address + 1))
    {
        return -1;
    }

    *out = lsb + ((uint16_t)msb << 8);
    return 0;
}

static int write_word(uint16_t value, uint16_t address)
{
    // Write LSB
    if (bus_write((uint8_t)(value & 0xFF), address))
    {
        return -1;
    }

    // Write MSB
    if (bus_write((uint8_t)(value >> 8), address + 1))
    {
        return -1;
    }

    return 0;
}

#define ZERO(a, b) ((a) + (b) == 0) ? 1 : 0
#define CARRY(a, b) ((a) + (b) < (a)) ? 1 : 0
#define HALF_CARRY(a, b) ((((a) & 0xF) + ((a) & 0xF)) & 0x10) ? 1 : 0

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

    regs->f.z = ZERO(*val, (uint8_t)-1);
    regs->f.h = HALF_CARRY(*val, (uint8_t)-1);
    regs->f.n = 1;
}

/* ----------- Misc. ----------- */

OPCODE(NOP)
{
    log("DEBUG: NOP");
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
    log("LD B, 0x%02x", arg);
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
    log("LD C, 0x%02x", arg);
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
    log("LD D, 0x%02x", arg);
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
    log("LD E, 0x%02x", arg);
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
    log("LD H, 0x%02x", arg);
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
    log("LD L, 0x%02x", arg);
    return 0;
}

// LD reg8, reg8

OPCODE(LD_A_A)
{
    log("DEBUG: LD A, A");
    return 0;
}


OPCODE(LD_A_B)
{
    regs->a = regs->b;
    log("DEBUG: LD A, B");
    return 0;
}


OPCODE(LD_A_C)
{
    regs->a = regs->c;
    log("DEBUG: LD A, C");
    return 0;
}


OPCODE(LD_A_D)
{
    regs->a = regs->d;
    log("DEBUG: LD A, D");
    return 0;
}


OPCODE(LD_A_E)
{
    regs->a = regs->e;
    log("DEBUG: LD A, E");
    return 0;
}


OPCODE(LD_A_H)
{
    regs->a = regs->h;
    log("DEBUG: LD A, H");
    return 0;
}


OPCODE(LD_A_L)
{
    regs->a = regs->l;
    log("DEBUG: LD A, L");
    return 0;
}


OPCODE(LD_B_A)
{
    regs->b = regs->a;
    log("DEBUG: LD B, A");
    return 0;
}

OPCODE(LD_B_B)
{
    log("DEBUG: LD B, B");
    return 0;
}


OPCODE(LD_B_C)
{
    regs->b = regs->c;
    log("DEBUG: LD B, C");
    return 0;
}


OPCODE(LD_B_D)
{
    regs->b = regs->d;
    log("DEBUG: LD B, D");
    return 0;
}


OPCODE(LD_B_E)
{
    regs->b = regs->e;
    log("DEBUG: LD B, E");
    return 0;
}


OPCODE(LD_B_H)
{
    regs->b = regs->h;
    log("DEBUG: LD B, H");
    return 0;
}


OPCODE(LD_B_L)
{
    regs->b = regs->l;
    log("DEBUG: LD B, L");
    return 0;
}


OPCODE(LD_C_A)
{
    regs->c = regs->a;
    log("DEBUG: LD C, A");
    return 0;
}


OPCODE(LD_C_B)
{
    regs->c = regs->b;
    log("DEBUG: LD C, B");
    return 0;
}

OPCODE(LD_C_C)
{
    log("DEBUG: LD C, C");
    return 0;
}


OPCODE(LD_C_D)
{
    regs->c = regs->d;
    log("DEBUG: LD C, D");
    return 0;
}


OPCODE(LD_C_E)
{
    regs->c = regs->e;
    log("DEBUG: LD C, E");
    return 0;
}


OPCODE(LD_C_H)
{
    regs->c = regs->h;
    log("DEBUG: LD C, H");
    return 0;
}


OPCODE(LD_C_L)
{
    regs->c = regs->l;
    log("DEBUG: LD C, L");
    return 0;
}


OPCODE(LD_D_A)
{
    regs->d = regs->a;
    log("DEBUG: LD D, A");
    return 0;
}


OPCODE(LD_D_B)
{
    regs->d = regs->b;
    log("DEBUG: LD D, B");
    return 0;
}


OPCODE(LD_D_C)
{
    regs->d = regs->c;
    log("DEBUG: LD D, C");
    return 0;
}

OPCODE(LD_D_D)
{
    log("DEBUG: LD D, D");
    return 0;
}


OPCODE(LD_D_E)
{
    regs->d = regs->e;
    log("DEBUG: LD D, E");
    return 0;
}


OPCODE(LD_D_H)
{
    regs->d = regs->h;
    log("DEBUG: LD D, H");
    return 0;
}


OPCODE(LD_D_L)
{
    regs->d = regs->l;
    log("DEBUG: LD D, L");
    return 0;
}


OPCODE(LD_E_A)
{
    regs->e = regs->a;
    log("DEBUG: LD E, A");
    return 0;
}


OPCODE(LD_E_B)
{
    regs->e = regs->b;
    log("DEBUG: LD E, B");
    return 0;
}


OPCODE(LD_E_C)
{
    regs->e = regs->c;
    log("DEBUG: LD E, C");
    return 0;
}


OPCODE(LD_E_D)
{
    regs->e = regs->d;
    log("DEBUG: LD E, D");
    return 0;
}

OPCODE(LD_E_E)
{
    log("DEBUG: LD E, E");
    return 0;
}


OPCODE(LD_E_H)
{
    regs->e = regs->h;
    log("DEBUG: LD E, H");
    return 0;
}


OPCODE(LD_E_L)
{
    regs->e = regs->l;
    log("DEBUG: LD E, L");
    return 0;
}


OPCODE(LD_H_A)
{
    regs->h = regs->a;
    log("DEBUG: LD H, A");
    return 0;
}


OPCODE(LD_H_B)
{
    regs->h = regs->b;
    log("DEBUG: LD H, B");
    return 0;
}


OPCODE(LD_H_C)
{
    regs->h = regs->c;
    log("DEBUG: LD H, C");
    return 0;
}


OPCODE(LD_H_D)
{
    regs->h = regs->d;
    log("DEBUG: LD H, D");
    return 0;
}


OPCODE(LD_H_E)
{
    regs->h = regs->e;
    log("DEBUG: LD H, E");
    return 0;
}

OPCODE(LD_H_H)
{
    log("DEBUG: LD H, H");
    return 0;
}


OPCODE(LD_H_L)
{
    regs->h = regs->l;
    log("DEBUG: LD H, L");
    return 0;
}


OPCODE(LD_L_A)
{
    regs->l = regs->a;
    log("DEBUG: LD L, A");
    return 0;
}


OPCODE(LD_L_B)
{
    regs->l = regs->b;
    log("DEBUG: LD L, B");
    return 0;
}


OPCODE(LD_L_C)
{
    regs->l = regs->c;
    log("DEBUG: LD L, C");
    return 0;
}


OPCODE(LD_L_D)
{
    regs->l = regs->d;
    log("DEBUG: LD L, D");
    return 0;
}


OPCODE(LD_L_E)
{
    regs->l = regs->e;
    log("DEBUG: LD L, E");
    return 0;
}


OPCODE(LD_L_H)
{
    regs->l = regs->h;
    log("DEBUG: LD L, H");
    return 0;
}

OPCODE(LD_L_L)
{
    log("DEBUG: LD L, L");
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
    log("DEBUG: LD A, (BC)");
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
    log("DEBUG: LD A, (DE)");
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
    log("DEBUG: LD A, (HL)");
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
    log("DEBUG: LD B, (HL)");
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
    log("DEBUG: LD C, (HL)");
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
    log("DEBUG: LD D, (HL)");
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
    log("DEBUG: LD E, (HL)");
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
    log("DEBUG: LD H, (HL)");
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
    log("DEBUG: LD L, (HL)");
    return 0;
}

// LD (reg16), A

OPCODE(LD_BC_A)
{
    if (bus_write(regs->a, regs->bc))
    {
        return -1;
    }
    log("DEBUG: LD (BC), A");
    return 0;
}

OPCODE(LD_DE_A)
{
    if (bus_write(regs->a, regs->de))
    {
        return -1;
    }
    log("DEBUG: LD (DE), A");
    return 0;
}

OPCODE(LD_HL_A)
{
    if (bus_write(regs->a, regs->hl))
    {
        return -1;
    }
    log("DEBUG: LD (HL), A");
    return 0;
}

OPCODE(LD_HL_B)
{
    if (bus_write(regs->b, regs->hl))
    {
        return -1;
    }
    log("DEBUG: LD (HL), B");
    return 0;
}

OPCODE(LD_HL_C)
{
    if (bus_write(regs->c, regs->hl))
    {
        return -1;
    }
    log("DEBUG: LD (HL), C");
    return 0;
}

OPCODE(LD_HL_D)
{
    if (bus_write(regs->d, regs->hl))
    {
        return -1;
    }
    log("DEBUG: LD (HL), D");
    return 0;
}

OPCODE(LD_HL_E)
{
    if (bus_write(regs->e, regs->hl))
    {
        return -1;
    }
    log("DEBUG: LD (HL), E");
    return 0;
}

OPCODE(LD_HL_H)
{
    if (bus_write(regs->h, regs->hl))
    {
        return -1;
    }
    log("DEBUG: LD (HL), H");
    return 0;
}

OPCODE(LD_HL_L)
{
    if (bus_write(regs->l, regs->hl))
    {
        return -1;
    }
    log("DEBUG: LD (HL), L");
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
    log("DEBUG: LD (HL), 0x%02x", imm8);
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
    log("DEBUG: LD A, (0x%02x)", imm16);
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

    log("DEBUG: LD (0x%02x), A", imm16);
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

    log("DEBUG: LD A, (C)");
    return 0;
}

// LD (C), A

OPCODE(LD_C_A2)
{
    if (bus_write(regs->a, 0xFF00 + regs->c))
    {
        return -1;
    }

    log("DEBUG: LD (C), A");
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

    log("DEBUG: LDD A, (HL)");
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

    log("DEBUG: LDD (HL), A");
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

    log("DEBUG: LDI A, (HL)");
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

    log("DEBUG: LDI (HL), A");
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

    log("DEBUG: LDH (0x%02x), A", imm8);
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

    log("DEBUG: LDH A, (0x%02x)", imm8);
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

    log("DEBUG: LD BC, 0x%04x", imm16);
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

    log("DEBUG: LD DE, 0x%04x", imm16);
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

    log("DEBUG: LD HL, 0x%04x", imm16);
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

    log("DEBUG: LD SP, 0x%04x", imm16);
    return 0;
}

// LD reg16, reg16

OPCODE(LD_SP_HL)
{
    regs->sp = regs->hl;

    log("DEBUG: LD SP, HL");
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
    log("DEBUG: LDHL SP, 0x%02x", imm8);
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

    log("DEBUG: LD (0x%04x), SP", imm16);
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

    log("DEBUG: PUSH AF");
    return 0;
}

OPCODE(PUSH_BC)
{
    regs->sp -= 2;

    if (write_word(regs->bc, regs->sp))
    {
        return -1;
    }

    log("DEBUG: PUSH BC");
    return 0;
}

OPCODE(PUSH_DE)
{
    regs->sp -= 2;

    if (write_word(regs->de, regs->sp))
    {
        return -1;
    }

    log("DEBUG: PUSH DE");
    return 0;
}

OPCODE(PUSH_HL)
{
    regs->sp -= 2;

    if (write_word(regs->hl, regs->sp))
    {
        return -1;
    }

    log("DEBUG: PUSH HL");
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

    log("DEBUG: POP AF");
    return 0;
}

OPCODE(POP_BC)
{
    if (read_word(&regs->bc, regs->sp))
    {
        return -1;
    }
    regs->sp += 2;

    log("DEBUG: POP BC");
    return 0;
}

OPCODE(POP_DE)
{
    if (read_word(&regs->de, regs->sp))
    {
        return -1;
    }
    regs->sp += 2;

    log("DEBUG: POP DE");
    return 0;
}

OPCODE(POP_HL)
{
    if (read_word(&regs->hl, regs->sp))
    {
        return -1;
    }
    regs->sp += 2;

    log("DEBUG: POP HL");
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
    log("DEBUG: ADD A, A");
    return 0;
}

OPCODE(ADD_A_B)
{
    regs->f.z = ZERO(regs->a, regs->b);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, regs->b);
    regs->f.c = CARRY(regs->a, regs->b);

    regs->a += regs->b;
    log("DEBUG: ADD A, B");
    return 0;
}

OPCODE(ADD_A_C)
{
    regs->f.z = ZERO(regs->a, regs->c);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, regs->c);
    regs->f.c = CARRY(regs->a, regs->c);

    regs->a += regs->c;
    log("DEBUG: ADD A, C");
    return 0;
}

OPCODE(ADD_A_D)
{
    regs->f.z = ZERO(regs->a, regs->d);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, regs->d);
    regs->f.c = CARRY(regs->a, regs->d);

    regs->a += regs->d;
    log("DEBUG: ADD A, D");
    return 0;
}

OPCODE(ADD_A_E)
{
    regs->f.z = ZERO(regs->a, regs->e);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, regs->e);
    regs->f.c = CARRY(regs->a, regs->e);

    regs->a += regs->e;
    log("DEBUG: ADD A, E");
    return 0;
}

OPCODE(ADD_A_H)
{
    regs->f.z = ZERO(regs->a, regs->h);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, regs->h);
    regs->f.c = CARRY(regs->a, regs->h);

    regs->a += regs->h;
    log("DEBUG: ADD A, H");
    return 0;
}

OPCODE(ADD_A_L)
{
    regs->f.z = ZERO(regs->a, regs->l);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, regs->l);
    regs->f.c = CARRY(regs->a, regs->l);

    regs->a += regs->l;
    log("DEBUG: ADD A, L");
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
    log("DEBUG: ADD A, (HL)");
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
    log("DEBUG: ADD A, 0x%02x", imm8);
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
    log("DEBUG: ADC A, A");
    return 0;
}

OPCODE(ADC_A_B)
{
    regs->f.z = ZERO(regs->a, regs->b + regs->f.c);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, regs->b + regs->f.c);
    regs->f.c = CARRY(regs->a, regs->b + regs->f.c);

    regs->a += regs->b + regs->f.c;
    log("DEBUG: ADC A, B");
    return 0;
}

OPCODE(ADC_A_C)
{
    regs->f.z = ZERO(regs->a, regs->c + regs->f.c);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, regs->c + regs->f.c);
    regs->f.c = CARRY(regs->a, regs->c + regs->f.c);

    regs->a += regs->c + regs->f.c;
    log("DEBUG: ADC A, C");
    return 0;
}

OPCODE(ADC_A_D)
{
    regs->f.z = ZERO(regs->a, regs->d + regs->f.c);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, regs->d + regs->f.c);
    regs->f.c = CARRY(regs->a, regs->d + regs->f.c);

    regs->a += regs->d + regs->f.c;
    log("DEBUG: ADC A, D");
    return 0;
}

OPCODE(ADC_A_E)
{
    regs->f.z = ZERO(regs->a, regs->e + regs->f.c);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, regs->e + regs->f.c);
    regs->f.c = CARRY(regs->a, regs->e + regs->f.c);

    regs->a += regs->e + regs->f.c;
    log("DEBUG: ADC A, E");
    return 0;
}

OPCODE(ADC_A_H)
{
    regs->f.z = ZERO(regs->a, regs->h + regs->f.c);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, regs->h + regs->f.c);
    regs->f.c = CARRY(regs->a, regs->h + regs->f.c);

    regs->a += regs->h + regs->f.c;
    log("DEBUG: ADC A, H");
    return 0;
}

OPCODE(ADC_A_L)
{
    regs->f.z = ZERO(regs->a, regs->l + regs->f.c);
    regs->f.n = 0;
    regs->f.h = HALF_CARRY(regs->a, regs->l + regs->f.c);
    regs->f.c = CARRY(regs->a, regs->l + regs->f.c);

    regs->a += regs->l;
    log("DEBUG: ADC A, L");
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
    log("DEBUG: ADC A, (HL)");
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
    log("DEBUG: ADC A, 0x%02x", imm8);
    return 0;
}

void register_opcodes()
{
    /* ----------- Misc. ----------- */
    ADD_OPCODE(0x00, 1, 1, NOP);

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

    // SSUB A, (HL)
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

}
