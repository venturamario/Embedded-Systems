/*
    Third lab for the SEU subject at UPC, MEI master.
    Mario Ventura Burgos & Jofre Coll Vila
*/
#pragma region Includes
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_log.h"
#pragma endregion

#pragma region Defines
#define SQUARE_SIZE        500.0f               // Tamaño del cuadrado en unidades del osciloscopio
#define PWM_RESOLUTION     LEDC_TIMER_8_BIT     // Resolución de 8 bits (0-255)
#define MAX_DUTY    ((1 << 8) - 1)              // Duty cycle máximo para 8 bits
#define GPIO_CHANNEL_X  GPIO_NUM_21             // GPIO para el canal X (EJE x)
#define GPIO_CHANNEL_Y  GPIO_NUM_22             // GPIO para el canal Y (EJE y)
#define FRAME_RATE  24                          // Frames por segundo
#define STEPS_PER_SIDE  50                      // Nº de pasos para suavizar el dibujado del cuadrado
#pragma endregion

// Function que convierte coordenadas a valores de duty cycle (0-255)
static inline int convertCoordinateToDuty(float coord) {
    float normalized = coord / SQUARE_SIZE;

    // Mínimo
    if (normalized < 0.0f) {
        normalized = 0.0f;
    }
    // Máximo
    if (normalized > 1.0f) {
        normalized = 1.0f;
    }

    // Convertir a 0-255
    return (int)(normalized * MAX_DUTY);
}

// Function que establece la posición del punto en el osciloscopio
void setPositionXY(int duty_x, int duty_y) {
    // Eje X
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty_x);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

    // Eje Y
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, duty_y);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
}

// Function para dibujar un cuadrado de SQUARE_SIZE x SQUARE_SIZE en el osciloscopio
void drawSquare() {
    // Definir vértices
    float square[4][2] = {
        {0, 0},                          // INFERIOR IZQUIERDO
        {SQUARE_SIZE, 0},                // INFERIOR DERECHO
        {SQUARE_SIZE, SQUARE_SIZE},      // SUPERIOR DERECHO
        {0, SQUARE_SIZE}                 // SUPERIOR IZQUIERDO
    };

    float frame_duration = 1000.0f / FRAME_RATE;   // ms per frame
    float side_duration  = frame_duration / 4.0f;  // ms per side
    float step_delay_ms  = side_duration / STEPS_PER_SIDE;

    // Dibujar lados del cuadrado
    for (int i = 0; i < 4; i++) {
        // Para hacer ciclos de 4 en 4 y saber en qué vértice estamos
        int next = (i + 1) % 4;
        
        // Coordenadas del vértice que se dibuja
        float x0 = square[i][0];
        float y0 = square[i][1];
        float x1 = square[next][0];
        float y1 = square[next][1];

        // Dibujar segmento del cuadrado iterando sobre STEPS_PER_SIDE
        for (int step = 0; step <= STEPS_PER_SIDE; step++) {
            float proporcionAvance = (float)step / STEPS_PER_SIDE;
            float x = x0 + proporcionAvance * (x1 - x0);
            float y = y0 + proporcionAvance * (y1 - y0);
            setPositionXY(convertCoordinateToDuty(x), convertCoordinateToDuty(y));
            vTaskDelay(pdMS_TO_TICKS(step_delay_ms));
        }
    }
}

void app_main(void)
{
    // Usamos una region para poder agruapar la configuracion del PWM y mejorar la legibilidad
    #pragma region PWM Setup
    ledc_timer_config_t ledc_timer = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,
        .duty_resolution = PWM_RESOLUTION,
        .timer_num       = LEDC_TIMER_0,
        .freq_hz         = 250000,
        .clk_cfg         = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ch_x = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LEDC_CHANNEL_0,
        .timer_sel  = LEDC_TIMER_0,
        .gpio_num   = GPIO_CHANNEL_X,
        .duty       = 0
    };
    ledc_channel_config(&ch_x);

    ledc_channel_config_t ch_y = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LEDC_CHANNEL_1,
        .timer_sel  = LEDC_TIMER_0,
        .gpio_num   = GPIO_CHANNEL_Y,
        .duty       = 0
    };
    ledc_channel_config(&ch_y);
    #pragma endregion

    // Main loop en el que se dibuja el cuadrado indefinidamente mediante llamadas a drawSquare()
    ESP_LOGI("MAIN", "Dibujando cuadrado a %.2f FPS...", (float)FRAME_RATE);
    while (1) {
        drawSquare();
    }
}