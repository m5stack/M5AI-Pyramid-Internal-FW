/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef _DEMO_H_
#define _DEMO_H_

#ifdef __cplusplus

extern "C" {
#endif

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "stdio.h"
#include "config.h"
#include "rgb.h"
#include "ina3221.h"
#include "fan.h"
#include "power.h"
#include "usart.h"
#include <string.h>
#include "ip.h"
#include "lcd.h"
#include "flash.h"
#include "button.h"

/* ----------------------- Def44ines ------------------------------------------*/
// 线圈寄存器基地址和数量
#define MB_COIL_COUNT (18)

// 离散输入寄存器基地址和数量
#define MB_DISCRETE_INPUT_COUNT (14)

// 输入寄存器基地址和数量
#define MB_INPUT_REG_COUNT (14)

// 保持寄存器基地址和数量
#define MB_HOLDING_REG_START_OFFSET (1)     // Holding register start address
#define MB_HOLDING_REG_COUNT        (1024)  // Total number of holding registers

// 线圈寄存器定义
#define MB_COIL_EXT_5V_SUPPLY      (1)   // [RW] 外设5V供电 (PA11)
#define MB_COIL_INT_5V_MAIN_POWER  (2)   // [RW] 内部5V主电源，写0时延时关闭 (PA15)
#define MB_COIL_CLK_MODE_SELECT    (3)   // [RW] 时钟输出模式，0=随PCIE上电，1=手动 (NC)
#define MB_COIL_PCIE_CLK_OUTPUT    (4)   // [RW] NGFF PCIE时钟输出 (PB10)
#define MB_COIL_PCIE1_POWER        (5)   // [RW] PCIE1供电 (PB2)
#define MB_COIL_PCIE2_POWER        (6)   // [RW] PCIE2供电 (PB4)
#define MB_COIL_GL3510_RESET       (7)   // [RW] GL3510复位 (PB11)
#define MB_COIL_DS1_HIGH_CURRENT   (8)   // [RW] DS1口大电流模式 (PB12)
#define MB_COIL_DS2_HIGH_CURRENT   (9)   // [RW] DS2口大电流模式 (PB15)
#define MB_COIL_DS1_POWER          (10)  // [RW] DS1口电源 (PB13)
#define MB_COIL_DS2_POWER          (11)  // [RW] DS2口电源 (PB14)
#define MB_COIL_DS3_POWER          (12)  // [RW] DS3口电源 (PC11)
#define MB_COIL_GROVE_PORT2_SUPPLY (13)  // [RW] Grove Port2供电 (PC15)
#define MB_COIL_GROVE_PORT3_SUPPLY (14)  // [RW] Grove Port3供电 (PB7)
#define MB_COIL_SAVE_COIL_CONFIG   (15)  // [WO] 保存相关配置 (NC)
#define MB_COIL_SAVE_HOLD_CONFIG   (16)  // [WO] 保存相关配置 (NC)
#define MB_COIL_POWER_OFF          (17)  // [WO] 关闭电源 (NC)
#define MB_COIL_SYSTEM_READY       (18)  // [WO] 系统就绪标志位（特殊寄存器，只写。）
// 离散输入寄存器定义
#define MB_DISCRETE_INPUT_PCIE1_PRESENT  (1)   // [RO] PCIE1 设备存在信号 (PA12)
#define MB_DISCRETE_INPUT_PCIE2_PRESENT  (2)   // [RO] PCIE2 设备存在信号 (PB3)
#define MB_DISCRETE_INPUT_3V3_POWER_GOOD (3)   // [RO] 3.3V PowerGood反馈 (PB6)
#define MB_DISCRETE_INPUT_1V8_POWER_GOOD (4)   // [RO] 1.8V PowerGood反馈 (PF1)
#define MB_DISCRETE_INPUT_TOP_BUTTON     (5)   // [RO] 顶部按钮输入 (PC10)
#define MB_DISCRETE_INPUT_TOP_FALL       (6)   // [RO] 顶部按钮下降沿事件
#define MB_DISCRETE_INPUT_TOP_RISE       (7)   // [RO] 顶部按钮上升沿事件
#define MB_DISCRETE_INPUT_TOP_LONG       (8)   // [RO] 顶部按钮长按事件
#define MB_DISCRETE_INPUT_LCD_BUTTON     (9)   // [RO] LCD按钮输入 (PC6)
#define MB_DISCRETE_INPUT_LCD_FALL       (10)  // [RO] LCD按钮下降沿事件
#define MB_DISCRETE_INPUT_LCD_RISE       (11)  // [RO] LCD按钮上升沿事件
#define MB_DISCRETE_INPUT_LCD_LONG       (12)  // [RO] LCD按钮长按事件
#define MB_DISCRETE_INPUT_LCD_STATUS     (13)  // [RO] LCD状态1表示正常、0表示异常
#define MB_DISCRETE_INPUT_BTN_POWER_OFF  (14)  // [RO] 按键关机状态，为1表明按键关机触发，定部按钮按压15s以上触发，关机之后该寄存器清0

// 输入寄存器定义
#define MB_INPUT_REG_PCIE1_VOLTAGE      (1)   // [RO] PCIE1 电压
#define MB_INPUT_REG_PCIE1_CURRENT      (2)   // [RO] PCIE1 电流
#define MB_INPUT_REG_PCIE2_VOLTAGE      (3)   // [RO] PCIE2 电压
#define MB_INPUT_REG_PCIE2_CURRENT      (4)   // [RO] PCIE2 电流
#define MB_INPUT_REG_USB1_VOLTAGE       (5)   // [RO] USB1 电压
#define MB_INPUT_REG_USB1_CURRENT       (6)   // [RO] USB1 电流
#define MB_INPUT_REG_USB2_VOLTAGE       (7)   // [RO] USB2 电压
#define MB_INPUT_REG_USB2_CURRENT       (8)   // [RO] USB2 电流
#define MB_INPUT_REG_INTERNAL5V_VOLTAGE (9)   // [RO] 内部5V 电压
#define MB_INPUT_REG_INTERNAL5V_CURRENT (10)  // [RO] 内部5V 电流
#define MB_INPUT_REG_EXTERNAL5V_VOLTAGE (11)  // [RO] 外部5V 电压
#define MB_INPUT_REG_EXTERNAL5V_CURRENT (12)  // [RO] 外部5V 电流
#define MB_INPUT_REG_FAN_SPEED          (13)  // [RO] 风扇实际转速
#define MB_INPUT_REG_FIRMWARE_VERSION   (14)  // [RO] 固件版本

// 保持寄存器定义
#define MB_HOLD_REG_MAIN_POWER_DELAY  (1)    // [RW] 主电源延时关闭时间
#define MB_HOLD_REG_FAN_SPEED_PERCENT (2)    // [RW] 风扇转速百分比
#define MB_HOLD_REG_VDD_CPU_OFFSET    (3)    // [RW] VDD_CPU 电压
#define MB_HOLD_REG_UART_BAUD_HIGH    (4)    // [RW] 串口波特率高字节
#define MB_HOLD_REG_UART_BAUD_LOW     (5)    // [RW] 串口波特率低字节
#define MB_HOLD_REG_WAKEUP_TIME_HIGH  (6)    // [RW] 定时唤醒时间高字节(单位：秒)
#define MB_HOLD_REG_WAKEUP_TIME_LOW   (7)    // [RW] 定时唤醒时间低字节(单位：秒)
#define MB_HOLD_REG_I2C_ADDR          (8)    // [RW] I2C地址寄存器（高字节地址/低字节寄存器）
#define MB_HOLD_REG_I2C_READ          (9)    // [RW] I2C读命令寄存器（操作后清除）
#define MB_HOLD_REG_I2C_WRITE         (10)   // [RW] I2C写命令寄存器（操作后清除）
#define MB_HOLD_REG_RGB_COUNT         (11)   // [RW] RGB灯数量
#define MB_HOLD_REG_RGB_MODE          (12)   // [RW] RGB灯模式
#define MB_HOLD_REG_LCD_BRIGHTNESS    (13)   // [RW] LCD背光亮度
#define MB_HOLD_REG_LCD_MODE          (14)   // [RW] LCD显示模式
#define MB_HOLD_REG_LCD_ASSCI         (15)   // [RW] LCD字符映射区
#define MB_HOLD_REG_ETH0_IP_HIGH      (16)   // [RW] ETH0 IP 高字节
#define MB_HOLD_REG_ETH0_IP_LOW       (17)   // [RW] ETH0 IP 低字节
#define MB_HOLD_REG_ETH1_IP_HIGH      (18)   // [RW] ETH1 IP 高字节
#define MB_HOLD_REG_ETH1_IP_LOW       (19)   // [RW] ETH1 IP 低字节
#define MB_HOLD_REG_WLAN_IP_HIGH      (20)   // [RW] WLAN IP 高字节
#define MB_HOLD_REG_WLAN_IP_LOW       (21)   // [RW] WLAN IP 低字节
#define MB_HOLD_REG_RGB1_ARM_START    (64)   // [RW] RGB1灯映射起始地址
#define MB_HOLD_REG_RGB1_ARM_END      (127)  // [RW] RGB1灯映射结束地址
#define MB_HOLD_REG_LCD_RAM_START     (256)  // [RW] LCD RAM 起始地址
#define MB_HOLD_REG_LCD_RAM_END       (511)  // [RW] LCD RAM 结束地址（共占用256个寄存器 = 512字节）

extern __IO uint8_t g_mb_coils[MB_COIL_COUNT + 1];
extern __IO uint16_t g_mb_holding_regs[MB_HOLDING_REG_COUNT + 1];

void modbus_reg_init(void);

#ifdef __cplusplus
}
#endif

#endif /* _DEMO_H_ */
