#ifndef CPU__
#define CPU

#include <inttypes.h>


struct flag_reg {
	uint8_t nu : 4;
	uint8_t c : 1;
	uint8_t h : 1;
	uint8_t n : 1;
	uint8_t z : 1;
}

struct registers {
	struct flag_reg fl;
	uint16_t af;
	uint16_t bc;
	uint16_t de;
	uint16_t hl;
	uint16_t sp;
	uint16_t pc;
}

inline void init_registers(struct registers * regs);



#endif

