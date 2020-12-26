#include <stdio.h>
#include <string.h>
#include "bus.h"


char mem[256];


int read(void *buf, uint16_t src, uint16_t size)
{
	printf("Read from %d\n", src);
	memcpy(buf, &mem[src], size);
	return 0;
}

int write(void *buf, uint16_t dst, uint16_t size)
{
	printf("Write to %d\n", dst);
	memcpy(&mem[dst], buf, size);
	return 0;
}

int main(int argc, const char *argv[])
{
	memset(mem, 'a', 256);
	char buf[256];

	printf("add_bus_connection returned: %d\n", add_bus_connection(100, 256, read, write));
	printf("bus_read from address %d returned result %d\n", 1, bus_read(buf, 1, 4));
	printf("bus_read from address %d returned result %d\n", 500, bus_read(buf, 500, 4));
	printf("bus_read from address %d returned result %d\n", 355, bus_read(buf, 355, 4));
	printf("bus_read from address %d returned result %d\n", 300, bus_read(buf, 300, 4));

	
	printf("bus_write to address %d returned result %d\n", 1, bus_write(buf, 1, 4));
	printf("bus_write to address %d returned result %d\n", 500, bus_write(buf, 500, 4));
	printf("bus_write to address %d returned result %d\n", 355, bus_write(buf, 355, 4));
	printf("bus_write to address %d returned result %d\n", 300, bus_write(buf, 300, 4));
}

