#ifndef DHT22_H
#define DHT22_H

extern int temperature;
extern int humidity;

void get_dht22_data(int *sensor_data);
void dht22_monitor_task();

#endif