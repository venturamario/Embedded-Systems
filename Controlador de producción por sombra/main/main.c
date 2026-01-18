#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "esp_log.h"

static const char *TAG = "CONTADOR";

/* ===================== TU CONFIGURACIÓN ORIGINAL ===================== */
#define LED_EMISOR      GPIO_NUM_4
#define LDR_PIN         ADC1_CHANNEL_0

#define UMBRAL_CAIDA    150
#define VALOR_LUZ_ALTA  1000
#define DEBOUNCE_MS     200

#define AVISO_LENTO_MS  10000
#define AVISO_PARADA_MS 20000

static int contador_total = 0;
static int contador_minuto = 0;

static int ultimo_valor = 0;

static int64_t inicio_minuto = 0;
static int64_t ultima_deteccion_us = 0;
static int64_t ultima_deteccion_ms = 0;

static bool aviso_lento_lanzado = false;
static bool aviso_parada_lanzado = false;

/* ===================== TU FUNCIONES ===================== */
void clearScreen(void)
{
    for (int i = 0; i < 50; i++) {
        printf("\n");
    }
}

void projectPresentation(void)
{
    printf("==================================================================================\n");
    printf("                CONTADOR DE OBJETOS POR SOMBRA (LDR + LED)\n");
    printf("==================================================================================\n\n");
    printf("                  Mario Ventura Burgos & Jofre Coll Vila\n\n");
    printf("  SOBRE EL CIRCUITO:\n");
    printf("  • Pasa objetos frente al sensor LDR para contar.\n");
    printf("  • Se muestra el total acumulado y las unidades por minuto (UPM).\n");
    printf("  • Detección de línea lenta y parada.\n");
    printf("  • Gráfica en tiempo real en http://192.168.4.1\n\n");
}

/* ===================== SERVIDOR WEB ===================== */
httpd_handle_t server = NULL;

static const char *html_page = 
"<!DOCTYPE html>"
"<html><head><title>Contador Produccion</title>"
"<meta name='viewport' content='width=device-width, initial-scale=1'>"
"<style>"
"body{font-family:Arial;text-align:center;background:#f0f0f0;margin:40px;}"
"h1{color:#333;}"
".value{font-size:2em;margin:20px;}"
"#estado{font-weight:bold;}"
"canvas{background:white;border:1px solid #8c8c8c;}"
"</style></head>"
"<body>"
"<h1>Contador de Producción por Sombra</h1>"
"<div class='value'>Total: <span id='total'>0</span></div>"
"<div class='value'>UPM: <span id='upm'>0</span></div>"
"<div class='value'>Estado: <span id='estado'>Normal</span></div>"
"<canvas id='myChart' width='800' height='400'></canvas>"
"<script>"
"const upmHistory = [];"
"const canvas = document.getElementById('myChart');"
"const ctx = canvas.getContext('2d');"
"const w = canvas.width;"
"const h = canvas.height;"
"const pad = 50;"
"const maxPts = 30;"
"function draw() {"
"  ctx.clearRect(0,0,w,h);"
"  ctx.fillStyle = '#f8f8f8'; ctx.fillRect(0,0,w,h);"
"  ctx.strokeStyle = '#000'; ctx.lineWidth = 2;"
"  ctx.beginPath(); ctx.moveTo(pad,pad); ctx.lineTo(pad,h-pad); ctx.lineTo(w-pad,h-pad); ctx.stroke();"
"  ctx.strokeStyle = '#ddd'; ctx.lineWidth = 1;"
"  for(let i=0;i<=5;i++){"
"    let y = pad + i*(h-2*pad)/5;"
"    ctx.beginPath(); ctx.moveTo(pad,y); ctx.lineTo(w-pad,y); ctx.stroke();"
"    ctx.fillStyle = '#000'; ctx.font = '12px Arial'; ctx.textAlign = 'right';"
"    ctx.fillText(Math.round(60*(5-i)/5), pad-10, y+5);"
"  }"
"  if(upmHistory.length>1){"
"    ctx.strokeStyle = 'blue'; ctx.lineWidth = 3;"
"    ctx.beginPath();"
"    for(let i=0;i<upmHistory.length;i++){"
"      let x = pad + i*(w-2*pad)/(maxPts-1);"
"      let y = h - pad - (upmHistory[i]/60)*(h-2*pad);"
"      i===0 ? ctx.moveTo(x,y) : ctx.lineTo(x,y);"
"    }"
"    ctx.stroke();"
"  }"
"}"
"function update(){"
"  fetch('/data').then(r=>r.json()).then(d=>{"
"    document.getElementById('total').innerText=d.total;"
"    document.getElementById('upm').innerText=d.upm;"
"    const est = d.estado==0?'Normal':d.estado==1?'Ralentización':'PARADA';"
"    document.getElementById('estado').innerText=est;"
"    document.getElementById('estado').style.color=d.estado==0?'green':d.estado==1?'orange':'red';"
"    upmHistory.push(d.upm);"
"    if(upmHistory.length>maxPts)upmHistory.shift();"
"    draw();"
"  });"
"}"
"setInterval(update,1000); update();"
"</script>"
"</body></html>";

static esp_err_t root_handler(httpd_req_t *req)
{
    httpd_resp_send(req, html_page, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t data_handler(httpd_req_t *req)
{
    int estado = 0;
    if (aviso_parada_lanzado) estado = 2;
    else if (aviso_lento_lanzado) estado = 1;

    char json[128];
    snprintf(json, sizeof(json),
             "{\"total\":%d,\"upm\":%d,\"estado\":%d}",
             contador_total, contador_minuto, estado);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* Wi-Fi AP y servidor (igual que antes) */
static void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = "ContadorProduccion",
            .ssid_len = strlen("ContadorProduccion"),
            .password = "12345678",
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi AP iniciado");
}

static void start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t root = { .uri = "/", .method = HTTP_GET, .handler = root_handler };
        httpd_uri_t data = { .uri = "/data", .method = HTTP_GET, .handler = data_handler };
        httpd_register_uri_handler(server, &root);
        httpd_register_uri_handler(server, &data);
    }
}

/* ===================== APP MAIN ===================== */
void app_main(void)
{
    wifi_init_softap();
    start_webserver();

    gpio_set_direction(LED_EMISOR, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_EMISOR, 1);

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(LDR_PIN, ADC_ATTEN_DB_11);

    clearScreen();
    projectPresentation();

    printf("Conecta al Wi-Fi 'ContadorProduccion' (pass: 12345678)\n");
    printf("Abre en el navegador: http://192.168.4.1\n\n");

    inicio_minuto = esp_timer_get_time() / 1000000LL;

    while (1) {
        int valor_actual = adc1_get_raw(LDR_PIN);

        int64_t ahora_us = esp_timer_get_time();
        int64_t ahora_ms = ahora_us / 1000;
        int64_t ahora_s  = ahora_us / 1000000LL;

        if (ahora_s - inicio_minuto >= 60) {
            printf("\n------< MINUTO COMPLETADO >------\n");
            printf("Unidades en el último minuto: %d → Throughput: %.2f UPM\n", contador_minuto, (float)contador_minuto);
            printf("Total acumulado: %d\n\n", contador_total);
            printf("-----------------------------------------------------------------\n\n");

            contador_minuto = 0;
            inicio_minuto = ahora_s;
        }

        if (valor_actual < (ultimo_valor - UMBRAL_CAIDA) &&
            ultimo_valor > VALOR_LUZ_ALTA &&
            (ahora_us - ultima_deteccion_us) > (DEBOUNCE_MS * 1000)) {

            contador_total++;
            contador_minuto++;

            ultima_deteccion_us = ahora_us;
            ultima_deteccion_ms = ahora_ms;

            aviso_lento_lanzado = false;
            aviso_parada_lanzado = false;

            printf("¡Objeto detectado! Total: %d | Último minuto: %d\n", contador_total, contador_minuto);
        }

        if (ultima_deteccion_ms != 0) {
            int64_t tiempo_sin_objetos = ahora_ms - ultima_deteccion_ms;

            if (tiempo_sin_objetos >= AVISO_PARADA_MS && !aviso_parada_lanzado) {
                printf(" ⛔⛔ LINEA PARADA (> %lld ms sin detecciones)\n", tiempo_sin_objetos);
                aviso_parada_lanzado = true;
            }
            else if (tiempo_sin_objetos >= AVISO_LENTO_MS && !aviso_lento_lanzado) {
                printf(" ⚠️  LINEA LENTA O RALENTIZADA (> %lld ms sin detecciones)\n", tiempo_sin_objetos);
                aviso_lento_lanzado = true;
            }
        }
        
        ultimo_valor = valor_actual;
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}