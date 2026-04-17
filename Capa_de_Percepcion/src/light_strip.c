#include "light_strip.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "driver/gpio.h"
#include "led_strip.h"

#define TAG             "LIGHT_STRIP"
#define STRIP_GPIO      18
#define STRIP_LED_COUNT 8

static led_strip_handle_t strip = NULL;

static const rgb_color_t sunrise_colors[21] = {
    { 25,   4,   0},
    { 30,   5,   0},
    { 35,   6,   0},
    { 40,   7,   0},
    { 45,   8,   0},
    { 50,   9,   0},
    { 55,  10,   0},
    { 60,  11,   0},
    { 65,  12,   0},
    { 70,  13,   0},
    { 75,  15,   0},
    { 80,  18,   0},
    { 85,  19,   0},
    { 90,  20,   0},
    { 95,  21,   0},
    {100,  22,   0},
    {110,  30,   0},
    {120,  45,   0},
    {125,  65,   0},
    {130,  90,   5},
    {150, 110,  10},
};


esp_err_t actuadores_init(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num         = STRIP_GPIO,
        .max_leds               = STRIP_LED_COUNT,
        .led_model              = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .flags.invert_out       = false,
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src            = RMT_CLK_SRC_DEFAULT,
        .resolution_hz      = 10 * 1000 * 1000,
        .mem_block_symbols  = 64,
        .flags.with_dma     = false,
    };

    esp_err_t err = led_strip_new_rmt_device(&strip_config, &rmt_config, &strip);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "No se pudo inicializar la tira: %s", esp_err_to_name(err));
        return err;
    }

    return tira_off();
}


esp_err_t tira_set_color(uint8_t r, uint8_t g, uint8_t b)
{
    for (int i = 0; i < STRIP_LED_COUNT; i++) {
        esp_err_t err = led_strip_set_pixel(strip, i, r, g, b);
        if (err != ESP_OK) return err;
    }
    return led_strip_refresh(strip);
}


esp_err_t tira_off(void)
{
    esp_err_t err = led_strip_clear(strip);
    if (err != ESP_OK) return err;
    return led_strip_refresh(strip);
}


esp_err_t tira_sunrise_from_array(uint32_t step_ms, int *band_system)
{
    for (int i = 0; i < 21; i++) {
        //ESP_LOGI(TAG, "Estado de la bandera: %d", *band_system);
        if(*band_system){
            tira_set_color(
                sunrise_colors[i].r,
                sunrise_colors[i].g,
                sunrise_colors[i].b
            );
            vTaskDelay(pdMS_TO_TICKS(step_ms));
        }else{
            tira_off();
            vTaskDelay(pdMS_TO_TICKS(step_ms));
            break;
        }
    }
    return ESP_OK;
}
