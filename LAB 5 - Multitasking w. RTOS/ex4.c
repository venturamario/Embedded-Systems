// /*********************************************************************
//  *  LABORATORIO 5 – APARTADO 4
//  *  Medición del overhead del scheduler de FreeRTOS (ESP32-C6 @ 160 MHz)
//  *  Basado en el código del apartado 2 (LEDs + mutex)
//  *********************************************************************/

// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/semphr.h"
// #include "driver/gpio.h"
// #include "esp_timer.h"

// #define LED1    GPIO_NUM_4
// #define LED2    GPIO_NUM_5
// #define LED3    GPIO_NUM_6

// SemaphoreHandle_t xMutex;

// // Variables globales para medir overhead
// static uint64_t overhead_min = 999999;
// static uint64_t overhead_max = 0;
// static uint64_t overhead_sum = 0;
// static uint32_t measurements = 0;

// void task_led1(void *pvParameters)
// {
//     while (1) {
//         xSemaphoreTake(xMutex, portMAX_DELAY);
//         gpio_set_level(LED1, 1);
//         xSemaphoreGive(xMutex);
//         vTaskDelay(pdMS_TO_TICKS(300));

//         // === MEDICIÓN DEL OVERHEAD ===
//         uint64_t t1 = esp_timer_get_time();           // Antes del cambio
//         vTaskDelay(1);                                 // Forzamos cambio de contexto
//         uint64_t t2 = esp_timer_get_time();           // Después del cambio

//         uint64_t overhead = t2 - t1;                   // Tiempo total del switch

//         if (overhead < overhead_min) overhead_min = overhead;
//         if (overhead > overhead_max) overhead_max = overhead;
//         overhead_sum += overhead;
//         measurements++;

//         xSemaphoreTake(xMutex, portMAX_DELAY);
//         gpio_set_level(LED1, 0);
//         xSemaphoreGive(xMutex);
//         vTaskDelay(pdMS_TO_TICKS(300));
//     }
// }

// void task_led23(void *pvParameters)
// {
//     bool state = false;
//     while (1) {
//         xSemaphoreTake(xMutex, portMAX_DELAY);
//         if (state) {
//             gpio_set_level(LED2, 0);
//             gpio_set_level(LED3, 1);
//         } else {
//             gpio_set_level(LED2, 1);
//             gpio_set_level(LED3, 0);
//         }
//         state = !state;
//         xSemaphoreGive(xMutex);

//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }
// }

// void task_print_overhead(void *pvParameters)
// {
//     while (1) {
//         vTaskDelay(pdMS_TO_TICKS(5000));  // Cada 5 segundos
//         if (measurements > 0) {
//             double avg = (double)overhead_sum / measurements;
//             printf("\n=== OVERHEAD DEL SCHEDULER (FreeRTOS @ 160 MHz) ===\n");
//             printf("Mínimo : %llu µs\n", overhead_min);
//             printf("Máximo : %llu µs\n", overhead_max);
//             printf("Media  : %.2f µs\n", avg);
//             printf("Muestras: %lu\n", measurements);
//             printf("==============================================\n\n");
//         }
//     }
// }

// void app_main(void)
// {
//     // Configuración GPIOs
//     gpio_reset_pin(LED1); gpio_reset_pin(LED2); gpio_reset_pin(LED3);
//     gpio_set_direction(LED1, GPIO_MODE_OUTPUT);
//     gpio_set_direction(LED2, GPIO_MODE_OUTPUT);
//     gpio_set_direction(LED3, GPIO_MODE_OUTPUT);

//     xMutex = xSemaphoreCreateMutex();

//     printf("\n");
//     printf("==========================================\n");
//     printf("  LABORATORIO 5 - APARTADO 4\n");
//     printf("  Medición de overhead del scheduler\n");
//     printf("  ESP32-C6 @ 160 MHz\n");
//     printf("==========================================\n");

//     // Tareas con misma prioridad para forzar round-robin
//     xTaskCreate(task_led1,        "LED1",      2048, NULL, 5, NULL);
//     xTaskCreate(task_led23,       "LED23",     2048, NULL, 5, NULL);
//     xTaskCreate(task_print_overhead, "PrintOH", 2048, NULL, 4, NULL);  // Prioridad menor
// }