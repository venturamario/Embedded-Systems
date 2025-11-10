#include <stdio.h>
#include "driver/twai.h"
#include "esp_log.h"
#include "esp_err.h"

static const char *TAG = "CAN_ECHO";

// === CONFIGURACIÓN DE PINES CAN ===
// Según el pinout del ESP32-C6-DevKitC-1:
// Usa GPIO5 (TX) y GPIO4 (RX), pero puedes cambiarlos si tu montaje usa otros.
#define CAN_TX_GPIO 5
#define CAN_RX_GPIO 4

void app_main(void)
{
    // ==========================
    // CONFIGURACIÓN DEL CONTROLADOR TWAI
    // ==========================

    // Config general: define pines, modo y colas
    twai_general_config_t g_config = {
        .mode = TWAI_MODE_NORMAL,   // Modo normal (no loopback)
        .tx_io = CAN_TX_GPIO,       // GPIO de transmisión CAN
        .rx_io = CAN_RX_GPIO,       // GPIO de recepción CAN
        .clkout_io = TWAI_IO_UNUSED,
        .bus_off_io = TWAI_IO_UNUSED,
        .tx_queue_len = 10,
        .rx_queue_len = 10,
        .alerts_enabled = TWAI_ALERT_NONE,
        .clkout_divider = 0
    };

    // Config de temporización (500 kbps)
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();

    // Filtro: acepta todos los mensajes
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    // ==========================
    // INICIALIZACIÓN
    // ==========================
    esp_err_t res;

    res = twai_driver_install(&g_config, &t_config, &f_config);
    if (res == ESP_OK) {
        ESP_LOGI(TAG, "Driver TWAI instalado correctamente");
    } else {
        ESP_LOGE(TAG, "Error instalando driver TWAI: %s", esp_err_to_name(res));
        return;
    }

    res = twai_start();
    if (res == ESP_OK) {
        ESP_LOGI(TAG, "Bus CAN iniciado en modo NORMAL (500 Kbps)");
    } else {
        ESP_LOGE(TAG, "Error iniciando bus CAN: %s", esp_err_to_name(res));
        twai_driver_uninstall();
        return;
    }

    // ==========================
    // BUCLE PRINCIPAL (ECO)
    // ==========================
    while (true) {
        twai_message_t msg;

        // Esperar mensaje CAN (timeout 1s)
        res = twai_receive(&msg, pdMS_TO_TICKS(1000));
        if (res == ESP_OK) {
            ESP_LOGI(TAG, "Mensaje recibido - ID: 0x%03X, DLC: %d", msg.identifier, msg.data_length_code);

            printf("Datos: ");
            for (int i = 0; i < msg.data_length_code; i++) {
                printf("%02X ", msg.data[i]);
            }
            printf("\n");

            // Reenviar exactamente el mismo mensaje (eco)
            res = twai_transmit(&msg, pdMS_TO_TICKS(1000));
            if (res == ESP_OK) {
                ESP_LOGI(TAG, "Mensaje reenviado correctamente (eco)");
            } else {
                ESP_LOGW(TAG, "Error al reenviar mensaje: %s", esp_err_to_name(res));
            }
        } else if (res == ESP_ERR_TIMEOUT) {
            ESP_LOGW(TAG, "Timeout esperando mensaje CAN...");
        } else {
            ESP_LOGE(TAG, "Error en recepción TWAI: %s", esp_err_to_name(res));
        }
    }

    // (Nunca se alcanza en este ejemplo)
    twai_stop();
    twai_driver_uninstall();
}