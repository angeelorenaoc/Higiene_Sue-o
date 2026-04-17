#include "sensor_audio.h"

#include <math.h>
#include "esp_log.h"

static float s_buffer[RMS_SIZE];
static int   s_index = 0;

static const float CALIB_OFFSET = 78.0f;


void audio_init(void)
{
    s_index = 0;
}


void audio_process_sample(adc_sample_t sample)
{
    float v = (sample.value * 3.3f) / 4095.0f;
    s_buffer[s_index++] = v;

    if (s_index < RMS_SIZE) return;

    /* Offset DC */
    float sum = 0.0f;
    for (int i = 0; i < RMS_SIZE; i++) sum += s_buffer[i];
    float offset = sum / RMS_SIZE;

    /* RMS centrado */
    float sum_sq = 0.0f;
    for (int i = 0; i < RMS_SIZE; i++) {
        float centered = s_buffer[i] - offset;
        sum_sq += centered * centered;
    }

    float rms = sqrtf(sum_sq / RMS_SIZE);
    if (rms < 1e-6f) rms = 1e-6f;

    float db = 20.0f * log10f(rms) + CALIB_OFFSET;

    //ESP_LOGI("AUDIO", "RMS: %.4f V | dB: %.2f", rms, db);
    s_index = 0;
}
