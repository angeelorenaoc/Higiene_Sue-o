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
extern int motor_state;
extern int alarm_state;

// ==========================// CERTIFICADOS // ==========================
static const char ca_cert[] = 
"-----BEGIN CERTIFICATE-----\n" \
"MIID/zCCAuegAwIBAgIULhijJblnJITto14srOdld/gKybkwDQYJKoZIhvcNAQEL\n" \
"BQAwgY4xCzAJBgNVBAYTAkNPMRIwEAYDVQQIDAlBbnRpb3F1aWExETAPBgNVBAcM\n" \
"CE1lZGVsbGluMQ0wCwYDVQQKDARVZGVBMQwwCgYDVQQLDANJb1QxEjAQBgNVBAMM\n" \
"CUNBLURhbmllbDEnMCUGCSqGSIb3DQEJARYYZGFuaWVsLmxlb25kQHVkZWEuZWR1\n" \
"LmNvMB4XDTI2MDQyNDIwMjIwOFoXDTI3MDQyNDIwMjIwOFowgY4xCzAJBgNVBAYT\n" \
"AkNPMRIwEAYDVQQIDAlBbnRpb3F1aWExETAPBgNVBAcMCE1lZGVsbGluMQ0wCwYD\n" \
"VQQKDARVZGVBMQwwCgYDVQQLDANJb1QxEjAQBgNVBAMMCUNBLURhbmllbDEnMCUG\n" \
"CSqGSIb3DQEJARYYZGFuaWVsLmxlb25kQHVkZWEuZWR1LmNvMIIBIjANBgkqhkiG\n" \
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEAw3IXiSIemuX+dh6wRoyZ9CeJV6rSBwYRxwQh\n" \
"LcP65j1NwGL3sAmeUdVNPvylAMy5tGtoC8yswyFXnBu6YPqIBcSeuODNjH4z7XI4\n" \
"MGuUkfJOIrR8ugGtrM2zGaelSYSL/ozGEg3o6J9dXDHP9aXrdkEyDyDabv16My1g\n" \
"MLtpqyf2YBJi87Yp7bacVb3BS1AFH0rDay2rnbfIYeticKPUI1N1+995NlYUI2jE\n" \
"95YIt8eE7GC/p3UFlLMAwO6ahQaOavssmAELlLr366YScY66WTC0ORGEAOdtwPuJ\n" \
"3hnzAeQGr356/iLrf74d2G1+VEqjLBuzxDjjCFAj9ALOGBeV7QIDAQABo1MwUTAd\n" \
"BgNVHQ4EFgQUa50PLjrgLi8L48KhE+IKSDK8SE0wHwYDVR0jBBgwFoAUa50PLjrg\n" \
"Li8L48KhE+IKSDK8SE0wDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0BAQsFAAOC\n" \
"AQEACKHillcuU2wZH1JyonSvEWbq6YXPG9Zq04TJl35/nsoF4mEvs1qOXS5UsIl/\n" \
"qs81/gsLdVPiQVawW5Z5ZQoVrtrhcSuZBEmeom9DHGnr7GkMsuI30BVIXNE17wGk\n" \
"Rih0I5bkmjS3xx7DPE3Z2ZXknuHDN5Z26PW+6z9QwZ1xCVJWwwtWKi3XNlLB1sFL\n" \
"wfj4QlmANFZtoB733QNU9GXEVcXTj2PYXhnQXVlbvrpYoq3FXOPspqTiXfF7J9wW\n" \
"TDlfWJMhNCk7Bql/7LR8t1dktZkZfb5kBUPFGa/ArUA7TsmWLh0T7ba5Oys1OA1X\n" \
"ECOXkbjgNwpShwBhO1r4y6enjQ==\n" \
"-----END CERTIFICATE-----\n";

static const char client_cert[] =
"-----BEGIN CERTIFICATE-----\n" \
"MIID8TCCAtmgAwIBAgIUKSBFfNuuVSFR3dnb4CNZ9MFFtPIwDQYJKoZIhvcNAQEL\n" \
"BQAwgY4xCzAJBgNVBAYTAkNPMRIwEAYDVQQIDAlBbnRpb3F1aWExETAPBgNVBAcM\n" \
"CE1lZGVsbGluMQ0wCwYDVQQKDARVZGVBMQwwCgYDVQQLDANJb1QxEjAQBgNVBAMM\n" \
"CUNBLURhbmllbDEnMCUGCSqGSIb3DQEJARYYZGFuaWVsLmxlb25kQHVkZWEuZWR1\n" \
"LmNvMB4XDTI2MDQyNDIzMDcxOFoXDTI3MDQyNDIzMDcxOFowgZExCzAJBgNVBAYT\n" \
"AkNPMRIwEAYDVQQIDAlBbnRpb3F1aWExETAPBgNVBAcMCE1lZGVsbGluMQ0wCwYD\n" \
"VQQKDARVZGVBMQwwCgYDVQQLDANJb1QxFTATBgNVBAMMDEVTUDMyLUNsaWVudDEn\n" \
"MCUGCSqGSIb3DQEJARYYZGFuaWVsLmxlb25kQHVkZWEuZWR1LmNvMIIBIjANBgkq\n" \
"hkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAkQ8tJ8f6dRNK0JoZ17h+4zwvT/mzReZX\n" \
"dyc0lrtAlW7gBAfOUT3SaWddQYzruSwcUUlGBtVQZ1X5v3EjPoZlwHEIIamxJujz\n" \
"Q7/N9IkOwaCXcVtXNxeoABEEWrdtx7pIsVMmwMsFyhV4LLr1aPfWqzeMNJeAQTNW\n" \
"TT9dn7W2+Ka8naJsHsupF9PKkkIP7haSZIPm6Q0VTODrLvvjFs+wa7WzUoyxtAVS\n" \
"mv66/TPq8qFiUkMh+oM3IEXfeewELcaXeCsyXa8kq2O2hhbjX9eOdTMR261cEajl\n" \
"h1CVHxz1BQ4Lgv1qVeWFo6hnM6bOQmjiYA2mL1/b85snzadbfmdUeQIDAQABo0Iw\n" \
"QDAdBgNVHQ4EFgQUjJWuq8qPrkP+7fs9iaCFOJpwepMwHwYDVR0jBBgwFoAUa50P\n" \
"LjrgLi8L48KhE+IKSDK8SE0wDQYJKoZIhvcNAQELBQADggEBAK1Xu1uDwqUuIg+2\n" \
"CuJeqf9azkIID6iVlHBnATM91ISvtNYFHQM9hzwzX0ZKg8Haf2WL9gN61405GsTC\n" \
"sJSp+OKs+2R0x5NLAAZQYCsLdUEIkZ8+m5IvYQzjD17gxr9+emj3wL1yUyq+peaa\n" \
"FW8eDEtjrmSipVv8R6f+lFCVtzUmnsYnwpvw3QJ6Nwvk2N3T/xGvbpSpD9GfJs9p\n" \
"uoIx1YvWRQXoXQ5Ry291u7A/QdOowYcjGKAEHanmWwROq+egAMWVdLM1W0LXnpt1\n" \
"skRUPZ1VMubvtYU+mj1lSZ/Pt44/dTKLkisxLshPSWscrX+Tx2YU5eXinSpKliXC\n" \
"EMINHL0=\n" \
"-----END CERTIFICATE-----\n";

static const char client_key[] =
"-----BEGIN PRIVATE KEY-----\n"
"MIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQCRDy0nx/p1E0rQ\n"
"mhnXuH7jPC9P+bNF5ld3JzSWu0CVbuAEB85RPdJpZ11BjOu5LBxRSUYG1VBnVfm/\n"
"cSM+hmXAcQghqbEm6PNDv830iQ7BoJdxW1c3F6gAEQRat23HukixUybAywXKFXgs\n"
"uvVo99arN4w0l4BBM1ZNP12ftbb4prydomwey6kX08qSQg/uFpJkg+bpDRVM4Osu\n"
"++MWz7BrtbNSjLG0BVKa/rr9M+ryoWJSQyH6gzcgRd957AQtxpd4KzJdrySrY7aG\n"
"FuNf1451MxHbrVwRqOWHUJUfHPUFDguC/WpV5YWjqGczps5CaOJgDaYvX9vzmyfN\n"
"p1t+Z1R5AgMBAAECggEABzdmZgSTLNXb3UrSnve6YEWd45r8KdhkC4pzbvg70J37\n"
"yQI2Z7ZCoc8W8EGOzInUPXr+tSz8MK0x8oMyCf9CT/OPlap3z1lmw2e8Wuj38fay\n"
"R10QQVkM7jaVLTTTjPcd0gHdUK6znV0PZcebQ9s93mCyDz+7jVgnd/T/YbL25Pcb\n"
"VecCS0AkzcsAW9tdoh6X0NIqR+8oH1zKiQ+0KeIVJSgAGOE5062ROG+i2vZAPHNw\n"
"vztTEhXZ3Fdu7Myl5KuCsV8acR0dYATBc6mBSe+QRrxT6Ds5r0UhexEtT2fmYs/M\n"
"AOHrj4CTllSgvdLQ5IQmlbHlnCVWZbKljyJr3gM4PQKBgQDMChqVjGkGR1gzUK5H\n"
"idMchWW5raclmhXloJblA4S9cBy2c72Kxu1yg5j90w0yI6tP6PXe+p1jMea+F3vp\n"
"ZQL7yTnSzmuoGEBVZtTbJFffkEcR96+i0hfhFDA5qVga+fD1qxTT2nyKNe1TYYOU\n"
"SRGPl/OiBJXNeLHZEg5KAmZEKwKBgQC1//3PvxS20M/osSzbgygChRwZ2yayixGl\n"
"Hp2+leCvC2vESxanca6P+Zjv3hVn54LKJY9Z+UGETBMg8o4LcqzIZX2fuCDtARuR\n"
"qq+Y1LO5mguXrWjbavMvuYG+iDsQ84Rnymi/1CcSLjEIyuwgxPBgU0dt0zEVfeRD\n"
"Gliuns7D6wKBgB9gVmI061mZ0whhGcvKd/pk0RXSjGeN4Fwla87f0aPH/8JW3gjf\n"
"OlhnwDkYIWlabtek/VpCs70zSUZmGhnz/Jd3hDrUn12EDTCU1Zq1ZgXtC+DJc+rX\n"
"3/AuyJO1ZOURFmrQ2i0R5iDi1hICZteAvQCV31NE3TxxmPfGfQgIgm1tAoGAXfkK\n"
"5azFFkwlPgHVppZHH135j4Qvq/rQppBzZMfglqFwCI+458Z0GtUG2buzE85fIt9X\n"
"8F4aLSRWsCltnI2el26H+eNJ5PYwdPtL5b7V+dyHZc7dGIZVgOvpCScwUGVclXbW\n"
"tU3myqVdHiwg1h65+xKlDBkC/BTfuqIs7iUDn/cCgYBXNItC7YuT9F4/a9Bxxyex\n"
"DcFWZdZIq4BkfdloCqqwhwpdLBmCcDDMgXlcfKhJ70wunpEE3XKUNtwr5N/AvNvU\n"
"gFwLeFz0Xj/HUAYEbsBiFv97FSow7z0RH8T/jKRDu+zC+TE6Yw8zGxPr82TXaN4/\n"
"z8WdR3g7dmXYCiPJJ8zBWg==\n"
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

int subs_motor(void)
{
    return motor_state;
}

int subs_alarm(void)
{
    return alarm_state;
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
            esp_mqtt_client_subscribe(
                mqtt_client,
                MQTT_TOPIC_MOTOR,
                1);

            esp_mqtt_client_subscribe(
                mqtt_client,
                MQTT_TOPIC_ALARM,
                1);
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

        case MQTT_EVENT_DATA:
        {
            char topic[event->topic_len + 1];
            char payload[event->data_len + 1];

            memcpy(topic, event->topic, event->topic_len);
            topic[event->topic_len] = '\0';

            memcpy(payload, event->data, event->data_len);
            payload[event->data_len] = '\0';

            ESP_LOGI(TAG, "Topic: %s", topic);
            ESP_LOGI(TAG, "Payload: %s", payload);

            if (strcmp(topic, MQTT_TOPIC_ALARM) == 0)
            {
                alarm_state = (atoi(payload) != 0);

                ESP_LOGI(TAG, "Alarm state: %d", alarm_state);
            }

            if (strcmp(topic, MQTT_TOPIC_MOTOR) == 0)
            {
                motor_state = (atoi(payload) != 0);

                ESP_LOGI(TAG, "Motor state: %d", motor_state);
            }
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



