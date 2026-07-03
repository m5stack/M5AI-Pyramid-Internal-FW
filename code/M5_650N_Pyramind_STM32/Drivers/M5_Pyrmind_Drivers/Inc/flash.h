/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef _FLASH_H_
#define _FLASH_H_

#ifdef __cplusplus

extern "C" {
#endif

#include "main.h"
#include <stdbool.h>
#include "stm32g4xx_hal_flash_ex.h"
#include "config.h"
#include "demo.h"

/**
 * @brief Flash page size for STM32G4 series (2 KB).
 * @brief STM32G4 系列的 Flash 页面大小（2 KB）。
 */
#define STM32G4xx_PAGE_SIZE (0x800)  // Page size: 2048 bytes (2 KB)

/**
 * @brief Base address of Flash memory.
 * @brief Flash 存储器的起始地址。
 */
#define STM32G4xx_FLASH_PAGE0_STARTADDR (0x8000000)  // Start address of flash page 0 (0x08000000)

/**
 * @brief Start address of Flash page 60.
 * @brief Flash 第 60 页的起始地址。
 */
#define STM32G4xx_FLASH_PAGE60_STARTADDR (STM32G4xx_FLASH_PAGE0_STARTADDR + 60 * STM32G4xx_PAGE_SIZE)

/**
 * @brief Start address of Flash page 61.
 * @brief Flash 第 61 页的起始地址。
 */
#define STM32G4xx_FLASH_PAGE61_STARTADDR (STM32G4xx_FLASH_PAGE0_STARTADDR + 61 * STM32G4xx_PAGE_SIZE)

/**
 * @brief Start address of Flash page 62.
 * @brief Flash 第 62 页的起始地址。
 */
#define STM32G4xx_FLASH_PAGE62_STARTADDR (STM32G4xx_FLASH_PAGE0_STARTADDR + 62 * STM32G4xx_PAGE_SIZE)

extern __IO uint8_t page60_buffer[16];
extern __IO uint16_t page61_buffer[12];
extern __IO uint16_t page62_buffer[2];

void flash_init(void);

bool set_page60_buffer(void);
bool get_page60_buffer(void);

bool set_page61_buffer(void);
bool get_page61_buffer(void);

bool set_page62_buffer(void);
bool get_page62_buffer(void);

void set_page60_update_flag(bool flag);
void set_page61_update_flag(bool flag);
void set_page62_update_flag(bool flag);

void update_flash(void);

#ifdef __cplusplus
}
#endif

#endif /* _FLASH_H_ */
