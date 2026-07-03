/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#include "fan.h"

static __IO uint32_t crr1 = 0, crr2 = 0, crr3 = 0; /**< 定时器捕获值1、2、3 */
static __IO uint8_t measure_flag = 0;              /**< 测量完成标志 */
static __IO uint32_t freq        = 0;              /**< 计算出的频率 */
static __IO uint32_t duty        = 0;              /**< 占空比百分比 */
static __IO uint32_t fan_freq    = 0;              /**< 风扇转速频率 */
static __IO uint32_t fan_rpm     = 0;              /**< 风扇转速，单位RPM */
static __IO uint8_t measure_cnt  = 1;              /**< 捕获计数，状态机 */

/**
 * @brief Map a value from one range to another.
 * @brief 将输入值从一个范围映射到另一个范围。
 *
 * @param x Input value. 输入值。
 * @param in_min Input range minimum. 输入范围最小值。
 * @param in_max Input range maximum. 输入范围最大值。
 * @param out_min Output range minimum. 输出范围最小值。
 * @param out_max Output range maximum. 输出范围最大值。
 * @return Mapped output value. 映射后的输出值。
 */
static long map(long x, long in_min, long in_max, long out_min, long out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/**
 * @brief Calculate fan frequency, duty cycle, and RPM based on captured timer values.
 * @brief 根据定时器捕获值计算风扇频率、占空比和转速。
 */
static void capture(void)
{
    uint32_t diff1 = 0, diff2 = 0;
    uint32_t max_cnt = __HAL_TIM_GET_AUTORELOAD(&htim2); /**< 获取定时器最大计数值 */

    /* 计算捕获值间的时间差，考虑定时器溢出 */
    if (crr2 >= crr1)
        diff1 = crr2 - crr1;
    else
        diff1 = (max_cnt + 1) + crr2 - crr1;

    if (crr3 >= crr1)
        diff2 = crr3 - crr1;
    else
        diff2 = (max_cnt + 1) + crr3 - crr1;

    /* 防止除零和异常情况 */
    if (diff2 == 0) {
        fan_freq = 0;
        fan_rpm  = 0;
        duty     = 0;
        return;
    }

    freq     = TIMER_CLOCK_HZ / diff2; /**< 计算频率Hz */
    duty     = diff1 * 100 / diff2;    /**< 计算占空比% */
    fan_freq = freq;
    fan_rpm  = freq * (60 / FAN_PULSE_PER_REV); /**< 计算转速RPM */
}

/**
 * @brief Initialize the fan PWM output and set fan off initially.
 * @brief 初始化风扇PWM输出，默认关闭风扇。
 */
void fan_init(void)
{
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);  // Start PWM timer channel
    set_fan_pwm(0);                            // Set PWM to 0, turn off fan
}

/**
 * @brief Set the fan PWM duty cycle.
 * @brief 设置风扇PWM占空比。
 *
 * @param pwm PWM duty cycle (0-100).
 * @param pwm PWM占空比 (0-100)。
 */
void set_fan_pwm(uint16_t pwm)
{
    uint16_t compare = map(pwm, 0, FAN_PWM_MAX, 0, FAN_ARR_MAX); /**< 映射到定时器比较值 */
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, compare);        /**< 设置比较寄存器 */
#ifdef FAN_DEBUG
    printf("set fan pwm: %d, compare: %d\n", pwm, compare);
#endif
}

/**
 * @brief Get the current fan RPM by measuring input capture signals.
 * @brief 通过输入捕获信号测量当前风扇转速(RPM)。
 *
 * @return Fan RPM, or 0xFFFF on timeout.
 * @return 风扇转速，超时返回0xFFFF。
 */
uint16_t get_fan_rpm(void)
{
    uint32_t start = HAL_GetTick(); /**< 获取当前系统时钟计数 */
    crr1 = crr2 = crr3 = 0;
    measure_flag       = 0;

    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1); /**< 启动输入捕获中断 */

    /* 等待测量完成或超时 */
    while (measure_flag == 0) {
        if ((HAL_GetTick() - start) > MEASURE_TIMEOUT_MS) {
            HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_1); /**< 超时停止输入捕获 */
#ifdef FAN_DEBUG
            printf("get fan rpm timeout\n");
#endif
            return 0xFFFF;
        }
    }

    capture(); /**< 计算转速相关参数 */
#ifdef FAN_DEBUG
    printf("get fan rpm: %d\n", fan_rpm);
#endif
    return (uint16_t)fan_rpm;
}

/**
 * @brief Check if the PWM value is within the valid range.
 * @brief 检查PWM值是否在有效范围内。
 *
 * @param pwm PWM duty cycle to check.
 * @param pwm 需要检查的PWM占空比。
 * @return true if valid, false otherwise.
 * @return 有效返回true，无效返回false。
 */
bool fan_pwm_check(uint16_t pwm)
{
    if (pwm > FAN_PWM_MAX) {
        return false;
    }
    return true;
}

/**
 * @brief Timer input capture callback function.
 *        This function is called by HAL on input capture interrupt.
 * @brief 定时器输入捕获中断回调函数，由HAL调用。
 *
 * @param htim Pointer to timer handler.
 * @param htim 定时器句柄指针。
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim2) {
        uint32_t value = HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_1); /**< 读取捕获寄存器值 */
        switch (measure_cnt) {
            case 1:
                crr1 = value;
                __HAL_TIM_SET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_1,
                                              TIM_INPUTCHANNELPOLARITY_FALLING); /**< 设置捕获极性为下降沿 */
                measure_cnt = 2;
                break;

            case 2:
                crr2 = value;
                __HAL_TIM_SET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_1,
                                              TIM_INPUTCHANNELPOLARITY_RISING); /**< 设置捕获极性为上升沿 */
                measure_cnt = 3;
                break;

            case 3:
                crr3 = value;
                HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_1); /**< 停止输入捕获中断 */
                measure_cnt  = 1;
                measure_flag = 1; /**< 标记测量完成 */
                break;

            default:
                HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_1); /**< 异常情况下停止输入捕获中断 */
                measure_cnt = 1;
                break;
        }
    }
}
