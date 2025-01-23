#include <FreeRTOS.h>
#include <task.h>

#include <stdio.h>

#include <bl_dma.h>
#include <bl_gpio.h>
#include <bl_irq.h>
#include <bl_sec.h>
#include <bl_sys_time.h>
#include <bl_uart.h>
#include <hal_boot2.h>
#include <hal_board.h>
#include <hal_hwtimer.h>

#include <blog.h>
#include <lwip/tcpip.h>

#include "wifi.h"

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

// Stack&Priority Configuration
// Wifi
#define WIFI_STACK_SIZE 1024
#define WIFI_TASK_PRIORITY 14
// HTTP Server
#define HTTP_SERVER_STACK_SIZE 2048
#define HTTP_SERVER_TASK_PRIORITY 12
// DHT22
#define SENSOR_DHT22_STACK_SIZE 512
#define SENSOR_DHT22_TASK_PRIORITY 11
// MQ2
#define SENSOR_MQ2_STACK_SIZE 512
#define SENSOR_MQ2_TASK_PRIORITY 11
// Relay
#define RELAY_STACK_SIZE 512
#define RELAY_TASK_PRIORITY 11

void bfl_main(void) {
    // Initialize UART
    bl_uart_init(0, 16, 7, 255, 255, 2 * 1000 * 1000);
    printf("UART Initialized.\n");

    // Define heap regions for FreeRTOS
    vPortDefineHeapRegions(xHeapRegions);

    /* Initialize system */
    blog_init();
    bl_irq_init();
    bl_sec_init();
    bl_dma_init();
    hal_boot2_init();
    hal_board_cfg(0);

    // Static task variables
    // Wifi
    static StackType_t wifi_stack[WIFI_STACK_SIZE];
    static StaticTask_t wifi_task;
    // HTTP Server
    static StackType_t httpd_stack[HTTP_SERVER_STACK_SIZE];
    static StaticTask_t httpd_task;
    // DHT22
    static StackType_t sensor_dht22_stack[SENSOR_DHT22_STACK_SIZE];
    static StaticTask_t sensor_dht22_task;
    // MQ2
    static StackType_t sensor_mq2_stack[SENSOR_MQ2_STACK_SIZE];
    static StaticTask_t sensor_mq2_task;
    //Relay
    static StackType_t relay_stack[RELAY_STACK_SIZE];
    static StaticTask_t relay_task;

    // Tasks creation
    // Wifi Task
    printf("[SYSTEM] Starting WiFi task\r\n");
    extern void task_wifi(void *pvParameters);
    xTaskCreateStatic(task_wifi, (char *)"wifi", WIFI_STACK_SIZE, 
    NULL, WIFI_TASK_PRIORITY, wifi_stack, &wifi_task);
    // HTTP Server Task
    printf("[SYSTEM] Starting httpd task\r\n");
    extern void task_httpd(void *pvParameters);
    xTaskCreateStatic(task_httpd, (char *)"httpd", HTTP_SERVER_STACK_SIZE, 
    NULL, HTTP_SERVER_TASK_PRIORITY, httpd_stack, &httpd_task);
    // DHT22 Task
    extern void dht22_monitor_task();
    xTaskCreateStatic(dht22_monitor_task, "dht22", SENSOR_DHT22_STACK_SIZE, 
    NULL, SENSOR_DHT22_TASK_PRIORITY, sensor_dht22_stack, &sensor_dht22_task);
    // MQ2 Task
    extern void mq2_monitor_task();
    xTaskCreateStatic(mq2_monitor_task, "mq2", SENSOR_MQ2_STACK_SIZE, 
    NULL, SENSOR_MQ2_TASK_PRIORITY, sensor_mq2_stack, &sensor_mq2_task);
    // Relay Task
    extern void relay_control_task();
    xTaskCreateStatic(relay_control_task, "relay", RELAY_STACK_SIZE, 
    NULL, RELAY_TASK_PRIORITY, relay_stack, &relay_task);

    /* Start TCP/IP stack */
    printf("[SYSTEM] Starting TCP/IP stack\r\n");
    tcpip_init(NULL, NULL);
    
    /* Start scheduler */
    printf("[SYSTEM] Starting scheduler\r\n");
    vTaskStartScheduler();
}
