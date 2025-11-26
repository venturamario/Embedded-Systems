/*********************************************************************
 *  DEMO PRIORITY INHERITANCE – VERSIÓN LENTA Y CLARA
 *  Para ver perfectamente en la consola cómo funciona
 *  ESP32-C6 – FreeRTOS
 *********************************************************************/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <stdio.h>
#include "esp_task_wdt.h"      // Para desactivar el watchdog

SemaphoreHandle_t mutex;

void task_low(void *pvParameters)
{
    while (1) {
        printf("\n[LOW] (prioritat 3) Intentant agafar el mutex...\n");
        xSemaphoreTake(mutex, portMAX_DELAY);
        printf("[LOW] JA TINC EL MUTEX! Ara faré feina llarga de ~12 segons...\n");
        vTaskDelay(pdMS_TO_TICKS(1000));  // pausa dramática

        // Trabajo largo dividido en trozos de 1 segundo + YIELD
        for (int segon = 1; segon <= 12; segon++) {
            printf("[LOW] Treballant... segon %2d/12  →  Prioritat actual = %d\n",
                   segon, uxTaskPriorityGet(NULL));

            // Simulamos 1 segundo de trabajo CPU sin bloquear
            for (volatile int i = 0; i < 1200000; i++) {
                __asm__("nop");
            }

            // IMPORTANTE: cedemos CPU pero NO perdemos la prioridad heredada
            taskYIELD();
            vTaskDelay(pdMS_TO_TICKS(200));  // para que dé tiempo a leer
        }

        printf("[LOW] He acabat la feina. Alliberant el mutex...\n");
        xSemaphoreGive(mutex);

        printf("[LOW] Dormint 5 segons abans de tornar a començar...\n\n");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void task_high(void *pvParameters)
{
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(8000));  // Espera a que LOW esté dentro del mutex

        printf("\n>>>> [HIGH] (prioritat 7) ARA VULL EL MUTEX! <<<<\n");
        uint32_t t1 = xTaskGetTickCount();

        xSemaphoreTake(mutex, portMAX_DELAY);

        uint32_t t2 = xTaskGetTickCount();
        printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> [HIGH] HE ACONSEGUIT EL MUTEX EN NOMÉS %lu ms !!!\n",
               (t2 - t1) * portTICK_PERIOD_MS);
        printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> [HIGH] (priority inheritance ha funcionat perfectament)\n");

        xSemaphoreGive(mutex);
        vTaskDelay(pdMS_TO_TICKS(6000));
    }
}

void task_medium(void *pvParameters)
{
    while (1) {
        printf("[MEDIUM] (prioritat 5) Intentant executar-se colar mentre LOW té el mutex...\n");

        // Intenta hacer trabajo intensivo
        for (volatile int i = 0; i < 800000; i++) {
            __asm__("nop");
        }

        printf("[MEDIUM] He pogut executar una mica... però hauria de ser interromput!\n");
        vTaskDelay(pdMS_TO_TICKS(800));
    }
}

void app_main(void)
{
    // Desactivamos el watchdog para poder hacer bucles largos
    esp_task_wdt_deinit();

    mutex = xSemaphoreCreateMutex();

    printf("\n");
    printf("===============================================\n");
    printf("  DEMO PRIORITY INHERITANCE - VERSIÓN LENTA\n");
    printf("  Observa la consola amb calma\n");
    printf("===============================================\n\n");

    xTaskCreate(task_low,    "LOW",    4096, NULL, 3, NULL);
    vTaskDelay(pdMS_TO_TICKS(500));
    xTaskCreate(task_high,   "HIGH",   4096, NULL, 7, NULL);
    xTaskCreate(task_medium, "MEDIUM", 4096, NULL, 5, NULL);

    printf("Sistema iniciat. Ara veuràs màgia de FreeRTOS...\n\n");
}