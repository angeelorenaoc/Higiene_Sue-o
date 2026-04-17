#ifndef BUZZER_H
#define BUZZER_H

#include <stdbool.h>
#include <stdint.h>
#include "driver/gpio.h"
#include "esp_err.h"

typedef struct {
    gpio_num_t pin;
    uint32_t frequency_hz;
    bool is_on;
} buzzer_t;

#define BUZZER_DEFAULT_FREQ_HZ 2000u
#define BUZZER_DUTY_MAX        1023u
#define BUZZER_DUTY_50_PERCENT 512u

void buzzer_init(buzzer_t *b, gpio_num_t pin, uint32_t frequency_hz);
esp_err_t buzzer_on(buzzer_t *b);
esp_err_t buzzer_off(buzzer_t *b);
esp_err_t buzzer_set_frequency(buzzer_t *b, uint32_t frequency_hz);
esp_err_t buzzer_beep(buzzer_t *b, uint32_t time_on_ms, uint32_t time_off_ms, uint32_t repeat_count);

#endif