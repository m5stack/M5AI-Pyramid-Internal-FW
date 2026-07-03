/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "i2c.h"     /**< I2C interface header, I2C接口头文件 */
#include "main.h"    /**< Main application header, 主应用头文件 */
#include "math.h"    /**< Math functions, 数学函数 */
#include "spi.h"     /**< SPI interface header, SPI接口头文件 */
#include "stdbool.h" /**< Boolean type support, 布尔类型支持 */
#include "stdio.h"   /**< Standard IO functions, 标准输入输出函数 */
#include "tim.h"     /**< Timer peripheral header, 定时器外设头文件 */
#include "usart.h"   /**< USART peripheral header, 串口外设头文件 */

// /**
//  * @brief Debug option for power management modules.
//  * @brief 电源管理模块调试开关。
//  */
// #define POWER_DEBUG

// /**
//  * @brief Debug option for flash memory operations.
//  * @brief Flash存储操作调试开关。
//  */
// #define FLASH_DEBUG

// /**
//  * @brief Debug option for fan control modules.
//  * @brief 风扇控制模块调试开关。
//  */
// #define FAN_DEBUG

// /**
//  * @brief Debug option for user UART communication.
//  * @brief 用户串口通信调试开关。
//  */
// #define USER_UART_DEBUG

// /**
//  * @brief Debug option for user I2C communication.
//  * @brief 用户I2C通信调试开关。
//  */
// #define USER_I2C_DEBUG

// /**
//  * @brief Debug option for LCD display functions.
//  * @brief LCD显示功能调试开关。
//  */
// #define LCD_DEBUG

/**
 * @brief Firmware version definition.
 * @brief 固件版本定义。
 */
#define FIRMWARE_VERSION (0x0001)

#ifdef __cplusplus
}
#endif

#endif /* _CONFIG_H_ */
