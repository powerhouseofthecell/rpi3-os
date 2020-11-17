#include "kernel/timer.hh"
#include "common/stdlib.hh"

uint64_t ticks = 0;

extern "C" void enable_interrupts();

// enables timer interrupts
void init_interrupts() {
    uint32_t reloadVal = 38400000U / HZ;
    
    // only bits 0:27 can be used for reloadVal
    assert(reloadVal < (1<<28));

    // initialize the timer (local timer), enable it and interrupts
    uint32_t* local_timer = (uint32_t*) LOCAL_TIMER;
    *local_timer = reloadVal | LOCAL_TIMER_ENABLE | LOCAL_TIMER_IRQ_ENABLE;
    
    // enables interrupts at the system level
    enable_interrupts();
}