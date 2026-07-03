#include "max77812.h"

/**
 * @brief Enable chip select for the specified MAX77812 PMIC.
 *        使能指定的 MAX77812 电源管理芯片（拉低片选信号）。
 *
 * @param pmic The PMIC instance to enable (MAX77812_PMIC1 or MAX77812_PMIC2).
 *             要使能的 PMIC 实例（MAX77812_PMIC1 或 MAX77812_PMIC2）。
 */
static void max77812_enable(max77812_pmic_t pmic)
{
    switch (pmic) {
        case MAX77812_PMIC1:
            PMIC1_CS_ENABLE();  // PMIC1 CS
            break;
        case MAX77812_PMIC2:
            PMIC2_CS_ENABLE();  // PMIC2 CS
            break;
        default:
            break;
    }
}

/**
 * @brief Disable chip select for the specified MAX77812 PMIC.
 *        关闭指定的 MAX77812 电源管理芯片（拉高片选信号）。
 *
 * @param pmic The PMIC instance to disable (MAX77812_PMIC1 or MAX77812_PMIC2).
 *             要关闭的 PMIC 实例（MAX77812_PMIC1 或 MAX77812_PMIC2）。
 */
static void max77812_disable(max77812_pmic_t pmic)
{
    switch (pmic) {
        case MAX77812_PMIC1:
            PMIC1_CS_DISABLE();  // PMIC1 CS
            break;
        case MAX77812_PMIC2:
            PMIC2_CS_DISABLE();  // PMIC2 CS
            break;
        default:
            break;
    }
}

/**
 * @brief Write single byte to MAX77812 register.
 *        向 MAX77812 寄存器写入一个字节数据。
 *
 * @param pmic The PMIC to access. 要访问的 PMIC。
 * @param reg  Register address. 寄存器地址。
 * @param data Data byte to write. 要写入的数据字节。
 * @return HAL status. HAL 状态返回值。
 */
HAL_StatusTypeDef max77812_write_reg(max77812_pmic_t pmic, uint8_t reg, uint8_t data)
{
    uint8_t tx_buf[4] = {
        MAX77812_SPI_HEADER(MAX77812_RW_WRITE, MAX77812_SINGLE_OP),  // 写入操作（单次）
        reg,                                                         // 地址
        0x01,                                                        // 数据长度
        data                                                         // 数据
    };

    max77812_enable(pmic);
    HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi1, tx_buf, 4, MAX77812_SPI_TIMEOUT);
    max77812_disable(pmic);

    return status;
}

/**
 * @brief Read single byte from MAX77812 register.
 *        从 MAX77812 寄存器读取一个字节数据。
 *
 * @param pmic The PMIC to access. 要访问的 PMIC。
 * @param reg  Register address. 寄存器地址。
 * @param data Pointer to store read byte. 用于存储读取数据的指针。
 * @return HAL status. HAL 状态返回值。
 */
HAL_StatusTypeDef max77812_read_reg(max77812_pmic_t pmic, uint8_t reg, uint8_t *data)
{
    uint8_t tx_buf[3] = {
        MAX77812_SPI_HEADER(MAX77812_RW_READ, MAX77812_SINGLE_OP),  // 读取操作（单次）
        reg,                                                        // 地址
        0x01                                                        // 数据长度
    };
    max77812_enable(pmic);
    HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi1, tx_buf, 3, MAX77812_SPI_TIMEOUT);
    if (status == HAL_OK) {
        status = HAL_SPI_Receive(&hspi1, data, 1, MAX77812_SPI_TIMEOUT);
    }
    max77812_disable(pmic);
    return status;
}

/**
 * @brief Write multiple bytes to consecutive MAX77812 registers.
 *        向 MAX77812 连续寄存器写入多个字节数据。
 *
 * @param pmic The PMIC to access. 要访问的 PMIC。
 * @param reg  Start register address. 起始寄存器地址。
 * @param data Pointer to data buffer. 要写入的数据缓存指针。
 * @param size Number of bytes to write. 写入的数据长度。
 * @return HAL status. HAL 状态返回值。
 */
HAL_StatusTypeDef max77812_mult_write_reg(max77812_pmic_t pmic, uint8_t reg, const uint8_t *data, uint8_t size)
{
    if (data == NULL || size == 0) return HAL_ERROR;

    uint8_t header[3] = {
        MAX77812_SPI_HEADER(MAX77812_RW_WRITE, MAX77812_MULTIPLE_OP), reg,
        size - 1  // 数据长度 - 1
    };

    max77812_enable(pmic);
    HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi1, header, sizeof(header), MAX77812_SPI_TIMEOUT);
    if (status == HAL_OK) {
        status = HAL_SPI_Transmit(&hspi1, (uint8_t *)data, size, MAX77812_SPI_TIMEOUT);
    }
    max77812_disable(pmic);
    return status;
}

/**
 * @brief Read multiple bytes from consecutive MAX77812 registers.
 *        从 MAX77812 连续寄存器中读取多个字节数据。
 *
 * @param pmic The PMIC to access. 要访问的 PMIC。
 * @param reg  Start register address. 起始寄存器地址。
 * @param data Pointer to buffer for read data. 用于保存读取数据的缓存指针。
 * @param size Number of bytes to read. 要读取的数据长度。
 * @return HAL status. HAL 状态返回值。
 */
HAL_StatusTypeDef max77812_mult_read_reg(max77812_pmic_t pmic, uint8_t reg, uint8_t *data, uint8_t size)
{
    if (data == NULL || size == 0) return HAL_ERROR;

    uint8_t header[3] = {
        MAX77812_SPI_HEADER(MAX77812_RW_READ, MAX77812_MULTIPLE_OP), reg,
        size - 1  // 数据长度 - 1
    };

    max77812_enable(pmic);
    HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi1, header, sizeof(header), MAX77812_SPI_TIMEOUT);
    if (status == HAL_OK) {
        status = HAL_SPI_Receive(&hspi1, data, size, MAX77812_SPI_TIMEOUT);
    }
    max77812_disable(pmic);

    return status;
}