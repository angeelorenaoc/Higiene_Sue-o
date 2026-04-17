#include "server.h"

#define PIN GPIO_NUM_21

int band_system = 0;
EventGroupHandle_t wifi_event_group = NULL;
esp_netif_t *sta_netif = NULL;
// ============================
// EVENTOS WIFI
// ============================
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
        case WIFI_EVENT_STA_START:
            ESP_LOGI(TAG, "Conectando al WiFi...");
            esp_wifi_connect();
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGW(TAG, "Desconectado, reconectando...");
            xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
            esp_wifi_connect();
            break;
        default:
            break;
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "IP obtenida: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

// ============================
// WIFI INIT
// ============================
void wifi_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    gpio_set_level(PIN, 1);
    wifi_event_group = xEventGroupCreate();
    esp_netif_init();
    esp_event_loop_create_default();
    sta_netif = esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL);
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
    ESP_LOGI(TAG, "WiFi inicializado");
}


// ============================
// HTTP HANDLERS
// ============================

// Página principal
esp_err_t root_get_handler(httpd_req_t *req)
{
    const char *resp =
        "<!DOCTYPE html>"
        "<html><body>"
        "<h1>ESP32 Server</h1>"
        "<p>Servidor funcionando</p>"
        "<a href=\"/status\">Estado</a><br>"
        "<a href=\"/led?state=on\">INICIAR ACTURADORES</a><br>"
        "<a href=\"/led?state=off\">DETENER ACTUADORES</a>"
        "</body></html>";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Estado en JSON
esp_err_t status_handler(httpd_req_t *req)
{
    esp_netif_ip_info_t ip_info;
    esp_netif_get_ip_info(sta_netif, &ip_info);
    char response[200];
    sprintf(response,
            "{ \"ip\": \"" IPSTR "\", \"status\": \"ok\" }",
            IP2STR(&ip_info.ip));
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Control LED (simulado)
esp_err_t led_handler(httpd_req_t *req)
{
    char query[100];
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK)
    {
        char param[10];
        if (httpd_query_key_value(query, "state", param, sizeof(param)) == ESP_OK)
        {
            if (strcmp(param, "on") == 0)
            {
                ESP_LOGI(TAG, "ACTUADORES ON");
                band_system = 1;
            }
            else if (strcmp(param, "off") == 0)
            {
                ESP_LOGI(TAG, "ACTUADORES OFF");
                band_system = 0;
            }
        }
    }
    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// ============================
// SERVIDOR HTTP
// ============================
httpd_handle_t start_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_uri_t root = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = root_get_handler};
        httpd_uri_t status = {
            .uri = "/status",
            .method = HTTP_GET,
            .handler = status_handler};
        httpd_uri_t led = {
            .uri = "/led",
            .method = HTTP_GET,
            .handler = led_handler};
        httpd_register_uri_handler(server, &root);
        httpd_register_uri_handler(server, &status);
        httpd_register_uri_handler(server, &led);
        ESP_LOGI(TAG, "Servidor HTTP iniciado");
    }
    return server;
}