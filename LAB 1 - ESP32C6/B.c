#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#define BUTTON_GPIO 0   // GPIO0 -> Pulsador
#define LED_GPIO    4   // GPIO4 -> LED

void app_main(void)
{
    // Configurar el pin del LED como salida
    gpio_config_t io_conf_led = {
        .pin_bit_mask = (1ULL << LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf_led);

    // Configurar el pin del botón como entrada (con pull-up)
    gpio_config_t io_conf_button = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,   // resistencia interna
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf_button);

    int led_state = 0;
    int last_button_state = 1; // pull-up → 1 = sin pulsar
    int button_pressed = 0;
    int64_t press_start_time = 0;

    while (1) {
        int button_state = gpio_get_level(BUTTON_GPIO);

        // Detectar flanco de bajada (inicio de pulsación)
        if (button_state == 0 && last_button_state == 1) {
            press_start_time = esp_timer_get_time(); // tiempo en µs
            button_pressed = 1;
        }

        // Detectar flanco de subida (fin de pulsación)
        if (button_state == 1 && last_button_state == 0) {
            if (button_pressed) {
                int64_t press_duration = esp_timer_get_time() - press_start_time;
                if (press_duration > 500000) { // 500 ms en microsegundos
                    led_state = !led_state;
                    gpio_set_level(LED_GPIO, led_state);
                    printf("Pulsación larga -> LED %s\n", led_state ? "ON" : "OFF");
                } else {
                    printf("Pulsación demasiado corta, ignorada\n");
                }
                button_pressed = 0;
            }
        }

        last_button_state = button_state;
        vTaskDelay(pdMS_TO_TICKS(10)); // pequeña espera
    }
}