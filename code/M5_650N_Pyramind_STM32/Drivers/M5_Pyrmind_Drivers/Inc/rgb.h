/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef _RGB_H_
#define _RGB_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"

/**
 * @brief Timing count corresponding to a logic '1' when driving RGB LEDs.
 * @brief RGB灯驱动信号中逻辑“1”的定时计数值
 */
#define BIT_1 (153)

/**
 * @brief Timing count corresponding to a logic '0' when driving RGB LEDs.
 * @brief RGB灯驱动信号中逻辑“0”的定时计数值
 */
#define BIT_0 (51)

/**
 * @brief Total timing period for one RGB LED data transmission.
 * @brief 一个 RGB 灯数据传输的总定时周期
 */
#define RGB_LED_TIMING_MAX (211)

/**
 * @brief Maximum number of RGB LEDs supported.
 * @brief RGB 灯珠的最大数量
 */
#define RGB_NUM_MAX (64)

/**
 * @brief Minimum number of RGB LEDs supported.
 * @brief RGB 灯珠的最小数量
 */
#define RGB_NUM_MIN (1)

/** @brief Default number of RGB LEDs
 *  @brief RGB灯珠默认数量
 */
#define RGB_DEFAULT_NUM (48)

/** @brief Maximum wait time for RGB DMA busy (milliseconds)
 *  @brief RGB输出忙等待的最大时长（毫秒）
 */
#define RGB_DMA_WAIT_TIMEOUT_MS (1)

/** @brief RGB update interval in milliseconds
 *  @brief RGB刷新时间间隔（毫秒）
 */
#define RGB_UPDATE_INTERVAL_MS (25)

/** @brief Number of colors in gradient mode
 *  @brief 渐变模式中的颜色数量
 */
#define RGB_GRADIENT_COLOR_COUNT (6)

/** @brief Maximum steps for gradient transition
 *  @brief 渐变颜色的最大步进值
 */
#define RGB_GRADIENT_STEP_MAX (360)

/** @brief Brightness in gradient mode
 *  @brief 渐变模式的亮度
 */
#define RGB_GRADIENT_BRIGHTNESS (35)

/** @brief Maximum brightness in blink mode
 *  @brief 闪烁模式下的最大亮度
 */
#define RGB_BLINK_BRIGHTNESS_MAX (250)

/** @brief Minimum brightness in blink mode
 *  @brief 闪烁模式下的最小亮度
 */
#define RGB_BLINK_BRIGHTNESS_MIN (20)

/** @brief Tail length in rainflow mode
 *  @brief 流水模式的尾部长度
 */
#define RGB_RAINFLOW_TAIL_LENGTH (20)

/** @brief Tail fading step in rainflow mode
 *  @brief 流水模式尾部颜色的衰减步进
 */
#define RGB_RAINFLOW_TAIL_STEP (50)

/**
 * @brief Number of LEDs in the RGB ring
 * @brief RGB圆环中的灯珠数量
 */
#define RGB_RING_COUNT (12)

/**
 * @brief Number of RGB LED strips
 * @brief RGB灯条的数量
 */
#define RGB_STRIP_COUNT (4)

/**
 * @brief Length of each RGB LED strip
 * @brief 每条RGB灯条的长度
 */
#define RGB_STRIP_LENGTH (9)

/**
 * @brief RGB display mode enumeration
 *
 * Defines the available RGB lighting modes.
 *
 * @brief RGB显示模式枚举
 * 定义可用的RGB灯效模式。
 */
typedef enum {
    RGB_MODE_ARM_MAP   = 0x00, /**< ARM mapping mode ARM映射模式 */
    RGB_MODE_GRADIENT  = 0x01, /**< Gradient mode 渐变模式 */
    RGB_MODE_BLINK     = 0x02, /**< Blink mode 闪烁模式 */
    RGB_MODE_RAINFLOW  = 0x03, /**< Rainflow mode 流水模式 */
    RGB_MODE_DOWNLOAD  = 0x04, /**< Download mode 下载模式 */
    RGB_MODE_RECORDING = 0x05, /**< Recording mode 录制模式 */
    RGB_MODE_OFF       = 0xFF  /**< RGB off mode 关闭RGB */
} rgb_mode_t;

void rgb_init(void);
void rgb_show(void);
void rgb_power_start_indicate(void);
void rgb_power_complete_indicate(void);
bool rgb_num_check(uint16_t num);
bool rgb_mode_check(uint16_t mode);
void set_rgb_arm_change_flag(bool flag);
bool get_rgb_arm_change_flag(void);
void set_rgb_mode_change_flag(bool flag);
bool get_rgb_mode_change_flag(void);
void update_rgb_custom(void);
void update_rgb_arm(void);
void rgb_power_off(void);
void rgb_power_on(void);

#ifdef __cplusplus
}
#endif
#endif /*__ RGB_H__ */
