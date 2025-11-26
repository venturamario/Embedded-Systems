// /*********************************************************************
//  *  LABORATORIO 5 – Multitarea con FreeRTOS
//  *  Apartado 2: Intermitencia LED1 (0.3 s) + alternancia LED2/LED3 (1 s)
//  *              Versión con MUTEX y versión con SEMÁFORO BINARIO
//  *
//  *  ESP32-C6 – ESP-IDF v5.5
//  *********************************************************************/

// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/semphr.h"
// #include "driver/gpio.h"

// // ====================== CONFIGURACIÓN DE PINES ======================
// #define LED1    GPIO_NUM_4
// #define LED2    GPIO_NUM_5
// #define LED3    GPIO_NUM_6

// // ====================== OBJETOS DE SINCRONIZACIÓN ======================
// SemaphoreHandle_t xMutex;
// SemaphoreHandle_t xBinarySem;

// // ====================== TAREAS VERSIÓN MUTEX ======================
// void task_led1_mutex(void *pvParameters)
// {
//     while (1) {
//         xSemaphoreTake(xMutex, portMAX_DELAY);
//         gpio_set_level(LED1, 1);
//         xSemaphoreGive(xMutex);
//         vTaskDelay(pdMS_TO_TICKS(300));

//         xSemaphoreTake(xMutex, portMAX_DELAY);
//         gpio_set_level(LED1, 0);
//         xSemaphoreGive(xMutex);
//         vTaskDelay(pdMS_TO_TICKS(300));
//     }
// }

// void task_led23_mutex(void *pvParameters)
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

// // ====================== TAREAS VERSIÓN SEMÁFORO BINARIO ======================
// void task_led1_bin(void *pvParameters)
// {
//     while (1) {
//         if (xSemaphoreTake(xBinarySem, pdMS_TO_TICKS(5)) == pdTRUE) {
//             gpio_set_level(LED1, 1);
//             xSemaphoreGive(xBinarySem);
//         }
//         vTaskDelay(pdMS_TO_TICKS(300));

//         if (xSemaphoreTake(xBinarySem, pdMS_TO_TICKS(5)) == pdTRUE) {
//             gpio_set_level(LED1, 0);
//             xSemaphoreGive(xBinarySem);
//         }
//         vTaskDelay(pdMS_TO_TICKS(300));
//     }
// }

// void task_led23_bin(void *pvParameters)
// {
//     bool state = false;

//     while (1) {
//         if (xSemaphoreTake(xBinarySem, pdMS_TO_TICKS(5)) == pdTRUE) {
//             if (state) {
//                 gpio_set_level(LED2, 0);
//                 gpio_set_level(LED3, 1);
//             } else {
//                 gpio_set_level(LED2, 1);
//                 gpio_set_level(LED3, 0);
//             }
//             state = !state;
//             xSemaphoreGive(xBinarySem);
//         }
//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }
// }

// // ====================== APP_MAIN – ELIGE UNA VERSIÓN ======================
// void app_main(void)
// {
//     // Configuración de los 3 LEDs como salida
//     gpio_reset_pin(LED1);
//     gpio_reset_pin(LED2);
//     gpio_reset_pin(LED3);
//     gpio_set_direction(LED1, GPIO_MODE_OUTPUT);
//     gpio_set_direction(LED2, GPIO_MODE_OUTPUT);
//     gpio_set_direction(LED3, GPIO_MODE_OUTPUT);

//     printf("\n");
//     printf("=======================================================================================\n");
//     printf("  ========================= LABORATORIO 5 - APARTADO 2 ================================\n");
//     printf("  ========================= ESP32-C6 + FreeRTOS =======================================\n");
//     printf("=======================================================================================\n");

//     // ==================== ELEGIR LA VERSIÓN A PROBAR ====================

//     // ---------- VERSIÓN 1: MUTEX ----------
//     // xMutex = xSemaphoreCreateMutex();
//     // if (xMutex != NULL) {
//     //     printf(">>> Ejecutando versión con MUTEX <<<\n");
//     //     xTaskCreate(task_led1_mutex,  "LED1_Mutex",  2048, NULL, 5, NULL);
//     //     xTaskCreate(task_led23_mutex, "LED23_Mutex", 2048, NULL, 5, NULL);
//     // }

//     // ---------- VERSIÓN 2: SEMÁFORO BINARIO ----------
//     xBinarySem = xSemaphoreCreateBinary();
//     if (xBinarySem != NULL) {
//         xSemaphoreGive(xBinarySem);
//         printf(">>> Ejecutando versión con SEMÁFORO BINARIO <<<\n");
//         xTaskCreate(task_led1_bin,  "LED1_Bin",  2048, NULL, 5, NULL);
//         xTaskCreate(task_led23_bin, "LED23_Bin", 2048, NULL, 5, NULL);
//     }

//     printf("Tareas creadas. Observa los LEDs!\n");
// }