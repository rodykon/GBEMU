#include <stdio.h>
#include <string.h>
#include "bus.h"


char mem[256];


int read(uint8_t *result, uint16_t src)
{
	printf("Read from %d\n", src);
	*result = mem[src];
	return 0;
}

int write(uint8_t value, uint16_t addr)
{
	printf("Write to %d\n", addr);
	mem[addr] = value;
	return 0;
}

int main(int argc, const char *argv[])
{
	memset(mem, 'a', 256);
	uint8_t result;

	printf("add_bus_connection returned: %d\n", add_bus_connection(100, 256, read, write));
	printf("bus_read from address %d returned result %d\n", 1, bus_read(&result, 1));
	printf("bus_read from address %d returned result %d\n", 500, bus_read(&result, 500));
	printf("bus_read from address %d returned result %d\n", 355, bus_read(&result, 355));
	printf("bus_read from address %d returned result %d\n", 356, bus_read(&result, 356));
	printf("bus_read from address %d returned result %d\n", 300, bus_read(&result, 300));

	
	printf("bus_write to address %d returned result %d\n", 1, bus_write('b', 1));
	printf("bus_write to address %d returned result %d\n", 500, bus_write('b', 500));
	printf("bus_write to address %d returned result %d\n", 355, bus_write('b', 355));
	printf("bus_write to address %d returned result %d\n", 356, bus_write('b', 356));
	printf("bus_write to address %d returned result %d\n", 300, bus_write('b', 300));
    printf("mem: %256s\n", mem);

	printf("remove_bus_connection returned: %d\n", remove_bus_connection(100));

}

