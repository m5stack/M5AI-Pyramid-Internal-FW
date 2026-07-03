/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef _FAN_H_
#define _FAN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"

/**
 * @brief Minimum PWM duty cycle for the fan.
 * @brief 风扇最小PWM占空比。
 */
#define FAN_PWM_MIN (0)

/**
 * @brief Maximum PWM duty cycle for the fan.
 * @brief 风扇最大PWM占空比。
 */
#define FAN_PWM_MAX (100)

/**
 * @brief Maximum timer auto-reload register value.
 * @brief 定时器自动重装载寄存器最大值。
 */
#define FAN_ARR_MAX (6800)

/**
 * @brief Default PWM duty cycle for the fan.
 * @brief 风扇默认PWM占空比。
 */
#define FAN_PWM_DEFAULT (FAN_PWM_MAX)

/**
 * @brief Timeout duration for fan speed measurement in milliseconds.
 * @brief 风扇转速测量超时时间，单位：毫秒。
 */
#define MEASURE_TIMEOUT_MS (50)

/**
 * @brief Timer clock frequency in Hertz.
 * @brief 定时器时钟频率，单位：赫兹。
 */
#define TIMER_CLOCK_HZ (1000000UL)

/**
 * @brief Number of pulses per revolution of the fan.
 * @brief 风扇每转一圈的脉冲数。
 */
#define FAN_PULSE_PER_REV (2)

void fan_init(void);
void set_fan_pwm(uint16_t pwm);
uint16_t get_fan_rpm(void);
bool fan_pwm_check(uint16_t pwm);

#ifdef __cplusplus
}
#endif

#endif /* _FAN_H_ */
