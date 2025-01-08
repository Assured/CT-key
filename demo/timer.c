#include "timer.h"
#include <generated/csr.h>




void timer_start() {
    uint32_t ctrl = 0;
    ctrl |= (1 << CSR_TIMER_CTRL_REG_CS_OFFSET) | (ADDR_STATUS << CSR_TIMER_CTRL_REG_ADDR_OFFSET);
    timer_CTRL_REG_write(ctrl);

    while(!(timer_READY_REG_read() & 0x1)) {
        busy_wait(10);
    }

    while(!(timer_READ_REG_read() & (1 << STATUS_RUNNING_BIT))) {
        busy_wait(10);
    }
}

void timer_stop() {
    uint32_t ctrl = 0;
    ctrl &= ~(1 << CSR_TIMER_CTRL_REG_CS_OFFSET);
    timer_CTRL_REG_write(ctrl);
}

int timer_is_running() {
    return timer_READ_REG_read() & (1 << STATUS_RUNNING_BIT);
}

void timer_set_prescaler(uint32_t prescaler) {
    if(timer_is_running()) {
        timer_stop();
    }
    
    timer_WRITE_REG_write(prescaler);
    uint32_t ctrl = 0;
    ctrl |= (ADDR_PRESCALER << CSR_TIMER_CTRL_REG_ADDR_OFFSET) | (1 << CSR_TIMER_CTRL_REG_WE_OFFSET);
    timer_CTRL_REG_write(ctrl);
}