#include "mqtts.h"

// ==========================// CONFIGURACIÓN WIFI// ==========================

#define WIFI_SSID      "Angee"
#define WIFI_PASSWORD  "123456789"

#define WIFI_CONNECTED_BIT BIT0

// ==========================// CONFIGURACIÓN MQTT TLS// ==========================

//#define MQTT_BROKER_URI "mqtt://10.183.49.229:8883"
#define MQTT_BROKER_URI "mqtts://10.44.251.114:8883"
//#define MQTT_BROKER_URI "mqtt://3.124.236.199:1883"

// ==========================// VARIABLES GLOBALES// ==========================
static const char *TAG = "DHT11_MQTT_TLS";
static EventGroupHandle_t wifi_event_group;
static int retry_num = 0;

static esp_mqtt_client_handle_t mqtt_client = NULL;
static bool mqtt_connected = false;


// ==========================// CERTIFICADOS // ==========================
static const char ca_cert[] = 
"-----BEGIN CERTIFICATE-----\n"
"MIIECTCCAvGgAwIBAgIUfgYhIg/H7TAWFQrJgYVdhWYWx8QwDQYJKoZIhvcNAQEL\n"
"BQAwgZMxCzAJBgNVBAYTAkNPMRIwEAYDVQQIDAlBbnRpb3F1aWExETAPBgNVBAcM\n"
"CE1lZGVsbGluMQ0wCwYDVQQKDARVZGVBMQwwCgYDVQQLDANJb1QxFzAVBgNVBAMM\n"
"DkNBLVN3ZWV0RHJlYW1zMScwJQYJKoZIhvcNAQkBFhhkYW5pZWwubGVvbmRAdWRl\n"
"YS5lZHUuY28wHhcNMjYwNTI5MTMwOTI5WhcNMjcwNTI5MTMwOTI5WjCBkzELMAkG\n"
"A1UEBhMCQ08xEjAQBgNVBAgMCUFudGlvcXVpYTERMA8GA1UEBwwITWVkZWxsaW4x\n"
"DTALBgNVBAoMBFVkZUExDDAKBgNVBAsMA0lvVDEXMBUGA1UEAwwOQ0EtU3dlZXRE\n"
"cmVhbXMxJzAlBgkqhkiG9w0BCQEWGGRhbmllbC5sZW9uZEB1ZGVhLmVkdS5jbzCC\n"
"ASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBANXqkL6Kj0IPxtdcNCt/PiaI\n"
"Z1lV3kAHrclWt3IE4qnJB2vMFTw8QX4gDB13GMrVWHjp4ad9Ma7h7P+oA0oPqYIQ\n"
"IhnYwphnoyl8dGA1ctExW8pI6ICqFSuJtkNNtuYpjWamjhSgd7xItH07bnO5aKiz\n"
"00pGgNuu1k3fh1p8PYCwsAc5ENbbqSDtb+x8l6iKLRpBIP7DwjsKpPCkRjxiv6Pc\n"
"2gF1HYwaLD/bwHBDkpHzMqTnJouwhiuUqLoAJMy4fJZh/SIn8BhYyaja0VnBfdB4\n"
"rNjeVSqdCjx9snAlEBlcgdVfNFV2nL6xsYofLzYiMCabauvALs7r51kKrn13HpEC\n"
"AwEAAaNTMFEwHQYDVR0OBBYEFAEix3fE82XSIILb7HiO2qsh97E6MB8GA1UdIwQY\n"
"MBaAFAEix3fE82XSIILb7HiO2qsh97E6MA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZI\n"
"hvcNAQELBQADggEBAIwjnIJykm5hPBky7dnFufswJ2dv3t7xDG2v4V8mke2H6Hej\n"
"MO/RZ3gr/Lqvk4TeLgVOikPoD8FcUFCMChkSXkeS7gl+ob1BgSV6JZeTs3V/2cvK\n"
"cTsnyb+EsUt8gPsM7XtmXo13u6t+PkVmyavHwb1OnwS+FB9cMbXIzEwAoqR5TafU\n"
"ed8kdlwpOY8qmsm3mMaDehMa2gmMB+VZ/Q3fmA1GGM7StU7SCU738laVN15ASQNw\n"
"+11DKqlDlZmC+dzUNTpJSVA4tuYsiz2M5cnDxL8gqUlbbGdJRX6lchb/Vqa9HTlj\n"
"05UJNqmZJIy/f5te1hH737EmtsBNpD668h3tdnc=\n"
"-----END CERTIFICATE-----\n";

static const char client_cert[] =
"-----BEGIN CERTIFICATE-----\n"
"MIID9jCCAt6gAwIBAgIUAI9UQOD4wflEwgKNZFCtCqdE7BYwDQYJKoZIhvcNAQEL\n"
"BQAwgZMxCzAJBgNVBAYTAkNPMRIwEAYDVQQIDAlBbnRpb3F1aWExETAPBgNVBAcM\n"
"CE1lZGVsbGluMQ0wCwYDVQQKDARVZGVBMQwwCgYDVQQLDANJb1QxFzAVBgNVBAMM\n"
"DkNBLVN3ZWV0RHJlYW1zMScwJQYJKoZIhvcNAQkBFhhkYW5pZWwubGVvbmRAdWRl\n"
"YS5lZHUuY28wHhcNMjYwNTI5MTMzMjU4WhcNMjcwNTI5MTMzMjU4WjCBkTELMAkG\n"
"A1UEBhMCQ08xEjAQBgNVBAgMCUFudGlvcXVpYTERMA8GA1UEBwwITWVkZWxsaW4x\n"
"DTALBgNVBAoMBFVkZUExDDAKBgNVBAsMA0lvVDEVMBMGA1UEAwwMRVNQMzItQ2xp\n"
"ZW50MScwJQYJKoZIhvcNAQkBFhhkYW5pZWwubGVvbmRAdWRlYS5lZHUuY28wggEi\n"
"MA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCpkCZF+LxqH5P4q1T8iOXnj9xC\n"
"j01XuD4BFCgc5aAQ7MVbIHSCSvVXJBsPFvq3xw+7Uv4X3wuMTss6sBu1+e+ERSbf\n"
"/qG4WKOJinkQlNRQM+fhUw5dDqjuB6BdwzkVFIarq5ld7gwDAPxQ+nVKVbDhSvDv\n"
"2Js6zn1zbTKX0KTEaUMKxWhQWJxqcTIpBKK9i48Jbht4YP6gLSlQ52pJPJHBZ0PA\n"
"p4fadkr3dRncjcMGNaJko9WU76US8B5XFGkuHdmxRG+Bovxe4cpif3lamwapH6Hv\n"
"gMStNCFy1Dze6ZgJYArxip/JQbYJ2KDATXVL3pFYbX348s86rjJAd5q7gOF1AgMB\n"
"AAGjQjBAMB0GA1UdDgQWBBQPlzo60ok51Frow9hI2CdYedMMljAfBgNVHSMEGDAW\n"
"gBQBIsd3xPNl0iCC2+x4jtqrIfexOjANBgkqhkiG9w0BAQsFAAOCAQEATMlFNY9f\n"
"smgtA4nGx4Yd3IXGIKz3fsma8dhRk1UJ8UuUQ4Ej3NaZptKmtXWkfMIlR+lTSjon\n"
"7iMGf/BZ19Xk55es+Gz9TIFxyOSdD/gqgAI1AVwbQ8jr29hYyk9zN5+RrxOFaonM\n"
"jvcH0NgUMmaoRsuJROQlw7dPjjsER4H9F71u9tU4ZdZBryqvYdJgwIlSO+5zRHGO\n"
"F+7RWzfb7Po6NpC8dgtbbD6uwqnG1IPFgEF7Yk/+bcVCSbNigxJJalMc/Pd2PMuV\n"
"FLv84nSz/DnbWJGI//lLqD+1fqMBSSjp5B9VdzjwAY0gNjXLHiHgXC37tu7YWk+E\n"
"tUdHbOJBDCrtIg==\n"
"-----END CERTIFICATE-----\n";

static const char client_key[] =
"-----BEGIN PRIVATE KEY-----\n"
"MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQCpkCZF+LxqH5P4\n"
"q1T8iOXnj9xCj01XuD4BFCgc5aAQ7MVbIHSCSvVXJBsPFvq3xw+7Uv4X3wuMTss6\n"
"sBu1+e+ERSbf/qG4WKOJinkQlNRQM+fhUw5dDqjuB6BdwzkVFIarq5ld7gwDAPxQ\n"
"+nVKVbDhSvDv2Js6zn1zbTKX0KTEaUMKxWhQWJxqcTIpBKK9i48Jbht4YP6gLSlQ\n"
"52pJPJHBZ0PAp4fadkr3dRncjcMGNaJko9WU76US8B5XFGkuHdmxRG+Bovxe4cpi\n"
"f3lamwapH6HvgMStNCFy1Dze6ZgJYArxip/JQbYJ2KDATXVL3pFYbX348s86rjJA\n"
"d5q7gOF1AgMBAAECggEAHSqfer+7YSHHaSnHd5zwtvEm+Qf/TMxzVeoqsDXLX73q\n"
"gK9HezX7l56tTN4uCDikextee6qxKNRQ3up2CcpRKdZfRn5dQQF2N+1qv7BqCNW0\n"
"2CxaNyWTZ0ZGjnpLMlYSiCr/OYn95PJhdGhwjnXI9CfY49jFcsscByPMZRLNlL8i\n"
"S/pCaOZFFMf5OsLVtZNCoxCD8g27b0om5y7PlRDYPDDARKAaQ4AYo7IU8Rs7JIvh\n"
"l2SyyY52fTTQKxMZwLUznJxFWLKD+Wbzen5AQ1bi1w2oPrxXYxCPmloUiyal+4do\n"
"VFZwd2Cntr9MxhI/O8VH0WekPeR8g96VvUWH10rMbQKBgQDqjqxiKtmRcL30d9af\n"
"TF2Ma6vsDjVULH5AuoPcrPdUxT1rHZ6smS0cErSXgMh53xviE+4e/arvB03FrVBi\n"
"97t6450agrdkSRhOdoeqU0OIrm6HfB6VZ+TYdWN9EJuaysMhajHWGZUelAIfR2DW\n"
"+yvfwdnmUF70qfiqV8GK5ezWVwKBgQC5EGvQDLpWO3bs4Fha9CqqyuxsiOdBPXDF\n"
"oWjIFtg6QlXBWfiNx857iqYEEZlFEfU1iJckhR49pi+FPmQPNaiaNvViX0c7nYs9\n"
"thusBT/Hmoeyo2w4JArBqxTkqFSj1bAaymOxUtBVb4wTe87RLur57IUoTbOo9eBB\n"
"ciQEUyUvEwKBgQCgPiqMwmNzFkffzUNF/yDl/uUsVqSeRetXA1V0nII4WqvUEx9F\n"
"ITyNMkZIPsY3ZHIjKKSEbZ6qwB2kz/vcMQzXLss0wlrR25IMgLO8bDf2F90RGH24\n"
"aosQSlpWpdL4lE9s86TqdP3ILyun/Nau6FeX/VKFIyFMGxb0IRHrd5su7wKBgGuW\n"
"VFc7TwjawSSsuDEISdXI6vGeFXMkGO+MCrJoNYSJE/m4KHukcTH5RNKQuJ4i9n8M\n"
"zXF5wj7s4iuAPgNpmn8s/DMYWP0bdduGh0fWNWIBQGpl/4AT0/0LTLYx9iLZE+w/\n"
"RP3NuzgwBSs+itmpgQmSq0bnBU9wwdLKXI33GGLpAoGBANUHg2tA9bjGRpcVrIAy\n"
"MUTICtmx+B58IZ0bJVMQnnpBivB9a6Q76p1EBeL13aR6cPiAXejkrKt7lePBFAVT\n"
"KREUZ683yFnBifYe8diH3a4fcl41fwjEEerMxQm0UfBREqaXWA9Z6zeJE1UpluaO\n"
"3+VngTWQ9S6hBmiIVBjhDHSE\n"
"-----END PRIVATE KEY-----\n";


bool mqtt_is_connected(void)
{
    return mqtt_connected;
}

void pub_light(float light_value)
{
    if (mqtt_is_connected())
    {
        char light_str[16];

        snprintf(light_str,
                 sizeof(light_str),
                 "l:%.2f",
                 light_value);

        int msg_id_light = esp_mqtt_client_publish(
            mqtt_client,
            MQTT_TOPIC_LIGHT,
            light_str,
            0,
            1,
            0
        );

        ESP_LOGI(TAG, "Luz publicada: %s %% | msg_id=%d", light_str, msg_id_light);
    }
    else
    {
        ESP_LOGW(TAG, "MQTT no conectado. No se publica luz.");
    }
}

void pub_audio(float audio_value)
{
    if (mqtt_is_connected())
    {
        char audio_str[16];

        snprintf(audio_str,
                 sizeof(audio_str),
                 "a:%.2f",
                 audio_value);

        int msg_id_audio = esp_mqtt_client_publish(
            mqtt_client,
            MQTT_TOPIC_AUDIO,
            audio_str,
            0,
            1,
            0
           );

        ESP_LOGI(TAG, "Audio publicado: %s dB | msg_id=%d", audio_str, msg_id_audio);
    } 
    else {
        ESP_LOGW(TAG, "MQTT no conectado. No se publica audio.");
    }
}

void pub_dht(dht11_data_t *data)
{
    if (mqtt_is_connected())
    {
        char tem_str[16];
        char hum_str[16];
        snprintf(tem_str, sizeof(tem_str), "t:%.1f", data->temperature);
        snprintf(hum_str,sizeof(hum_str),"h:%.1f",data->humidity);

        int msg_id = esp_mqtt_client_publish(
            mqtt_client,
            MQTT_TOPIC_TEMPERATURE,
            tem_str,
            0,
            1,
            0
        );

        int msg_h= esp_mqtt_client_publish(
            mqtt_client,
            MQTT_TOPIC_HUMIDITY,
            hum_str,
            0,
            1,
            0
        );
    }
    else
    {
        ESP_LOGW(TAG, "MQTT no conectado. No se publica DHT11.");
    }
}

// ==========================// EVENTOS WIFI// ==========================

static void wifi_event_handler(void *arg,esp_event_base_t event_base,int32_t event_id,void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        ESP_LOGI(TAG, "Iniciando conexión Wi-Fi...");
        esp_wifi_connect();
    }

    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "Wi-Fi desconectado. Reintentando...");

        esp_wifi_connect();
        retry_num++;

        ESP_LOGW(TAG, "Intento de reconexión Wi-Fi número %d", retry_num);
    }

    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;

        ESP_LOGI(TAG, "Wi-Fi conectado");
        ESP_LOGI(TAG, "IP obtenida: " IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "Gateway: " IPSTR, IP2STR(&event->ip_info.gw));
        ESP_LOGI(TAG, "Netmask: " IPSTR, IP2STR(&event->ip_info.netmask));

        esp_netif_dns_info_t dns_info;

        esp_netif_get_dns_info(
            esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"),
            ESP_NETIF_DNS_MAIN,
            &dns_info
        );

        ESP_LOGI(TAG, "DNS principal: " IPSTR, IP2STR(&dns_info.ip.u_addr.ip4));

        retry_num = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }

}

// ==========================// INICIALIZAR WIFI// ==========================

static void wifi_init_sta(void)
{
    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_event_handler,
            NULL,
            NULL
        )
    );

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &wifi_event_handler,
            NULL,
            NULL
        )
    );

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Esperando conexión Wi-Fi...");

    xEventGroupWaitBits(
        wifi_event_group,
        WIFI_CONNECTED_BIT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY
    );

}

// ==========================// SINCRONIZAR HORA NTP// ==========================

static void sincronizar_hora_ntp(void)
{
    ESP_LOGI(TAG, "Configurando SNTP...");

    esp_sntp_stop();

    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);

    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_setservername(1, "time.google.com");
    esp_sntp_setservername(2, "time.cloudflare.com");

    esp_sntp_init();

    time_t now = 0;
    struct tm timeinfo = { 0 };

    int intentos = 0;
    const int max_intentos = 60;

    while (timeinfo.tm_year < (2024 - 1900) && intentos < max_intentos) {
        ESP_LOGI(TAG, "Esperando sincronización NTP... intento %d", intentos + 1);

        vTaskDelay(pdMS_TO_TICKS(1000));

        time(&now);
        localtime_r(&now, &timeinfo);

        intentos++;
    }

    if (timeinfo.tm_year >= (2024 - 1900)) {
        ESP_LOGI(TAG, "Hora sincronizada correctamente");
        ESP_LOGI(TAG, "Fecha actual: %04d-%02d-%02d %02d:%02d:%02d",
                timeinfo.tm_year + 1900,
                timeinfo.tm_mon + 1,
                timeinfo.tm_mday,
                timeinfo.tm_hour,
                timeinfo.tm_min,
                timeinfo.tm_sec);
    } else {
        ESP_LOGE(TAG, "No se pudo sincronizar la hora por NTP");
    }

}

// ==========================// EVENTOS MQTT// ==========================

static void mqtt_event_handler(void *handler_args,esp_event_base_t base,int32_t event_id,void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t) event_id) {

        case MQTT_EVENT_BEFORE_CONNECT:
            ESP_LOGI(TAG, "MQTT: antes de conectar al broker");
            break;

        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "Conectado al broker MQTT por TLS");
            mqtt_connected = true;
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "Desconectado del broker MQTT");
            mqtt_connected = false;
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "Mensaje publicado correctamente. msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "Error en MQTT");

            if (event->error_handle) {
                ESP_LOGE(TAG, "Tipo de error: %d", event->error_handle->error_type);
                ESP_LOGE(TAG, "Error TLS: 0x%x", event->error_handle->esp_tls_last_esp_err);
                ESP_LOGE(TAG, "Error mbedTLS: 0x%x", event->error_handle->esp_tls_stack_err);
                ESP_LOGE(TAG, "Cert verify flags: 0x%x", event->error_handle->esp_tls_cert_verify_flags);
            }

            break;

        default:
            ESP_LOGI(TAG, "Evento MQTT recibido: %ld", event_id);
            break;
    }

}

// ==========================// INICIALIZAR MQTT TLS// ==========================

void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_config = {

        .broker.address.uri = MQTT_BROKER_URI,

        // Servidor MQTT
        .broker.verification.certificate = ca_cert,

        // Certificado del cliente 
        .credentials.authentication.certificate = client_cert,

        // Clave privada ESP32
        .credentials.authentication.key = client_key,

        .credentials.client_id = MQTT_CLIENT_ID,

        .session.protocol_ver = MQTT_PROTOCOL_V_3_1_1,
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_config);

    ESP_ERROR_CHECK(
        esp_mqtt_client_register_event(
            mqtt_client,
            ESP_EVENT_ANY_ID,
            mqtt_event_handler,
            NULL
        )
    );

    ESP_ERROR_CHECK(
        esp_mqtt_client_start(mqtt_client)
    );

    ESP_LOGI(TAG, "Cliente MQTT TLS iniciado");
}

void mqtts_init(void)
{
    wifi_init_sta();

    setenv("TZ", "COT5", 1);
    tzset();

    sincronizar_hora_ntp();

    mqtt_app_start();
}



