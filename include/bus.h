#ifndef BUS__
#define BUS__

#include <inttypes.h>

typedef int(*bus_read_t)(void*,uint16_t,uint16_t);
typedef int(*bus_write_t)(void*,uint16_t,uint16_t);

struct bus_connection
{
	struct bus_connection *next;
	uint16_t start_address;
	uint16_t size;
	bus_read_t read_func;
	bus_write_t write_func;
};

int add_bus_connection(uint16_t start_address, uint16_t size, bus_read_t read_func, bus_write_t write_func);
int remove_bus_connection(uint16_t start_address);

int bus_read(void *buf, uint16_t src, uint16_t size);
int bus_write(void *buf, uint16_t dst, uint16_t size);

#endif

