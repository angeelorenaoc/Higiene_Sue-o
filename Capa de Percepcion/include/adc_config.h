#ifndef ADC_CONFIG_H
#define ADC_CONFIG_H

#include <stdbool.h>
#include "esp_adc/adc_continuous.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#define ADC_CHANNEL_LOUDNESS  ADC_CHANNEL_0
#define ADC_CHANNEL_LIGHT     ADC_CHANNEL_1
#define ADC_ATTEN       ADC_ATTEN_DB_12
#define ADC_WIDTH       ADC_BITWIDTH_12
#define SAMPLE_FREQ_HZ  1000
#define FRAME_SIZE      256

extern adc_continuous_handle_t adc_handle;
extern adc_cali_handle_t       adc_cali_handle;
extern bool                    do_calibration;

typedef struct {
    uint16_t value;
} adc_sample_t;

bool adc_calibration_init(void);
void adc_dma_setup(void);

#endif /* ADC_CONFIG_H */
