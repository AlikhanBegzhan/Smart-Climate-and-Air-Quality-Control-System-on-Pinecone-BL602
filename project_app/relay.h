#ifndef RELAY_H
#define RELAY_H

// Limit Settings
extern int temperature_limit_low;
extern int temperature_limit_high;
extern int humidity_limit_high;

// Fans Status
extern int cooling_fans_status;
extern int heater_fan_status;

// Manual Control Variable
extern bool manual_control;

void get_dht22_data(int *sensor_data);
void dht22_monitor_task();

#endif