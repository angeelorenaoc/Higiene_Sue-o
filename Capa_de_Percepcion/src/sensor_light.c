#include "sensor_light.h"

#include "esp_log.h"
int count_light = 0;

float light_process_sample(adc_sample_t sample)
{
    float pct;

    if (do_calibration)
    {
        int voltage_mv = 0;
        adc_cali_raw_to_voltage(adc_cali_handle, sample.value, &voltage_mv);

        pct = ((float)voltage_mv / 2300.0f) * 100.0f;
    }
    else
    {
        float v = (sample.value * 3.3f) / 4095.0f;
        pct = (v / 2.3f) * 100.0f;
    }

    if (pct > 100.0f) pct = 100.0f;
    if (pct <   0.0f) pct =   0.0f;

    return pct;
}
