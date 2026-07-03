/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef _MX77812_H_
#define _MX77812_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"

/**
 * @brief SPI communication timeout in milliseconds.
 * @brief SPI 通信超时时间（毫秒）
 */
#define MAX77812_SPI_TIMEOUT (10)

/**
 * @brief SPI write operation flag (R/W = 1).
 * @brief SPI 写操作标志位（R/W = 1）
 */
#define MAX77812_RW_WRITE (0x80)

/**
 * @brief SPI read operation flag (R/W = 0).
 * @brief SPI 读操作标志位（R/W = 0）
 */
#define MAX77812_RW_READ (0x00)

/**
 * @brief Single operation mode flag (S/M = 0).
 * @brief 单次操作模式标志位（S/M = 0）
 */
#define MAX77812_SINGLE_OP (0x00)

/**
 * @brief Multiple operation mode flag (S/M = 1).
 * @brief 多次操作模式标志位（S/M = 1）
 */
#define MAX77812_MULTIPLE_OP (0x40)

/**
 * @brief Compose SPI header from R/W and S/M bits.
 * @brief 将R/W和S/M位组合成SPI头字节
 */
#define MAX77812_SPI_HEADER(RW, SM) ((RW) | (SM))

/**
 * @brief Enable chip select for PMIC1 (pull low).
 * @brief 使能 PMIC1 片选（GPIO拉低）
 */
#define PMIC1_CS_ENABLE() HAL_GPIO_WritePin(PMIC1_CS_GPIO_Port, PMIC1_CS_Pin, GPIO_PIN_RESET)

/**
 * @brief Disable chip select for PMIC1 (pull high).
 * @brief 失能 PMIC1 片选（GPIO拉高）
 */
#define PMIC1_CS_DISABLE() HAL_GPIO_WritePin(PMIC1_CS_GPIO_Port, PMIC1_CS_Pin, GPIO_PIN_SET)

/**
 * @brief Enable chip select for PMIC2 (pull low).
 * @brief 使能 PMIC2 片选（GPIO拉低）
 */
#define PMIC2_CS_ENABLE() HAL_GPIO_WritePin(PMIC2_CS_GPIO_Port, PMIC2_CS_Pin, GPIO_PIN_RESET)

/**
 * @brief Disable chip select for PMIC2 (pull high).
 * @brief 失能 PMIC2 片选（GPIO拉高）
 */
#define PMIC2_CS_DISABLE() HAL_GPIO_WritePin(PMIC2_CS_GPIO_Port, PMIC2_CS_Pin, GPIO_PIN_SET)

/**
 * @brief PMIC chip selector enum.
 * @brief PMIC芯片选择枚举类型
 */
typedef enum {
    MAX77812_PMIC1 = 0x00, /**< Select PMIC1 选择PMIC1 */
    MAX77812_PMIC2 = 0x01  /**< Select PMIC2 选择PMIC2 */
} max77812_pmic_t;

HAL_StatusTypeDef max77812_write_reg(max77812_pmic_t pmic, uint8_t reg, uint8_t data);
HAL_StatusTypeDef max77812_read_reg(max77812_pmic_t pmic, uint8_t reg, uint8_t *data);
HAL_StatusTypeDef max77812_mult_write_reg(max77812_pmic_t pmic, uint8_t reg, const uint8_t *data, uint8_t size);
HAL_StatusTypeDef max77812_mult_read_reg(max77812_pmic_t pmic, uint8_t reg, uint8_t *data, uint8_t size);

#ifdef __cplusplus
}
#endif

#endif /* _MX77812_H_ */