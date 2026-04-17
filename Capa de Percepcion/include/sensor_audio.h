#ifndef SENSOR_AUDIO_H
#define SENSOR_AUDIO_H

#include "adc_config.h"

#define RMS_SIZE         20
#define DECIMATION_AUDIO 1


void audio_init(void);
void audio_process_sample(adc_sample_t sample);

#endif /* SENSOR_AUDIO_H */
