#ifndef light_strip_h
#define light_strip_h
 
#include <stdint.h>
#include "esp_err.h"
 
/* Color RGB para la tira */
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_color_t;
 
esp_err_t actuadores_init(void);
esp_err_t tira_set_color(uint8_t r, uint8_t g, uint8_t b);
esp_err_t tira_off(void);
esp_err_t tira_sunrise_from_array(uint32_t step_ms);
 
#endif
 