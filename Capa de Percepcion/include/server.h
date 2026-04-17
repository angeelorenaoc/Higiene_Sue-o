#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "esp_http_server.h"

#define WIFI_SSID "SOMOS JD ARIAS TORO"
#define WIFI_PASS "1001804091"
#define WIFI_CONNECTED_BIT BIT0

static const char *TAG = "APP";
extern int band_system;

// Evento WiFi
/*static EventGroupHandle_t wifi_event_group;
static esp_netif_t *sta_netif = NULL;
*/
extern EventGroupHandle_t wifi_event_group;
extern esp_netif_t *sta_netif;
void wifi_init(void);
esp_err_t root_get_handler(httpd_req_t *req);
esp_err_t status_handler(httpd_req_t *req);
esp_err_t led_handler(httpd_req_t *req);
httpd_handle_t start_server(void);
