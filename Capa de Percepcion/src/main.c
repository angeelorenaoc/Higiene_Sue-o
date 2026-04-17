#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
 
#include "driver/gpio.h"
#include "esp_log.h"
 
#include "dht11.h"
#include "light_strip.h"
#include "motor.h"
#include "buzzer.h"
#include "server.h"
 
#define TAG           "APP_MAIN"
#define DHT11_PIN     GPIO_NUM_23
#define RTOS_delay(x) vTaskDelay(pdMS_TO_TICKS(x))
 
#include "adc_config.h"
#include "sensor_audio.h"
#include "sensor_light.h"

static QueueHandle_t audioQueue;
static QueueHandle_t lightQueue;
static QueueHandle_t dht_queue;

buzzer_t buzzer = {
    .pin = GPIO_NUM_2,
    .frequency_hz = BUZZER_DEFAULT_FREQ_HZ,
    .is_on = false,
};

static motor_t motor;

static void taskADC(void *pvParameters)
{
    uint8_t      buffer[FRAME_SIZE];
    uint32_t     length = 0;
    adc_sample_t sample;

    int decimation_audio = 0;
    int decimation_light = 0;

    while (1)
    {
        if (adc_continuous_read(adc_handle, buffer, FRAME_SIZE, &length, portMAX_DELAY) == ESP_OK
            && length > 0)
        {
            for (int i = 0; i < (int)length; i += sizeof(adc_digi_output_data_t))
            {
                adc_digi_output_data_t *p = (adc_digi_output_data_t *)&buffer[i];

                if (p->type2.channel == ADC_CHANNEL_LOUDNESS)
                {
                    if (++decimation_audio >= DECIMATION_AUDIO)
                    {
                        sample.value = p->type2.data;
                        xQueueSend(audioQueue, &sample, portMAX_DELAY);
                        decimation_audio = 0;
                    }
                }
                else if (p->type2.channel == ADC_CHANNEL_LIGHT)
                {
                    if (++decimation_light >= DECIMATION_LIGHT)
                    {
                        sample.value = p->type2.data;
                        xQueueSend(lightQueue, &sample, portMAX_DELAY);
                        decimation_light = 0;
                    }
                }
            }
        }
    }
}

//  DHT11 TASK 
static void task_dht(void *arg)
{
    dht11_data_t data;
    while (1)
    {
        esp_err_t err = dht11_read(DHT11_PIN, &data);
        if (err == ESP_OK) {
            xQueueOverwrite(dht_queue, &data);
        } else {
            ESP_LOGI(TAG, "DHT11 error: %s", esp_err_to_name(err));
        }
        RTOS_delay(2000);
    }
}

//  BUZZER TASK
static void task_buzzer(void *arg)
{
    while (1)
    {
        buzzer_set_frequency(&buzzer, 2000);
        buzzer_beep(&buzzer, 200, 150, 3);
        RTOS_delay(4000);

        buzzer_set_frequency(&buzzer, 2800);
        buzzer_beep(&buzzer, 100, 80, 2);
        RTOS_delay(4000);
    }
}
 
//  MOTOR TASK 
void motor_task(void *pvParameters)
{
    bool direction = true;
    const uint32_t duty = 1023;  // velocidad media

    while (1)
    {
        motor_stop(&motor);
        RTOS_delay(200);

        motor_set_direction(&motor, direction);
        motor_set_speed(&motor, duty);

        RTOS_delay(3000);
        direction = !direction;
    }
}
 
 
// SUNRISE TASK
static void task_sunrise(void *arg)
{
    while (1)
    {
        /* 21 pasos × 3000 ms ≈ 63 s de amanecer */
        tira_sunrise_from_array(3000);
        tira_off();
        RTOS_delay(1000);
    }
}
 
 
//  MONITOR TASK 
static void task_monitor(void *arg)
{
    dht11_data_t dht_data = {0};
    while (1)
    {
        if (xQueueReceive(dht_queue, &dht_data, 0) == pdTRUE) {
            ESP_LOGI(TAG, "(T, H) = (%.1f°C, %.1f%%)",
                     dht_data.temperature, dht_data.humidity);
        }
        RTOS_delay(1000);
    }
}
 
// AUDIO TASK
static void taskAudio(void *pvParameters)
{
    adc_sample_t sample;

    while (1)
    {
        if (xQueueReceive(audioQueue, &sample, portMAX_DELAY))
        {
            audio_process_sample(sample);
        }
    }
}


// LIGHT TASK
static void taskLight(void *pvParameters)
{
    adc_sample_t sample;

    while (1)
    {
        if (xQueueReceive(lightQueue, &sample, portMAX_DELAY))
        {
            light_process_sample(sample);
        }
    }
}

// ============================
// TAREA MONITOR IP
// ============================
void ip_monitor_task(void *pvParameters)
{
    esp_netif_ip_info_t ip_info;
    while (1)
    {
        xEventGroupWaitBits(wifi_event_group,
                            WIFI_CONNECTED_BIT,
                            pdFALSE,
                            pdFALSE,
                            portMAX_DELAY);
        if (sta_netif != NULL)
        {
            if (esp_netif_get_ip_info(sta_netif, &ip_info) == ESP_OK)
            {
                ESP_LOGI("NET",
                         "Servidor: http://" IPSTR " | MASK: " IPSTR " | GW: " IPSTR,
                         IP2STR(&ip_info.ip),
                         IP2STR(&ip_info.netmask),
                         IP2STR(&ip_info.gw));
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}


//  Main
void app_main(void)
{
    dht11_init(DHT11_PIN);
    actuadores_init();
    do_calibration = adc_calibration_init();
    adc_dma_setup();
 
    motor_init(&motor, GPIO_NUM_17, GPIO_NUM_19, GPIO_NUM_20);
    buzzer_init(&buzzer);

    audio_init();

    audioQueue = xQueueCreate(200, sizeof(adc_sample_t));
    lightQueue = xQueueCreate(50,  sizeof(adc_sample_t));
 
    dht_queue = xQueueCreate(1, sizeof(dht11_data_t));

    // SERVIDOR HTTP
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        nvs_flash_init();
    }
    wifi_init();
    // Tarea que imprime IP periódicamente
    xTaskCreate(ip_monitor_task, "ip_monitor", 4096, NULL, 4, NULL);
    // Esperar conexión WiFi
    xEventGroupWaitBits(wifi_event_group,
                        WIFI_CONNECTED_BIT,
                        pdFALSE,
                        pdFALSE,
                        portMAX_DELAY);
    ESP_LOGI(TAG, "Iniciando servidor HTTP...");
    start_server();

 
    xTaskCreate(task_dht,"task_dht", 2048, NULL, 4, NULL);
    xTaskCreate(task_monitor, "task_monitor", 2048, NULL, 3, NULL);    
    xTaskCreate(taskADC,   "ADC",   4096, NULL, 3, NULL);
    xTaskCreate(taskAudio, "Audio", 4096, NULL, 2, NULL);
    xTaskCreate(taskLight, "Light", 4096, NULL, 2, NULL);
    
    if (band_system)
    {
        xTaskCreate(task_sunrise, "task_sunrise", 4096, NULL, 4, NULL);
        xTaskCreate(task_buzzer, "Buzzer", 2048, NULL, 4, NULL);
        xTaskCreate(motor_task, "motor_task", 2048, NULL, 5, NULL);
    }

   
}