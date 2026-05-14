#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
 
#include "driver/gpio.h"
#include "esp_log.h"
 
#include "dht11.h"
#include "light_strip.h"
#include "motor.h"
#include "buzzer.h"
#include "mqtts.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "mqtt_client.h"
#include "esp_sntp.h"
#include "esp_rom_sys.h"
 
#define TAG1           "APP_MAIN"
#define DHT11_PIN     GPIO_NUM_23
#define RTOS_delay(x) vTaskDelay(pdMS_TO_TICKS(x))
 
#include "adc_config.h"
#include "sensor_audio.h"
#include "sensor_light.h"

static QueueHandle_t audioQueue;
static QueueHandle_t lightQueue;
static QueueHandle_t dht_queue;

static buzzer_t buzzer;
static motor_t motor;

int band_system = false;

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
                        xQueueOverwrite(audioQueue, &sample);
                        decimation_audio = 0;
                    }
                }
                else if (p->type2.channel == ADC_CHANNEL_LIGHT)
                {
                    if (++decimation_light >= DECIMATION_LIGHT)
                    {
                        sample.value = p->type2.data;
                        xQueueOverwrite(lightQueue, &sample);
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
            pub_dht(&data);
        }else {
            ESP_LOGI(TAG1, "DHT11 error: %s", esp_err_to_name(err));
        }
        RTOS_delay(1000);
    }
}


//  BUZZER TASK
static void task_buzzer(void *arg)
{
    while(1)
    {
        if(band_system)
        {
            buzzer_set_frequency(&buzzer, 2000);
            buzzer_beep(&buzzer, 200, 150, 3);
            RTOS_delay(4000);

            buzzer_set_frequency(&buzzer, 2800);
            buzzer_beep(&buzzer, 100, 80, 2);
            RTOS_delay(4000);
        }
        else
        {
            //apagar buzzer
            buzzer_set_frequency(&buzzer, 0);
            buzzer_off(&buzzer);
            RTOS_delay(4000);
        }
    }
}
 
//  MOTOR TASK 
void motor_task(void *pvParameters)
{
    int direction = 1;
    int past_state = band_system;
    int is_on = 0;
    const uint32_t duty = 1023;  // velocidad media

    while (1){

        if(!is_on)
        {
            motor_stop(&motor);
            RTOS_delay(200);
        }
        else if (!band_system)
        {
            motor_stop(&motor);
            RTOS_delay(200);
            motor_set_direction(&motor, direction);
            motor_set_speed(&motor, duty);
            RTOS_delay(3000);
            motor_stop(&motor);
            direction = !direction;
        }else
        {
            motor_stop(&motor);
            RTOS_delay(200);
            motor_set_direction(&motor, direction);
            motor_set_speed(&motor, duty);
            RTOS_delay(3000);
            motor_stop(&motor);
            direction = !direction;
        }
        
        if (past_state == band_system)
        {
            is_on = 0;
        }else
        {
            is_on = 1;
        }
        past_state = band_system;
    }
}
 
// SUNRISE TASK
static void task_sunrise(void *arg)
{
    while(1){
        tira_sunrise_from_array(3000, &band_system);
        RTOS_delay(1000);
    }
}
 
 
 
// AUDIO TASK
static void taskAudio(void *pvParameters)
{
    adc_sample_t sample;

    while (1)
    {
        float audio_value = 0.0f;
        if (xQueueReceive(audioQueue, &sample, portMAX_DELAY))
        {
            audio_value = audio_process_sample(sample);
            pub_audio(audio_value);
            ESP_LOGI(TAG1, "Nivel de ruido: %.2f dB", audio_value);
        }
        RTOS_delay(1000);
    }
}


// LIGHT TASK
static void taskLight(void *pvParameters)
{
    adc_sample_t sample;
    while (1)
    {
        float light_value = 0.0f;

        if (xQueueReceive(lightQueue, &sample, portMAX_DELAY))
        {
            light_value = light_process_sample(sample);
            pub_light(light_value);

            ESP_LOGI(TAG1, "Luz detectada: %.2f %%", light_value);
        }
        RTOS_delay(1000);
    }
}

// TAREA MONITOR IP
/*
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
        vTaskDelay(pdMS_TO_TICKS(6000));
    }
}*/

//  Main
void app_main(void)
{
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    mqtts_init();

    dht11_init(DHT11_PIN);
    actuadores_init();
    do_calibration = adc_calibration_init();
    adc_dma_setup();
 
    motor_init(&motor, GPIO_NUM_17, GPIO_NUM_19, GPIO_NUM_20);
    buzzer_init(&buzzer, GPIO_NUM_2, BUZZER_DEFAULT_FREQ_HZ);

    audio_init();

    audioQueue = xQueueCreate(1, sizeof(adc_sample_t));
    lightQueue = xQueueCreate(1,  sizeof(adc_sample_t));
 
    dht_queue = xQueueCreate(1, sizeof(dht11_data_t));


    xTaskCreate(task_dht,"task_dht", 2048, NULL, 4, NULL);   
    xTaskCreate(taskADC,   "ADC",   4096, NULL, 6, NULL);
    xTaskCreate(taskAudio, "Audio", 4096, NULL, 4, NULL);
    xTaskCreate(taskLight, "Light", 4096, NULL, 4, NULL);
    xTaskCreate(task_sunrise, "task_sunrise", 4096, NULL, 3, NULL);
    xTaskCreate(task_buzzer, "Buzzer", 2048, NULL, 3, NULL);
    xTaskCreate(motor_task, "motor_task", 2048, NULL, 3, NULL);
   
}