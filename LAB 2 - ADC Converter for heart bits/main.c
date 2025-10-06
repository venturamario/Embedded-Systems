#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_timer.h"
#include <stdlib.h>

#define THRESHOLD 2380
#define PULSES_PER_FREQUENCY 5

int pulses = 0;
int firstPulseTime = -1;

adc_oneshot_unit_handle_t adc1_handle;
bool lastHigh = false;  // Para debouncing

void app_main(void)
{
    // Configurar ADC
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&init_config1, &adc1_handle);

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_11,
    };
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &config);

    while (1) {
        int value = 0;
        adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &value);

        if (value > THRESHOLD) {
            if (!lastHigh) {  // Solo contar el pulso al inicio de la subida
                lastHigh = true;
                pulses++;

                if (pulses == 1) {
                    firstPulseTime = esp_timer_get_time() / 1000; // tiempo en ms
                    printf("Primer pulso detectado en tiempo %d ms\n", firstPulseTime);
                }

                if (pulses == PULSES_PER_FREQUENCY) {
                    int actualTime = esp_timer_get_time() / 1000;
                    int timeDiff = actualTime - firstPulseTime; // ms
                    float frequency_ppm = (float)PULSES_PER_FREQUENCY * 60000 / timeDiff; // pulsos por minuto
                    printf("Frecuencia actual tras %d pulsos: %.2f PPM\n", PULSES_PER_FREQUENCY, frequency_ppm);

                    // Reset para siguiente medición
                    pulses = 0;
                    firstPulseTime = -1;
                }
            }
        } else {
            lastHigh = false;  // Se baja el pulso
        }

        vTaskDelay(pdMS_TO_TICKS(500)); // Delay for 0'5 seconds
    }
}
