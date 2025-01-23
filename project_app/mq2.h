#ifndef MQ2_H
#define MQ2_H

#include <stdbool.h>

extern bool gas_detected;

bool mq2_detect();
void mq2_monitor_task();

#endif
