// FreeRTOS
#include <FreeRTOS.h>
#include <task.h>

// Input/output
#include <stdio.h>
#include <stdint.h>
#include <bl_uart.h>   

// GPIO library
#include <bl_gpio.h>

// Timer/Delay
#include "bl_timer.h"

//GPIO pin connected to DHT22
#define DHT_PIN 4

// Function to read data from DHT22 sensor
void read_dht22_data() {
    bl_gpio_enable_output(DHT_PIN, 0, 0);
    uint8_t data[5] = {0};

    // Set as output and send start signal
    bl_gpio_output_set(DHT_PIN, 0);
    bl_timer_delay_us(1000); // 1ms
    bl_gpio_output_set(DHT_PIN, 1);
    bl_timer_delay_us(30);  // 30us
    bl_gpio_enable_input(DHT_PIN, 0, 0); // Change to input mode

    // Wait for sensor response
    while (bl_gpio_input_get_value(DHT_PIN) == 1);
    while (bl_gpio_input_get_value(DHT_PIN) == 0);
    while (bl_gpio_input_get_value(DHT_PIN) == 1);

    // Read 40 bits of data
    for (int i = 0; i < 40; i++) {
        while (bl_gpio_input_get_value(DHT_PIN) == 0);  // Wait for the start of bit
        bl_timer_delay_us(30);  // Delay to capture bit value
        if (bl_gpio_input_get_value(DHT_PIN)) {
            data[i / 8] |= (1 << (7 - (i % 8)));  // Set bit if high
        }
        while (bl_gpio_input_get_value(DHT_PIN) == 1);  // Wait for the end of bit
    }

    // Parse and print data
    uint16_t humidity = (data[0] << 8) | data[1];
    uint16_t temperature = (data[2] << 8) | data[3];
    uint8_t checksum = data[4];

    if (checksum == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
        printf("Humidity: %d.%d%%\r\n", humidity / 10, humidity % 10);
        printf("Temperature: %d.%d°C\r\n", temperature / 10, temperature % 10);
    } else {
        printf("Checksum error.\r\n");
    }
}

void dht22_monitor_task(void *pvParameters) {
    while (1) {
        read_dht22_data();
        vTaskDelay(pdMS_TO_TICKS(2000));  // Wait for 2 seconds before the next reading
    }
}


