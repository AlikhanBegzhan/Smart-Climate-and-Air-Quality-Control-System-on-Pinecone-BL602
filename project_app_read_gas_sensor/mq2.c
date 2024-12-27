// FreeRTOS
#include <FreeRTOS.h>
#include <task.h>

// Input/output
#include <stdio.h>

#include <stdbool.h>

// GPIO library
#include "bl_gpio.h"

// Timer/Delay
#include "bl_timer.h"

#define MQ2_PIN 5

bool mq2_detect() {
    // Use bl_gpio_input_get_value to read the digital output from the MQ2 sensor
    int value = bl_gpio_input_get_value(MQ2_PIN);
    return value == 0; // Return true if gas is detected (high signal)
}

void mq2_monitor_task() {

    bl_gpio_enable_input(MQ2_PIN, 0, 0);

    while (1) {
        if (mq2_detect()) {
            printf("Gas detected!\n");
        } else {
            printf("No gas detected.\n");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));    // Wait for 1 second before the next reading
    }
}
