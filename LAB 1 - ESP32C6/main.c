#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define BTN_PIN 0  // Pin del botón
#define LED_PIN 4  // Pin del LED

void clearScreen() {
    // Secuencia ANSI para limpiar la pantalla y mover el cursor a la posición (0,0)
    printf("\033[2J\033[H");
}

void app_main(void)
{
    //Limpiar pantalla
    clearScreen();

    // Configurar el LED como salida
    gpio_config_t led_setup = {
        .pin_bit_mask = (1ULL << LED_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&led_setup);

    // Configurar el botón como entrada con pull-up
    gpio_config_t btn_setup = {
        .pin_bit_mask = (1ULL << BTN_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&btn_setup);

    int led_on = 0;        // Estado del LED (0 = apagado, 1 = encendido)
    int last_btn = 0;      // Último estado del botón

    while (1) {
        int btn_now = gpio_get_level(BTN_PIN);  // Leer estado actual del botón

        // Si sueltas el botón (de 0 a 1), cambiar el LED
        if (btn_now == 1 && last_btn == 0) {
            led_on = !led_on;  // Cambiar estado
            gpio_set_level(LED_PIN, led_on);
            printf("Botón pulsado -> LED %s\n", led_on ? "ON" : "OFF");
        }

        last_btn = btn_now;  // Guardar estado actual

        // Pausa de 50ms para evitar rebotes
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}