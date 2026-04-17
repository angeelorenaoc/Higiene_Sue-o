#ifndef SENSOR_LIGHT_H
#define SENSOR_LIGHT_H

#include "adc_config.h"

#define DECIMATION_LIGHT 20

/**
 * @brief Procesa una muestra cruda del ADC del fotodetector
 *        y la convierte a porcentaje de iluminación.
 *
 * @param sample  Muestra ADC recibida del canal de luz.
 */
void light_process_sample(adc_sample_t sample);

#endif /* SENSOR_LIGHT_H */
