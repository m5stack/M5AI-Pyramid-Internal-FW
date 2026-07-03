/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#include "time_wakeup.h"
#include "demo.h"

extern __IO uint8_t page60_buffer[16];

static __IO uint32_t wakeup_time = WAKEUP_TIME_DEFAULT;

__IO uint64_t wake_time_ms = 0;

__IO uint32_t power_turn_off_time = 0;

/**
 * @brief Set high 16 bits of the wakeup time.
 * @brief 设置唤醒时间的高 16 位
 */
void set_wakeup_time_high(uint16_t high)
{
    uint32_t new_wakeup_time = ((uint32_t)high << 16) | (wakeup_time & 0xFFFF);
    if (new_wakeup_time != wakeup_time) {
        wakeup_time  = new_wakeup_time;
        wake_time_ms = wakeup_time * 1000;
    }
}

/**
 * @brief Set low 16 bits of the IP address.
 * @brief 设置 IP 地址的低 16 位
 */
void set_wakeup_time_low(uint16_t low)
{
    uint32_t new_wakeup_time = (wakeup_time & 0xFFFF0000) | low;
    if (new_wakeup_time != wakeup_time) {
        wakeup_time  = new_wakeup_time;
        wake_time_ms = wakeup_time * 1000;
    }
}

/**
 * @brief Get full wakeup time.
 * @brief 获取完整的唤醒时间
 */
uint32_t get_wakeup_time(void)
{
    return wakeup_time;
}

void time_wakeup(void)
{
    if (wakeup_time == WAKEUP_TIME_DEFAULT) {
        return;
    }

    if (get_power_wakeup_status() == POWER_WAKEUP_STATUS_NONE) {
        return;
    }

    if (HAL_GetTick() - power_turn_off_time >= wake_time_ms) {
        get_page60_buffer();
        for (uint8_t i = 4; i < 15; i++) {
            g_mb_coils[i] = page60_buffer[i];
        }
        g_mb_coils[MB_COIL_EXT_5V_SUPPLY]     = 1;
        g_mb_coils[MB_COIL_INT_5V_MAIN_POWER] = 1;
        g_mb_coils[MB_COIL_CLK_MODE_SELECT]   = 0;
        system_power_on();
        peripheral_power_on();
        rgb_power_on();
        lcd_power_on();
        set_power_wakeup_status(POWER_WAKEUP_STATUS_NONE);
    }
}
