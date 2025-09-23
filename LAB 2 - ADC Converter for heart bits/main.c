#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_timer.h"
#include <stdlib.h>

#define THRESHOLD 2380
#define PULSES_PER_FREQUENCY 5

int frequency = 0;
int firstPulseTime = -1;
int pulses = 0;

adc_oneshot_unit_handle_t adc1_handle;

void app_main(void)
{
    // Configure ADC
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
            // Value = 1
            printf("ADC Value: %d -------> PULSO DETECTADO \n", value);
            if (pulses == 0) {
                firstPulseTime = esp_timer_get_time() / 1000;
                pulses++;
                printf("PRIMER PULSO DETECTADO EN TIEMPO %d\n", firstPulseTime);


            } else if (pulses == PULSES_PER_FREQUENCY) {
                int actualTime = esp_timer_get_time() / 1000;
                int timeDiff = abs(actualTime - firstPulseTime);
                frequency = timeDiff/PULSES_PER_FREQUENCY;                
                printf("FRECUENCIA ACTUAL TRAS ULTIMNOS %d: %d ms\n", pulses, frequency);

                pulses = 0;
                frequency = 0; 
                firstPulseTime = -1;

            } else {
                int actualTime = esp_timer_get_time() / 1000;
                firstPulseTime += actualTime;
                pulses++;
            }

        } else {
            // Value = 0
            printf("ADC Value: %d\n", value);
        }

        vTaskDelay(pdMS_TO_TICKS(500)); // Delay for 0'5 seconds
    }
}