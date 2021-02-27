#include "bus.h"
#include <stdlib.h>
#include "log.h"

static struct bus_connection *bus_list = NULL;

static inline int does_overlap(struct bus_connection *first, struct bus_connection *second)
{
	return (first->start_address >= second->start_address && first->start_address < second->start_address + second->size) ||
	       (second->start_address >= first->start_address && second->start_address < first->start_address + first->size);
}

static struct bus_connection * find_connection(uint16_t address)
{
	struct bus_connection *current = bus_list;

	while (current != NULL)
	{
		if (address >= current->start_address && address < current->start_address + current->size)
		{
			return current;
		}

		current = current->next;
	}

	return NULL;
}


int add_bus_connection(uint16_t start_address, uint16_t size, bus_read_t read_func, bus_write_t write_func)
{
	struct bus_connection *new_connection;
	struct bus_connection *current = bus_list;
	
	new_connection = (struct bus_connection*)malloc(sizeof(struct bus_connection));
	if (new_connection == NULL)
	{
		return -1;
	}
	
	new_connection->start_address = start_address;
	new_connection->size = size;
	new_connection->read_func = read_func;
	new_connection->write_func = write_func;
	new_connection->next = NULL;

	if (current == NULL)
	{
		bus_list = new_connection;
		return 0;
	}

	if (current->start_address > new_connection->start_address)
	{
		if (does_overlap(new_connection, current))
		{
			goto error;
		}
		bus_list = new_connection;
		new_connection->next = current;
		return 0;
	}

	for (; current->next != NULL; current = current->next)
	{
		if (current->next->start_address > new_connection->start_address)
		{
			if (does_overlap(new_connection, current) || does_overlap(new_connection, current->next))
			{
				goto error;
			}

			new_connection->next = current->next;
			current->next = new_connection;
			return 0;
		}
	}

	if (new_connection->start_address > current->start_address + current->size)
	{
		current->next = new_connection;
		return 0;
	}

error:
    log("Failed to add bus_connection %04x", start_address);
	free(new_connection);
	return -1;	
}

int remove_bus_connection(uint16_t start_address)
{
	struct bus_connection *to_free = NULL;
	struct bus_connection *current = bus_list;

	if (current->start_address == start_address)
	{
		bus_list = current->next;
		free(current);
        return 0;
	}

	for (; current->next != NULL; current = current->next)
	{
		if (current->next->start_address == start_address)
		{
			to_free = current->next;
			current->next = current->next->next;
			free(to_free);
			return 0;
		}
	}
    log("ERROR: Failed to remove bus connection %04x", start_address);
	return -1;
}

int bus_read(uint8_t *result, uint16_t src)
{
	struct bus_connection *connection = find_connection(src);
	if (connection == NULL || connection->read_func(result, src - connection->start_address))
	{
		log("ERROR: Could not read from bus address %04x", src);
        return -1;
	}
    return 0;
}

int bus_write(uint8_t src, uint16_t dst)
{
	struct bus_connection *connection = find_connection(dst);
	if (connection == NULL || connection->write_func(src, dst - connection->start_address))
	{
        log("ERROR: Could not write to bus address %04x", dst);
		return -1;
	}
    return 0;
}

