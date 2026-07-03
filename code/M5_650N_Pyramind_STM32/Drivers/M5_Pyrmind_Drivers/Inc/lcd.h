/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef _LCD_H_
#define _LCD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"

/**
 * @brief Default IP address.
 * @brief 默认 IP 地址
 */
#define LCD_MODE_DEFAULT (0)

/**
 * @brief Interval for switching IP addresses in milliseconds.
 * @brief IP 地址切换间隔（单位：毫秒）
 */
#define IP_SWITCH_INTERVAL_MS (3000)

/**
 * @brief Number of IP addresses to display.
 * @brief 显示的IP地址数量
 */
#define IP_DISPLAY_COUNT (3)

/**
 * @brief Index indicating "IP NULL" display.
 * @brief 表示“IP NULL”显示的索引
 */
#define IP_NULL_INDEX (0xFF)

/**
 * @brief Refresh interval for voltage/current mode in milliseconds.
 * @brief 电压电流模式刷新间隔（单位：毫秒）
 */
#define VOLTAGE_CURRENT_REFRESH_INTERVAL_MS (3000)

/**
 * @brief Number of power display items.
 * @brief 显示的电源项数量
 */
#define POWER_DISPLAY_COUNT (3)

/**
 * @brief PCIE power index.
 * @brief PCIE 电源索引
 */
#define POWER_PCIE (0x00)

/**
 * @brief USB power index.
 * @brief USB 电源索引
 */
#define POWER_USB (0x01)

/**
 * @brief System peripheral power index.
 * @brief 系统外设电源索引
 */
#define POWER_SYS_PER (0x02)

/**
 * @brief Total number of LCD display modes.
 * @brief LCD 显示模式总数
 */
#define LCD_MODE_COUNT (5)

/**
 * @brief Number of columns on the LCD screen.
 * @brief LCD 屏幕列数
 */
#define SCREEN_COLS (16)

/**
 * @brief Number of rows on the LCD screen.
 * @brief LCD 屏幕行数
 */
#define SCREEN_ROWS (2)

/**
 * @brief Total size of the LCD screen in characters.
 * @brief LCD 屏幕总字符数
 */
#define SCREEN_SIZE (SCREEN_COLS * SCREEN_ROWS)

/**
 * @brief LCD display mode enumeration.
 *
 * Defines available modes for LCD content display.
 *
 * @brief LCD 显示模式枚举
 * 定义 LCD 显示内容的模式
 */
typedef enum {
    LCD_MODE_LOG             = 0x00, /**< Log output mode 日志输出模式 */
    LCD_MODE_VOLTAGE_CURRENT = 0x01, /**< Voltage/current display mode 电压电流显示模式 */
    LCD_MODE_IP              = 0x02, /**< IP address display mode IP 地址显示模式 */
    LCD_MODE_ARM_MAP         = 0x03, /**< ARM mapping image display mode ARM 映射图显示模式 */
    LCD_MODE_CHAR            = 0x04  /**< Character display mode 字符显示模式 */
} lcd_mode_t;

extern volatile uint8_t lcd_initialized;

void draw_log(void);
void lcd_init(void);
bool lcd_mode_check(lcd_mode_t mode);
void set_lcd_mode_flag(bool flag);
void set_lcd_arm_flag(bool flag);
void update_lcd(void);
void set_char_update_flag(bool flag);
bool char_check(uint16_t ch);
void buffer_put_char(uint16_t ch);
void lcd_power_off(void);
void lcd_power_on(void);

#ifdef __cplusplus
}
#endif

#endif /* _LCD_H_ */
