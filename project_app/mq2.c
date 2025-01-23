#include <FreeRTOS.h>
#include <task.h>
#include <stdbool.h>
#include "bl_gpio.h"
#include "bl_timer.h"

// GPIO pin connected to MQ2
#define MQ2_PIN 5

bool gas_detected = false;

bool mq2_detect() {
    bl_gpio_enable_input(MQ2_PIN, 0, 0);
    int value = bl_gpio_input_get_value(MQ2_PIN);
    return value == 0; // Return true if gas is detected (low signal)
}

void mq2_monitor_task() {
    while (1) {
        if (mq2_detect()) {
            gas_detected = true;
        } else {
            gas_detected = false;
        }

        // Wait for 2 seconds before next reading
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}