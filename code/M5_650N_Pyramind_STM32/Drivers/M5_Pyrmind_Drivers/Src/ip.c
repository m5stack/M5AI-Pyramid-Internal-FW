/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#include "ip.h"

static __IO uint32_t ip[3]         = {IP_ADDR_DEFAULT, IP_ADDR_DEFAULT, IP_ADDR_DEFAULT};
static __IO uint8_t ip_update_flag = 0;

/**
 * @brief Set high 16 bits of the IP address.
 * @brief 设置 IP 地址的高 16 位
 */
void set_ip_high(uint8_t index, uint16_t high)
{
    uint32_t new_ip = ((uint32_t)high << 16) | (ip[index] & 0xFFFF);
    if (new_ip != ip[index]) {
        ip[index]      = new_ip;
        ip_update_flag = 1;
    }
}

/**
 * @brief Set low 16 bits of the IP address.
 * @brief 设置 IP 地址的低 16 位
 */
void set_ip_low(uint8_t index, uint16_t low)
{
    uint32_t new_ip = (ip[index] & 0xFFFF0000) | low;
    if (new_ip != ip[index]) {
        ip[index]      = new_ip;
        ip_update_flag = 1;
    }
}

/**
 * @brief Get full IP address.
 * @brief 获取完整的 IP 地址
 */
uint32_t get_ip(uint8_t index)
{
    return ip[index];
}

void set_ip_updated(uint8_t val)
{
    ip_update_flag = val;
}

uint8_t get_ip_updated(void)
{
    return ip_update_flag;
}