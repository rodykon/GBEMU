#include "mem_utils.h"
#include "bus.h"

int read_word(uint16_t *out, uint16_t address)
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

int write_word(uint16_t value, uint16_t address)
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