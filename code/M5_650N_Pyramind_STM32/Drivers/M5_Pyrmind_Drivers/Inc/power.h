/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef _POWER_H_
#define _POWER_H_

#ifdef __cplusplus

extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "config.h"
#include "max77812.h"
#include "fan.h"
#include "demo.h"

/******************************* PMIC 寄存器定义 *******************************/
#define PMIC_REG_RESET           (0x00)
#define PMIC_REG_EN_CTRL         (0x06)
#define PMIC_POK_STATUS_REGISTER (0x22)

/******************************* 相位使能位定义 *******************************/
#define EN_M1 (1 << 0)
#define EN_M2 (1 << 2)
#define EN_M3 (1 << 4)
#define EN_M4 (1 << 6)

/******************************* 电源轨与相位映射 *****************************/
// PMIC1配置
#define VDD_CORE_PHASE EN_M1  // M1
#define VDD_NN_PHASE   EN_M3  // M3

// PMIC2配置
#define DDRV1V1_PHASE EN_M3  // M3
#define DDRV0V6_PHASE EN_M4  // M4
#define VDD_CPU_PHASE EN_M1  // M1

#define SYSTEM_POWER_ON_TIMER_MIN (1000)

#define VDD_CPU_OFFSET_MAX (40)

#define VDD_CPU_OFFSET_DEFAULT (20)

typedef enum {
    POWER_WAKEUP_STATUS_NONE        = 0,
    POWER_WAKEUP_STATUS_SYSTEM      = 1,
    POWER_WAKEUP_STATUS_SYSTEM_PERI = 2
} power_wakeup_status_t;

void system_power_boot(void);
void system_power_on(void);
void system_power_off(void);
void peripheral_power_boot(void);
void peripheral_power_on(void);
void peripheral_power_off(void);
bool system_power_off_time_check(uint16_t power_off_timer);
bool vdd_core_offset_check(uint16_t vdd_core);
void set_system_main_power_update_status(bool status);
void set_peripheral_main_power_update_status(bool status);
void set_peripheral_aux_power_update_status(bool status);
void set_power_update_status(bool status);
void set_power_wakeup_status(power_wakeup_status_t status);
power_wakeup_status_t get_power_wakeup_status(void);
void system_power_update(void);
void peripheral_power_update(void);
void power_update(void);

#ifdef __cplusplus
}
#endif

#endif /* _POWER_H_ */
