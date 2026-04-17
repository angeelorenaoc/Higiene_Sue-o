#include "buzzer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/ledc.h"
#include "esp_log.h"

#define TAG "BUZZER"
#define BUZZER_LEDC_MODE    LEDC_LOW_SPEED_MODE
#define BUZZER_LEDC_TIMER   LEDC_TIMER_1
#define BUZZER_LEDC_CHANNEL LEDC_CHANNEL_1

void buzzer_init(buzzer_t *b, gpio_num_t pin, uint32_t frequency_hz)
{
    b->pin = pin;
    b->frequency_hz = frequency_hz;
    b->is_on = false;

    ledc_timer_config_t timer = {
        .speed_mode      = BUZZER_LEDC_MODE,
        .timer_num       = BUZZER_LEDC_TIMER,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz         = (b->frequency_hz == 0) ? BUZZER_DEFAULT_FREQ_HZ : b->frequency_hz,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&timer);

    ledc_channel_config_t channel = {
        .gpio_num   = b->pin,
        .speed_mode = BUZZER_LEDC_MODE,
        .channel    = BUZZER_LEDC_CHANNEL,
        .timer_sel  = BUZZER_LEDC_TIMER,
        .duty       = 0,
        .hpoint     = 0,
    };
    ledc_channel_config(&channel);

    if (b->frequency_hz == 0) {
        b->frequency_hz = BUZZER_DEFAULT_FREQ_HZ;
    }
    b->is_on = false;

    ESP_LOGI(TAG, "Buzzer inicializado (PIN=%d, freq=%lu Hz)",
             b->pin, (unsigned long)b->frequency_hz);
}

esp_err_t buzzer_on(buzzer_t *b)
{
    esp_err_t err = ledc_set_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL, BUZZER_DUTY_50_PERCENT);
    if (err != ESP_OK) return err;

    err = ledc_update_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL);
    if (err == ESP_OK) {
        b->is_on = true;
    }
    return err;
}

esp_err_t buzzer_off(buzzer_t *b)
{
    esp_err_t err = ledc_set_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL, 0);
    if (err != ESP_OK) return err;

    err = ledc_update_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL);
    if (err == ESP_OK) {
        b->is_on = false;
    }
    return err;
}

esp_err_t buzzer_set_frequency(buzzer_t *b, uint32_t frequency_hz)
{
    if (frequency_hz == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = ledc_set_freq(BUZZER_LEDC_MODE, BUZZER_LEDC_TIMER, frequency_hz);
    if (err == ESP_OK) {
        b->frequency_hz = frequency_hz;
    }
    return err;
}

esp_err_t buzzer_beep(buzzer_t *b, uint32_t time_on_ms, uint32_t time_off_ms, uint32_t repeat_count)
{
    for (uint32_t i = 0; i < repeat_count; i++) {
        ESP_ERROR_CHECK(buzzer_on(b));
        vTaskDelay(pdMS_TO_TICKS(time_on_ms));

        ESP_ERROR_CHECK(buzzer_off(b));
        if (i + 1 < repeat_count) {
            vTaskDelay(pdMS_TO_TICKS(time_off_ms));
        }
    }

    return ESP_OK;
}