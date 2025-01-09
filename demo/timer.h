#ifndef TIMER_H
#define TIMER_H

#define ADDR_CTRL 0x8
#define CTRL_START_BIT 0
#define CTRL_STOP_BIT 1

#define ADDR_STATUS 0x9
#define STATUS_RUNNING_BIT 0

#define ADDR_PRESCALER 0xa
#define ADDR_TIMER 0xb

typedef struct {
    uint32_t ctrl;
    uint32_t status;
    uint32_t prescaler;
    uint32_t timer;
} timer_t;

void timer_start();
void timer_stop();
int timer_is_running();
void timer_set_prescaler(uint32_t prescaler);
uint32_t timer_read();
void timer_write(uint32_t data);


#endif // TIMER_H