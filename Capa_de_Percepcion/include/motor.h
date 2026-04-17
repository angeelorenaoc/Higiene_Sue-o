#ifndef MOTOR_H
#define MOTOR_H

#include <stdbool.h>
#include <stdint.h>
#include "driver/gpio.h"
#include "driver/ledc.h"

typedef struct {
    gpio_num_t pwm_gpio;
    gpio_num_t in1_gpio;
    gpio_num_t in2_gpio;
    bool direction;
} motor_t;

void motor_init(motor_t *motor, gpio_num_t pwm_gpio, gpio_num_t in1_gpio, gpio_num_t in2_gpio);
void motor_set_direction(motor_t *motor, bool direction);
void motor_set_speed(motor_t *motor, uint32_t duty);
void motor_stop(motor_t *motor);
void motor_brake(motor_t *motor);

#endif