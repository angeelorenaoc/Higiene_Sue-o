#include "motor.h"
#include "esp_log.h"

#define TAG "MOTOR"
#define MOTOR_LEDC_MODE       LEDC_LOW_SPEED_MODE
#define MOTOR_LEDC_TIMER      LEDC_TIMER_0
#define MOTOR_LEDC_CHANNEL    LEDC_CHANNEL_0
#define MOTOR_LEDC_FREQ_HZ    5000
#define MOTOR_LEDC_RESOLUTION LEDC_TIMER_10_BIT

void motor_init(motor_t *motor, gpio_num_t pwm_gpio, gpio_num_t in1_gpio, gpio_num_t in2_gpio)
{
    if (motor == NULL) {
        return;
    }

    motor->pwm_gpio  = pwm_gpio;
    motor->in1_gpio  = in1_gpio;
    motor->in2_gpio  = in2_gpio;
    motor->direction = true;

    ledc_timer_config_t timer = {
        .speed_mode      = MOTOR_LEDC_MODE,
        .timer_num       = MOTOR_LEDC_TIMER,
        .duty_resolution = MOTOR_LEDC_RESOLUTION,
        .freq_hz         = MOTOR_LEDC_FREQ_HZ,
        .clk_cfg         = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer);

    ledc_channel_config_t channel = {
        .gpio_num   = motor->pwm_gpio,
        .speed_mode = MOTOR_LEDC_MODE,
        .channel    = MOTOR_LEDC_CHANNEL,
        .timer_sel  = MOTOR_LEDC_TIMER,
        .duty       = 0,
        .hpoint     = 0
    };
    ledc_channel_config(&channel);

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << motor->in1_gpio) | (1ULL << motor->in2_gpio),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    gpio_set_level(motor->in1_gpio, 0);
    gpio_set_level(motor->in2_gpio, 0);
}

void motor_set_direction(motor_t *motor, bool direction)
{
    if (motor == NULL) {
        return;
    }

    motor->direction = direction;

    if (direction) {
        gpio_set_level(motor->in1_gpio, 1);
        gpio_set_level(motor->in2_gpio, 0);
    } else {
        gpio_set_level(motor->in1_gpio, 0);
        gpio_set_level(motor->in2_gpio, 1);
    }
}

void motor_set_speed(motor_t *motor, uint32_t duty)
{
    if (motor == NULL) {
        return;
    }

    if (duty > 1023) {
        duty = 1023;
    }

    ledc_set_duty(MOTOR_LEDC_MODE, MOTOR_LEDC_CHANNEL, duty);
    ledc_update_duty(MOTOR_LEDC_MODE, MOTOR_LEDC_CHANNEL);
}

void motor_stop(motor_t *motor)
{
    if (motor == NULL) {
        return;
    }

    ledc_set_duty(MOTOR_LEDC_MODE, MOTOR_LEDC_CHANNEL, 0);
    ledc_update_duty(MOTOR_LEDC_MODE, MOTOR_LEDC_CHANNEL);

    gpio_set_level(motor->in1_gpio, 0);
    gpio_set_level(motor->in2_gpio, 0);
}

void motor_brake(motor_t *motor)
{
    if (motor == NULL) {
        return;
    }

    ledc_set_duty(MOTOR_LEDC_MODE, MOTOR_LEDC_CHANNEL, 0);
    ledc_update_duty(MOTOR_LEDC_MODE, MOTOR_LEDC_CHANNEL);

    gpio_set_level(motor->in1_gpio, 1);
    gpio_set_level(motor->in2_gpio, 1);
}