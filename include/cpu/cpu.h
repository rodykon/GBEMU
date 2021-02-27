#ifndef CPU__
#define CPU__

enum cpu_state
{
    STATE_NORMAL,
    STATE_HALT,
    STATE_STOP
};


void cpu_loop();

#endif

