/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef _TIME_WAKEUP_H_
#define _TIME_WAKEUP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"

#define WAKEUP_TIME_DEFAULT (0)

void set_wakeup_time_high(uint16_t high);
void set_wakeup_time_low(uint16_t low);
uint32_t get_wakeup_time(void);
void time_wakeup(void);

#ifdef __cplusplus
}
#endif

#endif /* _TIME_WAKEUP_H_ */
