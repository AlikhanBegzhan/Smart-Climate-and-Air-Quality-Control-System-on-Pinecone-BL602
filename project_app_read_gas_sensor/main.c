// FreeRTOS includes
#include <FreeRTOS.h>
#include <task.h>

// Input/Output
#include <stdio.h>

// UART library
#include <bl_uart.h>

extern uint8_t _heap_start;
extern uint8_t _heap_size;
extern uint8_t _heap_wifi_start;
extern uint8_t _heap_wifi_size;

static HeapRegion_t xHeapRegions[] = {
    { &_heap_start, (unsigned int)&_heap_size },
    { &_heap_wifi_start, (unsigned int)&_heap_wifi_size },
    { NULL, 0 }, // End of regions
    { NULL, 0 }
};

// Task configuration
#define SENSOR_MQ2_STACK_SIZE 512
#define SENSOR_MQ2_TASK_PRIORITY 5

void bfl_main(void) {
    // Initialize UART
    bl_uart_init(0, 16, 7, 255, 255, 2 * 1000 * 1000);
    printf("UART Initialized.\n");

    // Define heap regions for FreeRTOS
    vPortDefineHeapRegions(xHeapRegions);

    // Static task variables
    static StackType_t sensor_stack[SENSOR_MQ2_STACK_SIZE];
    static StaticTask_t sensor_task;

    extern void mq2_monitor_task(void *pvParameters);

    // Create the DHT22 task
    xTaskCreateStatic(
        mq2_monitor_task,         // Task function
        "MQ2_Read_Task",       // Task name
        SENSOR_MQ2_STACK_SIZE,       // Stack size
        NULL,                    // Task parameters
        SENSOR_MQ2_TASK_PRIORITY,    // Task priority
        sensor_stack,            // Stack buffer
        &sensor_task             // Static task control block
    );

    // Start the FreeRTOS scheduler
    vTaskStartScheduler();

    // Code should never reach here
    while (1) {
        // Trap in case of scheduler failure
    }
}
