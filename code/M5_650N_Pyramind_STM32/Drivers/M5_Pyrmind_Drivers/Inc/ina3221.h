/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef _INA3221_H_
#define _INA3221_H_

#include "config.h"

/**
 * @brief Shunt resistor value in milliohms.
 * @brief 分流电阻值（单位：毫欧）
 */
#define SHUNT_RESISTOR_MOHM (10)

/**
 * @brief Configuration register address.
 * @brief 配置寄存器地址
 */
#define INA3221_REG_CONFIG (0x00)

/**
 * @brief Shunt voltage register for channel 1.
 * @brief 第1通道分流电压寄存器地址
 */
#define INA3221_REG_SHUNT_VOLTAGE_CH1 (0x01)

/**
 * @brief Bus voltage register for channel 1.
 * @brief 第1通道总线电压寄存器地址
 */
#define INA3221_REG_BUS_VOLTAGE_CH1 (0x02)

/**
 * @brief Shunt voltage register for channel 2.
 * @brief 第2通道分流电压寄存器地址
 */
#define INA3221_REG_SHUNT_VOLTAGE_CH2 (0x03)

/**
 * @brief Bus voltage register for channel 2.
 * @brief 第2通道总线电压寄存器地址
 */
#define INA3221_REG_BUS_VOLTAGE_CH2 (0x04)

/**
 * @brief Shunt voltage register for channel 3.
 * @brief 第3通道分流电压寄存器地址
 */
#define INA3221_REG_SHUNT_VOLTAGE_CH3 (0x05)

/**
 * @brief Bus voltage register for channel 3.
 * @brief 第3通道总线电压寄存器地址
 */
#define INA3221_REG_BUS_VOLTAGE_CH3 (0x06)

/**
 * @brief Critical alert register for channel 1.
 * @brief 第1通道临界电流限制寄存器
 */
#define INA3221_REG_CRIT_CH1 (0x07)

/**
 * @brief Critical alert register for channel 2.
 * @brief 第2通道临界电流限制寄存器
 */
#define INA3221_REG_CRIT_CH2 (0x09)

/**
 * @brief Critical alert register for channel 3.
 * @brief 第3通道临界电流限制寄存器
 */
#define INA3221_REG_CRIT_CH3 (0x0B)

/**
 * @brief Manufacturer ID register address.
 * @brief 厂商 ID 寄存器地址
 */
#define INA3221_REG_MANUFACTURER_ID (0xFE)

/**
 * @brief Die ID register address.
 * @brief 芯片 ID 寄存器地址
 */
#define INA3221_REG_DIE_ID (0xFF)

/**
 * @brief INA3221 address when A1=0, A0=0.
 * @brief INA3221 地址（A1=0，A0=0）
 */
#define INA3221_ADDR_A1A0_00 (0x40)

/**
 * @brief INA3221 address when A1=0, A0=1.
 * @brief INA3221 地址（A1=0，A0=1）
 */
#define INA3221_ADDR_A1A0_01 (0x41)

/**
 * @brief INA3221 device object.
 * @brief INA3221 设备对象结构体
 */
typedef struct {
    I2C_HandleTypeDef *i2c; /**< I2C handle I2C句柄 */
    uint8_t address;        /**< I2C address I2C地址 */
    uint16_t voltage[3];    /**< Bus voltage for each channel 每通道总线电压值 */
    uint16_t shunt_raw[3];  /**< Raw shunt voltage for each channel 每通道分流电压原始值 */
    int32_t current[3];     /**< Current (mA) for each channel 每通道电流（单位：毫安） */
} ina3221_t;

void ina3221_init(ina3221_t *dev, I2C_HandleTypeDef *i2c, uint8_t addr);
void ina3221_update_measurements(ina3221_t *dev);
void ina3221_update_ch0(ina3221_t *dev);
void ina3221_update_ch1(ina3221_t *dev);
void ina3221_update_ch2(ina3221_t *dev);
void ina3221_set_critical_limit(ina3221_t *dev, uint8_t channel, uint16_t current_mA);

#endif  // _INA3221_H_