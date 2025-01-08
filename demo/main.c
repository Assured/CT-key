#include <stdio.h>
#include <generated/csr.h>

#define ADDR_CTRL 0x8
#define CTRL_START_BIT 0
#define CTRL_STOP_BIT 1

#define ADDR_STATUS 0x9
#define STATUS_RUNNING_BIT 0

#define ADDR_PRESCALER 0xa
#define ADDR_TIMER 0xb

int main(void) {
    uint32_t ctrl = 0;
    uint32_t data = 0x1;

    ctrl |= (1 << CSR_TIMER_CTRL_REG_CS_OFFSET);
    
    timer_CTRL_REG_write(ctrl);
    printf("%lx\n", ctrl);

    printf("Waiting for timer to be ready...\n");

    while(!(timer_STATUS_REG_read() & 0x1)) {
        busy_wait(10);
    }

    ctrl = timer_CTRL_REG_read();
    printf("%lx\n", ctrl);

    timer_WRITE_REG_write(data);
    ctrl |= (ADDR_CTRL << CSR_TIMER_CTRL_REG_ADDR_OFFSET) | (1 << CSR_TIMER_CTRL_REG_WE_OFFSET);
    printf("%lx\n", ctrl);
    timer_CTRL_REG_write(ctrl);

    busy_wait(10);

    ctrl &= ~(1 << CSR_TIMER_CTRL_REG_WE_OFFSET);
    ctrl &= ~(0xff << CSR_TIMER_CTRL_REG_ADDR_OFFSET);
    ctrl |= (ADDR_STATUS << CSR_TIMER_CTRL_REG_ADDR_OFFSET);

    printf("%lx\n", ctrl);

    timer_CTRL_REG_write(ctrl);

    printf("Timer is ready!\nWaiting for timer to start...\n");

    while(!(timer_READ_REG_read() & (1 << STATUS_RUNNING_BIT))) {
        busy_wait(10);
    }

    printf("Timer has started!\n");

    ctrl = timer_CTRL_REG_read();
    ctrl &= ~(0xff << CSR_TIMER_CTRL_REG_ADDR_OFFSET);
    ctrl |= (ADDR_TIMER << CSR_TIMER_CTRL_REG_ADDR_OFFSET);
    timer_CTRL_REG_write(ctrl);

    while(1) {
        uint32_t t = timer_READ_REG_read();
        busy_wait(100); // 1 second

        printf("Timer: %ld\n", t);
    }

    return 0;
}