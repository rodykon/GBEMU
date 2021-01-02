#include <stdio.h>
#include <string.h>
#include "bus.h"
#include "cpu/cpu.h"

char mem[256];


int read(uint8_t *result, uint16_t src)
{
	*result = mem[src];
	return 0;
}

int write(uint8_t value, uint16_t addr)
{
	mem[addr] = value;
	return 0;
}

int main(int argc, const char *argv[])
{
	memset(mem, 0x01, 256);

    if (add_bus_connection(0x0100, 256, read, write))
    {
        return -1;
    }

    cpu_loop();

    remove_bus_connection(0x0100);
}

