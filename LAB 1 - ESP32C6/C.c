#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#define BUTTON_GPIO 0   // GPIO0 -> Pulsador
#define LED_GPIO    4   // GPIO4 -> LED

typedef enum {
    STATE_NORMAL,
    STATE_BLINKING
} system_state_t;

void app_main(void)
{
    // Configuración LED
    gpio_config_t io_conf_led = {
        .pin_bit_mask = (1ULL << LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf_led);

    // Configuración botón (entrada con pull-up interno)
    gpio_config_t io_conf_button = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf_button);

    int last_button_state = 1; // pull-up → 1 = sin pulsar
    int button_pressed = 0;
    int64_t press_start_time = 0;

    system_state_t state = STATE_NORMAL;
    int led_state = 0;

    int64_t blink_start_time = 0;
    int64_t last_toggle_time = 0;

    while (1) {
        int button_state = gpio_get_level(BUTTON_GPIO);

        // Detectar flanco de bajada (inicio pulsación)
        if (button_state == 0 && last_button_state == 1) {
            press_start_time = esp_timer_get_time();
            button_pressed = 1;
        }

        // Detectar flanco de subida (fin pulsación)
        if (button_state == 1 && last_button_state == 0) {
            if (button_pressed) {
                int64_t press_duration = esp_timer_get_time() - press_start_time;
                if (press_duration > 500000) { // >0.5s
                    if (state == STATE_NORMAL) {
                        // Entrar en modo intermitencia
                        state = STATE_BLINKING;
                        blink_start_time = esp_timer_get_time();
                        last_toggle_time = blink_start_time;
                        printf("Pulsación larga -> INICIO intermitencia\n");
                    } else {
                        // Cancelar intermitencia
                        state = STATE_NORMAL;
                        gpio_set_level(LED_GPIO, 0); // LED apagado
                        printf("Pulsación larga -> CANCELAR intermitencia\n");
                    }
                }
                button_pressed = 0;
            }
        }

        // Lógica de intermitencia
        if (state == STATE_BLINKING) {
            int64_t now = esp_timer_get_time();

            // Cancelar automáticamente tras 10s
            if ((now - blink_start_time) > 10000000) { // 10s en µs
                state = STATE_NORMAL;
                gpio_set_level(LED_GPIO, 0);
                printf("Fin de intermitencia automática\n");
            } else {
                // Alternar LED cada 100ms
                if ((now - last_toggle_time) > 100000) {
                    led_state = !led_state;
                    gpio_set_level(LED_GPIO, led_state);
                    last_toggle_time = now;
                }
            }
        }

        last_button_state = button_state;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}