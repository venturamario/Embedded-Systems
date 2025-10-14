#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_log.h"

#define SIZE        500.0f
#define PWM_RES     LEDC_TIMER_8_BIT
#define MAX_DUTY    ((1 << 8) - 1)
#define PWM_X_GPIO  GPIO_NUM_21
#define PWM_Y_GPIO  GPIO_NUM_22
#define FRAME_RATE  24
#define STEPS_PER_SIDE  50   // increase for smoother motion

static inline int coord_to_duty(float coord) {
    float normalized = coord / SIZE;
    if (normalized < 0.0f) normalized = 0.0f;
    if (normalized > 1.0f) normalized = 1.0f;
    return (int)(normalized * MAX_DUTY);
}

void set_xy(int duty_x, int duty_y) {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty_x);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, duty_y);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
}

void drawSquare() {
    float square[4][2] = {
        {0,0}, {SIZE,0}, {SIZE,SIZE}, {0,SIZE}
    };

    float frame_duration = 1000.0f / FRAME_RATE;   // ms per frame
    float side_duration  = frame_duration / 4.0f;  // ms per side
    float step_delay_ms  = side_duration / STEPS_PER_SIDE;

    for (int i = 0; i < 4; i++) {
        int next = (i + 1) % 4;

        float x0 = square[i][0];
        float y0 = square[i][1];
        float x1 = square[next][0];
        float y1 = square[next][1];

        for (int step = 0; step <= STEPS_PER_SIDE; step++) {
            float t = (float)step / STEPS_PER_SIDE;
            float x = x0 + t * (x1 - x0);
            float y = y0 + t * (y1 - y0);
            set_xy(coord_to_duty(x), coord_to_duty(y));
            vTaskDelay(pdMS_TO_TICKS(step_delay_ms));
        }
    }
}

void app_main(void)
{
    ledc_timer_config_t ledc_timer = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,
        .duty_resolution = PWM_RES,
        .timer_num       = LEDC_TIMER_0,
        .freq_hz         = 250000,
        .clk_cfg         = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ch_x = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LEDC_CHANNEL_0,
        .timer_sel  = LEDC_TIMER_0,
        .gpio_num   = PWM_X_GPIO,
        .duty       = 0
    };
    ledc_channel_config(&ch_x);

    ledc_channel_config_t ch_y = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LEDC_CHANNEL_1,
        .timer_sel  = LEDC_TIMER_0,
        .gpio_num   = PWM_Y_GPIO,
        .duty       = 0
    };
    ledc_channel_config(&ch_y);

    ESP_LOGI("MAIN", "Dibujando cuadrado a %.2f FPS...", (float)FRAME_RATE);
    while (1) {
        drawSquare();
    }
}