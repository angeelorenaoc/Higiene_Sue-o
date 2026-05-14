#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "esp_timer.h"
#include "rom/ets_sys.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"

#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"

#include "mqtt_client.h"
#include "esp_sntp.h"

#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "dht11.h"

#define MQTT_CLIENT_ID "ESP32C6_DHT11"
#define MQTT_TOPIC_TEMPERATURE     "sensor/temperatura"
#define MQTT_TOPIC_HUMIDITY        "sensor/humedad"
#define MQTT_TOPIC_AUDIO           "sensor/audio"
#define MQTT_TOPIC_LIGHT           "sensor/luz"


// ==========================// PROTOTIPOS DE FUNCIONES// ========================== //

void mqtts_init(void);
static void mqtt_app_start(void);
static void sincronizar_hora_ntp(void);
static void wifi_init_sta(void);
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
bool mqtt_is_connected(void);
void pub_light(float light_value);
void pub_audio(float audio_value);
void pub_dht(dht11_data_t *data);
