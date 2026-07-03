/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef _USERRI2C_H_
#define _USERRI2C_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"

void useri2c_read_byte(void);
void useri2c_write_byte(uint8_t val);

#ifdef __cplusplus
}
#endif

#endif /* _USERRI2C_H_ */
