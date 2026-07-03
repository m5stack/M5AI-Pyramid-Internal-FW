#include "useri2c.h"

extern I2C_HandleTypeDef hi2c2;

/**
 * @brief Read a byte from the I2C device using address from Modbus register.
 * @brief 从I2C设备读取一个字节，地址由Modbus寄存器提供。
 *
 * Modbus格式：高字节为设备地址，低字节为寄存器地址
 * 读取成功后存入 MB_HOLD_REG_I2C_READ；失败则写入0xFFFF
 */
void useri2c_read_byte(void)
{
    uint16_t mb_raw  = g_mb_holding_regs[MB_HOLD_REG_I2C_ADDR];  // 高字节:I2C地址，低字节:寄存器地址
    uint8_t i2c_addr = (uint8_t)(mb_raw >> 8);                   // I2C从机地址
    uint8_t reg_addr = (uint8_t)(mb_raw & 0xFF);                 // I2C寄存器地址
    uint8_t data     = 0;
    HAL_StatusTypeDef status;

    status = HAL_I2C_Mem_Read(&hi2c2, (i2c_addr << 1), reg_addr, I2C_MEMADD_SIZE_8BIT, &data, 1, 10);

    if (status == HAL_OK) {
        g_mb_holding_regs[MB_HOLD_REG_I2C_READ] = data;
#ifdef USER_I2C_DEBUG
        printf("[I2C2][READ][OK] MB_ADDR=0x%04X | I2C_ADDR=0x%02X | REG_ADDR=0x%02X | DATA=0x%02X\r\n", mb_raw,
               i2c_addr, reg_addr, data);
#endif
    } else {
        g_mb_holding_regs[MB_HOLD_REG_I2C_READ] = 0xFFFF;
#ifdef USER_I2C_DEBUG
        printf("[I2C2][READ][FAIL] MB_ADDR=0x%04X | I2C_ADDR=0x%02X | REG_ADDR=0x%02X | HAL_Status=%d\r\n", mb_raw,
               i2c_addr, reg_addr, status);
#endif
    }
}

/**
 * @brief Write a byte to the I2C device using address from Modbus register.
 * @brief 向I2C设备写入一个字节，地址由Modbus寄存器提供。
 *
 * 写入成功将 MB_HOLD_REG_I2C_WRITE 清零；失败则写入0xFFFF。
 * @param val 要写入的数据字节（8位）
 */
void useri2c_write_byte(uint8_t val)
{
    uint16_t mb_raw  = g_mb_holding_regs[MB_HOLD_REG_I2C_ADDR];
    uint8_t i2c_addr = (uint8_t)(mb_raw >> 8);    // I2C从机地址
    uint8_t reg_addr = (uint8_t)(mb_raw & 0xFF);  // I2C寄存器地址
    uint8_t data     = val;
    HAL_StatusTypeDef status;

    status = HAL_I2C_Mem_Write(&hi2c2, (i2c_addr << 1), reg_addr, I2C_MEMADD_SIZE_8BIT, &data, 1, 10);

    if (status == HAL_OK) {
        g_mb_holding_regs[MB_HOLD_REG_I2C_WRITE] = 0;  // 写入成功
#ifdef USER_I2C_DEBUG
        printf("[I2C2][WRITE][OK] MB_ADDR=0x%04X | I2C_ADDR=0x%02X | REG_ADDR=0x%02X | DATA=0x%02X\r\n", mb_raw,
               i2c_addr, reg_addr, data);
#endif
    } else {
        g_mb_holding_regs[MB_HOLD_REG_I2C_WRITE] = 0xFFFF;  // 写入失败
#ifdef USER_I2C_DEBUG
        printf(
            "[I2C2][WRITE][FAIL] MB_ADDR=0x%04X | I2C_ADDR=0x%02X | REG_ADDR=0x%02X | DATA=0x%02X | HAL_Status=%d\r\n",
            mb_raw, i2c_addr, reg_addr, data, status);
#endif
    }
}