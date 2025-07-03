#include "pico/stdlib.h"
#include "cli.h"
#include "can_driver.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

int main() {
    stdio_init_all();
    printf("hello world\n");
    sleep_ms(5000);
    printf("hello world2\n");
    can_init();

    xTaskCreate(cli_task, "CLI Task", 1024, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1);
}