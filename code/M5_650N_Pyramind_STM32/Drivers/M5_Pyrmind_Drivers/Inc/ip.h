/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef _IP_H_
#define _IP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"

#define IP_ADDR_DEFAULT (0)

void set_ip_high(uint8_t index, uint16_t high);
void set_ip_low(uint8_t index, uint16_t low);
uint32_t get_ip(uint8_t index);
void set_ip_updated(uint8_t val);
uint8_t get_ip_updated(void);

#ifdef __cplusplus
}
#endif

#endif /* _IP_H_ */