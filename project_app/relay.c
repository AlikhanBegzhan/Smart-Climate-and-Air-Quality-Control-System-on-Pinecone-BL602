#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include <stdbool.h>
#include "bl_gpio.h"
#include "bl_timer.h"

// Sensor data
#include "mq2.h"
#include "dht22.h"

#define RELAY_FANS_PIN 0
#define RELAY_HEATER_PIN 1

// Thresholds
// Temperature in °C, Humidity in %
// Values in tenths, e.g. 20.5°C = 205, 60.5% = 605
// Values from DHT22 sensor come in tenths therefore we use tenths for better precision
int temperature_limit_low = 200;
int temperature_limit_high = 280;
int humidity_limit_high = 600;

int cooling_fans_status = 0;
int heater_fan_status = 0;

bool manual_control = false;

void relay_control_task() {
    bl_gpio_enable_output(RELAY_FANS_PIN, 0, 0);
    bl_gpio_enable_output(RELAY_HEATER_PIN, 0, 0);

    // Wait for 2 seconds before sensors start up
    vTaskDelay(pdMS_TO_TICKS(2000));

    while (1) {
        if (!manual_control) {
            // Settings for the relay
            bool low_temperature = temperature < temperature_limit_low;
            bool high_temperature = temperature > temperature_limit_high;
            bool high_humidity = humidity > humidity_limit_high;
            // Controlling switches of the relay
            if (gas_detected || high_temperature || high_humidity) {
                cooling_fans_status = 1;
                heater_fan_status = 0;
            } else if (low_temperature) {
                cooling_fans_status = 0;
                heater_fan_status = 1;
            } else {
                cooling_fans_status = 0;
                heater_fan_status = 0;
            }
        }
        bl_gpio_output_set(RELAY_FANS_PIN, cooling_fans_status);
        bl_gpio_output_set(RELAY_HEATER_PIN, heater_fan_status);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}