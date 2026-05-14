#include "adc_config.h"
#include "esp_log.h"

#define TAG "ADC_CONFIG"

adc_continuous_handle_t adc_handle      = NULL;
adc_cali_handle_t       adc_cali_handle = NULL;
bool                    do_calibration  = false;


bool adc_calibration_init(void)
{
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id  = ADC_UNIT_1,
        .atten    = ADC_ATTEN,
        .bitwidth = ADC_WIDTH,
    };

    if (adc_cali_create_scheme_curve_fitting(&cali_config, &adc_cali_handle) == ESP_OK) {
        //ESP_LOGI(TAG, "Calibración ADC habilitada");
        return true;
    }

    //ESP_LOGW(TAG, "Calibración ADC no soportada, usando conversión lineal");
    return false;
}


void adc_dma_setup(void)
{
    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = 1024,
        .conv_frame_size    = FRAME_SIZE,
    };
    adc_continuous_new_handle(&adc_config, &adc_handle);

    adc_digi_pattern_config_t pattern[2] = {
        {
            .atten     = ADC_ATTEN,
            .channel   = ADC_CHANNEL_LOUDNESS,
            .unit      = ADC_UNIT_1,
            .bit_width = ADC_BITWIDTH_12,
        },
        {
            .atten     = ADC_ATTEN,
            .channel   = ADC_CHANNEL_LIGHT,
            .unit      = ADC_UNIT_1,
            .bit_width = ADC_BITWIDTH_12,
        },
    };

    adc_continuous_config_t dig_cfg = {
        .sample_freq_hz = SAMPLE_FREQ_HZ,
        .conv_mode      = ADC_CONV_SINGLE_UNIT_1,
        .format         = ADC_DIGI_OUTPUT_FORMAT_TYPE2,
        .pattern_num    = 2,
        .adc_pattern    = pattern,
    };

    adc_continuous_config(adc_handle, &dig_cfg);
    adc_continuous_start(adc_handle);
}
