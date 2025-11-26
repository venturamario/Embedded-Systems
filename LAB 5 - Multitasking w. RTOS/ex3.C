// /*********************************************************************
//  *  LABORATORIO 5 – APARTADO 3
//  *  2 productores (pares/impares) + 4 consumidores con distribución equitativa
//  *  Una sola cola de 16 elementos
//  *  ESP32-C6 – ESP-IDF v5.5
//  *********************************************************************/

// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/queue.h"

// // Cola única de 16 elementos
// QueueHandle_t xQueue;

// // Contadores para verificar equidad (opcional, para el monitor)
// static uint32_t count_C1 = 0, count_C2 = 0, count_C3 = 0, count_C4 = 0;

// // --------------------- PRODUCTOR PARES (P1) ---------------------
// void producer_even(void *pvParameters)
// {
//     uint32_t value = 0;
//     while (1) {
//         if (xQueueSend(xQueue, &value, portMAX_DELAY) == pdPASS) {
//             // printf("P1 (par) envió: %lu\n", value);
//             value += 2;
//         }
//         vTaskDelay(pdMS_TO_TICKS(1000));  // Ritmo común: 100 ms
//     }
// }

// // --------------------- PRODUCTOR IMPARES (P2) ---------------------
// void producer_odd(void *pvParameters)
// {
//     uint32_t value = 1;
//     vTaskDelay(pdMS_TO_TICKS(500));  // Desfase para alternar con P1
//     while (1) {
//         if (xQueueSend(xQueue, &value, portMAX_DELAY) == pdPASS) {
//             // printf("P2 (impar) envió: %lu\n", value);
//             value += 2;
//         }
//         vTaskDelay(pdMS_TO_TICKS(1000));  // Mismo ritmo
//     }
// }

// // --------------------- CONSUMIDOR IMPARES (C1 y C3) ---------------------
// void consumer_odd(void *pvParameters)
// {
//     uint32_t id = (uint32_t)pvParameters;
//     uint32_t received;

//     while (1) {
//         if (xQueueReceive(xQueue, &received, portMAX_DELAY) == pdPASS) {
//             if (received % 2 == 1) {  // Solo procesar impares
//                 if (id == 1) count_C1++;
//                 else         count_C3++;
//                 printf("C%lu (impar) ← %lu\n", id, received);
//             } else {
//                 // Es par → devolverlo a la cola (al frente)
//                 xQueueSendToFront(xQueue, &received, 0);
//                 // Pequeña espera para dar oportunidad a otros
//                 vTaskDelay(1);
//             }
//         }
//     }
// }

// // --------------------- CONSUMIDOR PARES (C2 y C4) ---------------------
// void consumer_even(void *pvParameters)
// {
//     uint32_t id = (uint32_t)pvParameters;
//     uint32_t received;

//     while (1) {
//         if (xQueueReceive(xQueue, &received, portMAX_DELAY) == pdPASS) {
//             if (received % 2 == 0) {  // Solo procesar pares
//                 if (id == 2) count_C2++;
//                 else         count_C4++;
//                 printf("C%lu (par)   ← %lu\n", id, received);
//             } else {
//                 // Es impar → devolverlo a la cola (al frente)
//                 xQueueSendToFront(xQueue, &received, 0);
//                 vTaskDelay(1);
//             }
//         }
//     }
// }

// // --------------------- APP_MAIN ---------------------
// void app_main(void)
// {
//     printf("\n=== LABORATORIO 5 - APARTADO 3 ===\n");
//     printf("2 productores + 4 consumidores (distribución equitativa)\n");
//     printf("Cola única de 16 elementos\n\n");

//     xQueue = xQueueCreate(16, sizeof(uint32_t));

//     // Prioridades: productores > consumidores
//     const uint8_t PRIORIDAD_PRODUCTORES = 10;
//     const uint8_t PRIORIDAD_CONSUMIDORES = 5;

//     xTaskCreate(producer_even, "P1_Even", 2048, NULL, PRIORIDAD_PRODUCTORES, NULL);
//     xTaskCreate(producer_odd,  "P2_Odd",  2048, NULL, PRIORIDAD_PRODUCTORES, NULL);

//     xTaskCreate(consumer_odd,  "C1_Odd",  2048, (void*)1, PRIORIDAD_CONSUMIDORES, NULL);
//     xTaskCreate(consumer_odd,  "C3_Odd",  2048, (void*)3, PRIORIDAD_CONSUMIDORES, NULL);
//     xTaskCreate(consumer_even, "C2_Even", 2048, (void*)2, PRIORIDAD_CONSUMIDORES, NULL);
//     xTaskCreate(consumer_even, "C4_Even", 2048, (void*)4, PRIORIDAD_CONSUMIDORES, NULL);

//     // Opcional: mostrar contadores cada 5 segundos
//     while (1) {
//         vTaskDelay(pdMS_TO_TICKS(5000));
//         printf("\n--- Contadores (10s) ---\n");
//         printf("C1: %lu | C2: %lu | C3: %lu | C4: %lu\n\n", count_C1, count_C2, count_C3, count_C4);
//     }
// }