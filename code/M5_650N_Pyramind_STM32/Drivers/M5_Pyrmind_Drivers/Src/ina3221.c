/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "ina3221.h"

/**
 * @brief Swap byte order of 16-bit register.
 * @brief 交换16位寄存器的字节顺序（大端↔小端）
 *
 * @param val 16-bit value 输入值
 * @return Swapped 16-bit value 交换后的值
 */
static uint16_t swap_bytes(uint16_t val)
{
    return (val << 8) | (val >> 8);
}

/**
 * @brief Write a 16-bit value to a register.
 * @brief 写入16位寄存器
 *
 * @param dev INA3221 device object INA3221设备对象
 * @param reg_addr Register address 寄存器地址
 * @param val Value to write 要写入的值
 */
static void write_reg(ina3221_t *dev, uint8_t reg_addr, uint16_t val)
{
    uint16_t swapped = swap_bytes(val);
    HAL_I2C_Mem_Write(dev->i2c, dev->address, reg_addr, 1, (uint8_t *)&swapped, 2, 10);
}

/**
 * @brief Read a 16-bit value from a register.
 * @brief 读取16位寄存器的值
 *
 * @param dev INA3221 device object INA3221设备对象
 * @param reg_addr Register address 寄存器地址
 * @param val Pointer to store read value 存储读取值的指针
 */
static void read_reg(ina3221_t *dev, uint8_t reg_addr, uint16_t *val)
{
    uint8_t buf[2] = {0};
    if (HAL_I2C_Mem_Read(dev->i2c, dev->address, reg_addr, 1, buf, 2, 10) == HAL_OK) {
        *val = (buf[0] << 8) | buf[1];
    } else {
        *val = 0;  // 出错时置0
    }
}

/**
 * @brief Update voltage and current for a specific channel.
 * @brief 更新指定通道（0 ~ 2）的电压和电流
 *
 * @param dev INA3221 device object INA3221设备对象
 * @param ch Channel index (0-2) 通道索引（0~2）
 */
static void update_channel_measurement(ina3221_t *dev, uint8_t ch)
{
    if (ch >= 3) return;

    // 读取分流电压
    uint16_t raw_shunt;
    read_reg(dev, INA3221_REG_SHUNT_VOLTAGE_CH1 + 2 * ch, &raw_shunt);
    dev->shunt_raw[ch] = raw_shunt;

    // 计算电流：原始值需右移3位，转换为uV，再除以分流电阻
    int32_t val = (int16_t)raw_shunt;
    val >>= 3;
    val *= 40;  // 40uV/LSB
    dev->current[ch] = val / SHUNT_RESISTOR_MOHM;

    if (raw_shunt == 0xFFF8) {
        dev->current[ch] = 0;  // 无电流时补0
    }

    // 读取总线电压
    uint16_t raw_bus;
    read_reg(dev, INA3221_REG_BUS_VOLTAGE_CH1 + 2 * ch, &raw_bus);
    dev->voltage[ch] = raw_bus;
}

/**
 * @brief Initialize INA3221 device.
 * @brief 初始化 INA3221 设备
 *
 * @param dev INA3221 device object INA3221设备对象
 * @param i2c I2C handle I2C句柄
 * @param addr I2C address I2C设备地址
 */
void ina3221_init(ina3221_t *dev, I2C_HandleTypeDef *i2c, uint8_t addr)
{
    dev->i2c     = i2c;
    dev->address = addr;

    // 复位设备
    write_reg(dev, INA3221_REG_CONFIG, 0x8000);
    HAL_Delay(10);

    // 配置：使能3通道，平均16次，1.1ms采样，持续模式
    uint16_t cfg = 0x7527;
    write_reg(dev, INA3221_REG_CONFIG, cfg);
}

/**
 * @brief Update all channel voltage and current measurements.
 * @brief 更新所有通道的电压和电流值
 *
 * @param dev INA3221 device object INA3221设备对象
 */
void ina3221_update_measurements(ina3221_t *dev)
{
    for (int ch = 0; ch < 3; ch++) {
        uint16_t raw_shunt;
        read_reg(dev, INA3221_REG_SHUNT_VOLTAGE_CH1 + 2 * ch, &raw_shunt);
        dev->shunt_raw[ch] = raw_shunt;

        int32_t val = (int16_t)raw_shunt;
        val >>= 3;
        val *= 40;
        dev->current[ch] = val / SHUNT_RESISTOR_MOHM;

        if (raw_shunt == 0xFFF8) {
            dev->current[ch] = 0;  // 无电流时补0
        }

        uint16_t raw_bus;
        read_reg(dev, INA3221_REG_BUS_VOLTAGE_CH1 + 2 * ch, &raw_bus);
        dev->voltage[ch] = raw_bus;
    }
}

/**
 * @brief Update CH1 voltage and current.
 * @brief 更新通道1（CH1）的电压和电流
 *
 * @param dev INA3221 device object INA3221设备对象
 */
void ina3221_update_ch0(ina3221_t *dev)
{
    update_channel_measurement(dev, 0);
}

/**
 * @brief Update CH2 voltage and current.
 * @brief 更新通道2（CH2）的电压和电流
 *
 * @param dev INA3221 device object INA3221设备对象
 */
void ina3221_update_ch1(ina3221_t *dev)
{
    update_channel_measurement(dev, 1);
}

/**
 * @brief Update CH3 voltage and current.
 * @brief 更新通道3（CH3）的电压和电流
 *
 * @param dev INA3221 device object INA3221设备对象
 */
void ina3221_update_ch2(ina3221_t *dev)
{
    update_channel_measurement(dev, 2);
}

/**
 * @brief Set critical current limit for a channel.
 * @brief 设置某通道的临界电流限制
 *
 * @param dev INA3221 device object INA3221设备对象
 * @param channel Channel index (0-2) 通道索引（0~2）
 * @param current_mA Limit current in mA 临界电流值（单位：毫安）
 */
void ina3221_set_critical_limit(ina3221_t *dev, uint8_t channel, uint16_t current_mA)
{
    if (channel < 3) {
        uint16_t reg = INA3221_REG_CRIT_CH1 + 2 * channel;
        uint16_t val = (current_mA * SHUNT_RESISTOR_MOHM / 40) << 3;
        write_reg(dev, reg, val);
    }
}