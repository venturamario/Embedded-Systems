// #include <stdio.h>
// #include <stdbool.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/adc.h"
// #include "driver/gpio.h"
// #include "esp_timer.h"

// /* ===================== DEFINES ===================== */

// #define LED_EMISOR      GPIO_NUM_4
// #define LDR_PIN         ADC1_CHANNEL_0  // GPIO1 (ADC válido)

// #define UMBRAL_CAIDA    150
// #define VALOR_LUZ_ALTA  1000
// #define DEBOUNCE_MS     200

// #define AVISO_LENTO_MS  10000   // 10 segundos
// #define AVISO_PARADA_MS 20000   // 20 segundos

// /* ===================== VARIABLES ===================== */

// static int contador_total = 0;
// static int contador_minuto = 0;

// static int ultimo_valor = 0;

// static int64_t inicio_minuto = 0;        // en segundos
// static int64_t ultima_deteccion_us = 0;  // en microsegundos
// static int64_t ultima_deteccion_ms = 0;  // en milisegundos

// static bool aviso_lento_lanzado = false;
// static bool aviso_parada_lanzado = false;

// /* ===================== FUNCIONES AUX ===================== */

// void clearScreen(void)
// {
//     for (int i = 0; i < 50; i++) {
//         printf("\n");
//     }
// }

// void projectPresentation(void)
// {
//     printf("==================================================================================\n");
//     printf("                CONTADOR DE OBJETOS POR SOMBRA (LDR + LED)\n");
//     printf("==================================================================================\n\n");
//     printf("                  Mario Ventura Burgos & Jofre Coll Vila\n\n");
//     printf("  SOBRE EL CIRCUITO:\n");
//     printf("  • Pasa objetos frente al sensor LDR para contar.\n");
//     printf("  • Se muestra el total acumulado y las unidades por minuto (UPM).\n");
//     printf("  • Detección de línea lenta y parada.\n\n");
// }

// /* ===================== APP MAIN ===================== */

// void app_main(void)
// {
//     /* --- Configuración LED emisor --- */
//     gpio_set_direction(LED_EMISOR, GPIO_MODE_OUTPUT);
//     gpio_set_level(LED_EMISOR, 1);

//     /* --- Configuración ADC --- */
//     adc1_config_width(ADC_WIDTH_BIT_12);
//     adc1_config_channel_atten(LDR_PIN, ADC_ATTEN_DB_11);

//     clearScreen();
//     projectPresentation();

//     inicio_minuto = esp_timer_get_time() / 1000000LL; // segundos

//     while (1) {

//         int valor_actual = adc1_get_raw(LDR_PIN);

//         int64_t ahora_us = esp_timer_get_time();
//         int64_t ahora_ms = ahora_us / 1000;
//         int64_t ahora_s  = ahora_us / 1000000LL;

//         /* ===================== CÁLCULO UPM ===================== */
//         if (ahora_s - inicio_minuto >= 60) {

//             printf("\n------< MINUTO COMPLETADO >------\n");
//             printf("Unidades en el último minuto: %d → Throughput: %.2f UPM\n", contador_minuto, (float)contador_minuto);
//             printf("Total acumulado: %d\n\n", contador_total);
//             printf("-----------------------------------------------------------------\n\n");

//             contador_minuto = 0;
//             inicio_minuto = ahora_s;
//         }

//         /* ===================== DETECCIÓN DE OBJETO ===================== */
//         if (valor_actual < (ultimo_valor - UMBRAL_CAIDA) &&
//             ultimo_valor > VALOR_LUZ_ALTA &&
//             (ahora_us - ultima_deteccion_us) > (DEBOUNCE_MS * 1000)) {

//             contador_total++;
//             contador_minuto++;

//             ultima_deteccion_us = ahora_us;
//             ultima_deteccion_ms = ahora_ms;

//             // Reset de avisos al volver a detectar
//             aviso_lento_lanzado = false;
//             aviso_parada_lanzado = false;

//             printf("¡Objeto detectado! Total: %d | Último minuto: %d\n", contador_total, contador_minuto);
//         }

//         /* ===================== DETECCIÓN DE LÍNEA LENTA / PARADA ===================== */
//         if (ultima_deteccion_ms != 0) {

//             int64_t tiempo_sin_objetos = ahora_ms - ultima_deteccion_ms;

//             if (tiempo_sin_objetos >= AVISO_PARADA_MS && !aviso_parada_lanzado) {
//                 printf(" ");
//                 printf("⛔⛔ LINEA PARADA (> %lld ms sin detecciones)\n", tiempo_sin_objetos);
//                 printf(" ");
//                 aviso_parada_lanzado = true;
//             }
//             else if (tiempo_sin_objetos >= AVISO_LENTO_MS && !aviso_lento_lanzado) {
//                 printf(" ");
//                 printf("⚠️  LINEA LENTA O RALENTIZADA (> %lld ms sin detecciones)\n", tiempo_sin_objetos);
//                 printf(" ");
//                 aviso_lento_lanzado = true;
//             }
//         }
        
//         ultimo_valor = valor_actual;
//         vTaskDelay(pdMS_TO_TICKS(20));  // 50 Hz de muestreo
//     }
// }
