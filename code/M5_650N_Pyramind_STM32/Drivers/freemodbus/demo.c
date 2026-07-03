/*
 * FreeModbus Libary: BARE Demo Application
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id$
 */

#include "demo.h"
#include "driver_ssd1306_advance.h"

/* ----------------------- Static variables ---------------------------------*/

// 线圈寄存器状态数组
__IO uint8_t g_mb_coils[MB_COIL_COUNT + 1] = {0};

// 离散输入寄存器数组
__IO uint8_t g_mb_discrete_inputs[MB_DISCRETE_INPUT_COUNT + 1] = {0};

// 输入寄存器数组
__IO uint16_t g_mb_input_regs[MB_INPUT_REG_COUNT + 1] = {0};

// 保持寄存器数组
__IO uint16_t g_mb_holding_regs[MB_HOLDING_REG_COUNT + 1] = {0};

extern ina3221_t ina1;
extern ina3221_t ina2;
extern bool peripheral_aux_power_update_flag[MB_COIL_COUNT + 1];
extern __IO uint8_t uart_receiving_flag;

void modbus_reg_init(void)
{
    // 线圈寄存器
    for (uint8_t i = 4; i < 15; i++) {
        g_mb_coils[i] = page60_buffer[i];
    }
    g_mb_coils[MB_COIL_EXT_5V_SUPPLY]     = 1;
    g_mb_coils[MB_COIL_INT_5V_MAIN_POWER] = 1;
    g_mb_coils[MB_COIL_CLK_MODE_SELECT]   = 0;
    // 保持寄存器
    g_mb_holding_regs[MB_HOLD_REG_MAIN_POWER_DELAY]  = page61_buffer[1];
    g_mb_holding_regs[MB_HOLD_REG_FAN_SPEED_PERCENT] = page61_buffer[2];
    g_mb_holding_regs[MB_HOLD_REG_UART_BAUD_HIGH]    = (uint16_t)((USART_DEFAULT_BAUDRATE >> 16) & 0xFFFF);
    g_mb_holding_regs[MB_HOLD_REG_UART_BAUD_LOW]     = (uint16_t)(USART_DEFAULT_BAUDRATE & 0xFFFF);
    g_mb_holding_regs[MB_HOLD_REG_RGB_COUNT]         = page61_buffer[3];
    g_mb_holding_regs[MB_HOLD_REG_RGB_MODE]          = page61_buffer[4];
    g_mb_holding_regs[MB_HOLD_REG_VDD_CPU_OFFSET]    = page62_buffer[1];
    g_mb_holding_regs[MB_HOLD_REG_LCD_BRIGHTNESS]    = SSD1306_ADVANCE_DEFAULT_CONTRAST;
    g_mb_holding_regs[MB_HOLD_REG_LCD_MODE]          = page61_buffer[5];
    g_mb_holding_regs[MB_HOLD_REG_ETH0_IP_HIGH]      = page61_buffer[6];
    g_mb_holding_regs[MB_HOLD_REG_ETH0_IP_LOW]       = page61_buffer[7];
    g_mb_holding_regs[MB_HOLD_REG_ETH1_IP_HIGH]      = page61_buffer[8];
    g_mb_holding_regs[MB_HOLD_REG_ETH1_IP_LOW]       = page61_buffer[9];
    g_mb_holding_regs[MB_HOLD_REG_WLAN_IP_HIGH]      = page61_buffer[10];
    g_mb_holding_regs[MB_HOLD_REG_WLAN_IP_LOW]       = page61_buffer[11];
    set_ip_high(0, page61_buffer[6]);
    set_ip_low(0, page61_buffer[7]);
    set_ip_high(1, page61_buffer[8]);
    set_ip_low(1, page61_buffer[9]);
    set_ip_high(2, page61_buffer[10]);
    set_ip_low(2, page61_buffer[11]);
}

/* ----------------------- Functions ---------------------------------*/

// 线圈寄存器0
eMBErrorCode eMBRegCoilsCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode)
{
    uart_receiving_flag = 0;

    switch (eMode) {
        case MB_REG_READ:
            if (usAddress < 1 || usAddress + usNCoils - 1 >= (MB_COIL_COUNT - 3)) return MB_ENOREG;

            for (USHORT i = 0; i < usNCoils; i++) {
                uint8_t bit    = g_mb_coils[usAddress + i] ? 1 : 0;
                USHORT byteIdx = i / 8;
                UCHAR bitPos   = i % 8;

                if (bit) {
                    pucRegBuffer[byteIdx] |= (1 << bitPos);
                } else {
                    pucRegBuffer[byteIdx] &= ~(1 << bitPos);
                }
            }
            break;

        case MB_REG_WRITE:
            if (usAddress < 1 || usAddress + usNCoils - 1 > MB_COIL_COUNT) {
                return MB_ENOREG;
            }

            for (USHORT i = 0; i < usNCoils; i++) {
                USHORT byteIdx  = i / 8;
                UCHAR bitPos    = i % 8;
                uint8_t value   = (pucRegBuffer[byteIdx] >> bitPos) & 0x01;
                USHORT coilAddr = usAddress + i;

                bool valueChanged = (value != g_mb_coils[coilAddr]);

                if (!valueChanged) {
                    continue;
                }

                switch (coilAddr) {
                    case MB_COIL_EXT_5V_SUPPLY:
                        set_peripheral_main_power_update_status(true);
                        break;

                    case MB_COIL_INT_5V_MAIN_POWER:
                        set_system_main_power_update_status(true);
                        break;

                    case MB_COIL_SAVE_COIL_CONFIG:
                        if (value) {
                            set_page60_update_flag(true);
                        }
                        break;

                    case MB_COIL_SAVE_HOLD_CONFIG:
                        if (value) {
                            set_page61_update_flag(true);
                        }
                        break;

                    case MB_COIL_POWER_OFF:
                        if (value) {
                            set_power_update_status(true);
                        }
                        break;
                    case MB_COIL_SYSTEM_READY:
                        
                        break;
                    default:
                        if (coilAddr >= 3 && coilAddr <= 14) {
                            peripheral_aux_power_update_flag[coilAddr] = true;
                            set_peripheral_aux_power_update_status(true);
                        }
                        break;
                }

                g_mb_coils[coilAddr] = value;
            }
            break;

        default:
            return MB_ENOREG;
    }
    update_flash();  // 更新flash
    return MB_ENOERR;
}

eMBErrorCode eMBRegDiscreteCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNDiscrete)
{
    uart_receiving_flag = 0;
    
    if (usAddress < 1 || (usAddress + usNDiscrete - 1) > MB_DISCRETE_INPUT_COUNT) {
        return MB_ENOREG;
    }

    USHORT nBytes = (usNDiscrete + 7) / 8;
    memset(pucRegBuffer, 0, nBytes);
    
    // 按需读取请求范围内的寄存器
    for (USHORT i = 0; i < usNDiscrete; i++) {
        USHORT regAddr = usAddress + i;
        
        switch (regAddr) {
            case MB_DISCRETE_INPUT_PCIE1_PRESENT:
                g_mb_discrete_inputs[regAddr] = 
                    (HAL_GPIO_ReadPin(PCIE0_PRSNTN_GPIO_Port, PCIE0_PRSNTN_Pin) == GPIO_PIN_RESET);
                break;
                
            case MB_DISCRETE_INPUT_PCIE2_PRESENT:
                g_mb_discrete_inputs[regAddr] = 
                    (HAL_GPIO_ReadPin(PCIE1_PRSNTN_GPIO_Port, PCIE1_PRSNTN_Pin) == GPIO_PIN_RESET);
                break;
                
            case MB_DISCRETE_INPUT_3V3_POWER_GOOD:
                g_mb_discrete_inputs[regAddr] = 
                    (HAL_GPIO_ReadPin(VDD3_PG_GPIO_Port, VDD3_PG_Pin) == GPIO_PIN_SET);
                break;
                
            case MB_DISCRETE_INPUT_1V8_POWER_GOOD:
                g_mb_discrete_inputs[regAddr] = 
                    (HAL_GPIO_ReadPin(VDD18_PG_GPIO_Port, VDD18_PG_Pin) == GPIO_PIN_SET);
                break;
                
            case MB_DISCRETE_INPUT_TOP_BUTTON:
                g_mb_discrete_inputs[regAddr] = 
                    (HAL_GPIO_ReadPin(GPIOC, EC_BUTTON_Pin) == GPIO_PIN_RESET);
                break;
                
            case MB_DISCRETE_INPUT_TOP_FALL:
                g_mb_discrete_inputs[regAddr] = button_getFallEvent(&button_top);
                break;
                
            case MB_DISCRETE_INPUT_TOP_RISE:
                g_mb_discrete_inputs[regAddr] = button_getRiseEvent(&button_top);
                break;
                
            case MB_DISCRETE_INPUT_TOP_LONG:
                g_mb_discrete_inputs[regAddr] = button_getLongEvent(&button_top);
                break;
                
            case MB_DISCRETE_INPUT_LCD_BUTTON:
                g_mb_discrete_inputs[regAddr] = 
                    (HAL_GPIO_ReadPin(GPIOC, EC_BUTTON2_Pin) == GPIO_PIN_RESET);
                break;
                
            case MB_DISCRETE_INPUT_LCD_FALL:
                g_mb_discrete_inputs[regAddr] = button_getFallEvent(&button_lcd);
                break;
                
            case MB_DISCRETE_INPUT_LCD_RISE:
                g_mb_discrete_inputs[regAddr] = button_getRiseEvent(&button_lcd);
                break;
                
            case MB_DISCRETE_INPUT_LCD_LONG:
                g_mb_discrete_inputs[regAddr] = button_getLongEvent(&button_lcd);
                break;
            case MB_DISCRETE_INPUT_LCD_STATUS:
                g_mb_discrete_inputs[regAddr] = lcd_initialized;
                break;
            case MB_DISCRETE_INPUT_BTN_POWER_OFF:
                g_mb_discrete_inputs[regAddr] = btn_top_power_flag;
                break;
            default:
                // 对于未定义的寄存器，保持默认值（已通过memset清零）
                break;
        }
        
        // 填充返回缓冲区
        if (g_mb_discrete_inputs[regAddr]) {
            pucRegBuffer[i / 8] |= (1 << (i % 8));
        }
    }
    
    return MB_ENOERR;
}

eMBErrorCode eMBRegInputCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNRegs)
{
    uart_receiving_flag = 0;

    if (usAddress < 1 || usAddress + usNRegs - 1 > MB_INPUT_REG_FIRMWARE_VERSION) {
        return MB_ENOREG;
    }

    // 标记和缓存
    bool pcie1_read = false, pcie2_read = false, usb1_read = false;
    bool usb2_read = false, internal5v_read = false, external5v_read = false;

    uint16_t pcie1_v = 0, pcie1_c = 0;
    uint16_t pcie2_v = 0, pcie2_c = 0;
    uint16_t usb1_v = 0, usb1_c = 0;
    uint16_t usb2_v = 0, usb2_c = 0;
    uint16_t int5_v = 0, int5_c = 0;
    uint16_t ext5_v = 0, ext5_c = 0;

    for (USHORT i = 0; i < usNRegs; i++) {
        USHORT reg     = usAddress + i;
        uint16_t value = 0;

        switch (reg) {
            case MB_INPUT_REG_PCIE1_VOLTAGE:
            case MB_INPUT_REG_PCIE1_CURRENT:
                if (!pcie1_read) {
                    ina3221_update_ch1(&ina2);
                    pcie1_v    = ina2.voltage[1];
                    pcie1_c    = ina2.current[1];
                    pcie1_read = true;
                }
                value = (reg == MB_INPUT_REG_PCIE1_VOLTAGE) ? pcie1_v : pcie1_c;
                break;

            case MB_INPUT_REG_PCIE2_VOLTAGE:
            case MB_INPUT_REG_PCIE2_CURRENT:
                if (!pcie2_read) {
                    ina3221_update_ch1(&ina1);
                    pcie2_v    = ina1.voltage[1];
                    pcie2_c    = ina1.current[1];
                    pcie2_read = true;
                }
                value = (reg == MB_INPUT_REG_PCIE2_VOLTAGE) ? pcie2_v : pcie2_c;
                break;

            case MB_INPUT_REG_USB1_VOLTAGE:
            case MB_INPUT_REG_USB1_CURRENT:
                if (!usb1_read) {
                    ina3221_update_ch0(&ina1);
                    usb1_v    = ina1.voltage[0];
                    usb1_c    = ina1.current[0];
                    usb1_read = true;
                }
                value = (reg == MB_INPUT_REG_USB1_VOLTAGE) ? usb1_v : usb1_c;
                break;

            case MB_INPUT_REG_USB2_VOLTAGE:
            case MB_INPUT_REG_USB2_CURRENT:
                if (!usb2_read) {
                    ina3221_update_ch2(&ina2);
                    usb2_v    = ina2.voltage[2];
                    usb2_c    = ina2.current[2];
                    usb2_read = true;
                }
                value = (reg == MB_INPUT_REG_USB2_VOLTAGE) ? usb2_v : usb2_c;
                break;

            case MB_INPUT_REG_INTERNAL5V_VOLTAGE:
            case MB_INPUT_REG_INTERNAL5V_CURRENT:
                if (!internal5v_read) {
                    ina3221_update_ch2(&ina1);
                    int5_v          = ina1.voltage[2];
                    int5_c          = ina1.current[2];
                    internal5v_read = true;
                }
                value = (reg == MB_INPUT_REG_INTERNAL5V_VOLTAGE) ? int5_v : int5_c;
                break;

            case MB_INPUT_REG_EXTERNAL5V_VOLTAGE:
            case MB_INPUT_REG_EXTERNAL5V_CURRENT:
                if (!external5v_read) {
                    ina3221_update_ch0(&ina2);
                    ext5_v          = ina2.voltage[0];
                    ext5_c          = ina2.current[0];
                    external5v_read = true;
                }
                value = (reg == MB_INPUT_REG_EXTERNAL5V_VOLTAGE) ? ext5_v : ext5_c;
                break;

            case MB_INPUT_REG_FAN_SPEED:
                value = get_fan_rpm();
                break;

            case MB_INPUT_REG_FIRMWARE_VERSION:
                value = FIRMWARE_VERSION;
                break;

            default:
                value = 0;
                break;
        }

        g_mb_input_regs[reg] = value;
        *pucRegBuffer++      = value >> 8;
        *pucRegBuffer++      = value & 0xFF;
    }
    return MB_ENOERR;
}

eMBErrorCode eMBRegHoldingCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode)
{
    uart_receiving_flag = 0;

    if (usAddress < 1 || usAddress + usNRegs - 1 > MB_HOLDING_REG_COUNT) return MB_ENOREG;

    if (eMode == MB_REG_READ) {
        for (USHORT i = 0; i < usNRegs; i++) {
            if ((usAddress + i) == MB_HOLD_REG_I2C_READ) {
                useri2c_read_byte();
            }
            uint16_t val    = g_mb_holding_regs[usAddress + i];
            *pucRegBuffer++ = (UCHAR)(val >> 8);
            *pucRegBuffer++ = (UCHAR)(val & 0xFF);
        }
    } else if (eMode == MB_REG_WRITE) {
        for (USHORT i = 0; i < usNRegs; i++) {
            USHORT regAddr = usAddress + i;
            uint16_t val   = ((uint16_t)pucRegBuffer[0] << 8) | pucRegBuffer[1];
            pucRegBuffer += 2;

            bool updateReg = false;

            // RGB映射区，直接写寄存器并触发标志
            if (regAddr >= MB_HOLD_REG_RGB1_ARM_START && regAddr <= MB_HOLD_REG_RGB1_ARM_END) {
                g_mb_holding_regs[regAddr] = val;
                set_rgb_arm_change_flag(true);
                continue;
            }

            // LCD RAM区，写寄存器并处理
            if (regAddr >= MB_HOLD_REG_LCD_RAM_START && regAddr <= MB_HOLD_REG_LCD_RAM_END) {
                size_t idx                 = (regAddr - MB_HOLD_REG_LCD_RAM_START) * 2;
                g_mb_holding_regs[regAddr] = val;
                set_lcd_arm_flag(true);
                continue;
            }

            // 其他寄存器按需校验
            switch (regAddr) {
                case MB_HOLD_REG_MAIN_POWER_DELAY:
                    if (system_power_off_time_check(val)) {
                        updateReg = true;
                    }
                    break;
                case MB_HOLD_REG_FAN_SPEED_PERCENT:
                    if (fan_pwm_check(val)) {
                        set_fan_pwm(val);
                        updateReg = true;
                    }
                    break;
                case MB_HOLD_REG_VDD_CPU_OFFSET:
                    if (vdd_core_offset_check(val)) {
                        set_page62_update_flag(true);
                        updateReg = true;
                    }
                    break;
                case MB_HOLD_REG_UART_BAUD_HIGH:
                    set_baudrate_high(val);
                    updateReg = true;
                    break;
                case MB_HOLD_REG_UART_BAUD_LOW:
                    set_baudrate_low(val);
                    updateReg = true;
                    break;
                case MB_HOLD_REG_WAKEUP_TIME_HIGH:
                    set_wakeup_time_high(val);
                    updateReg = true;
                    break;
                case MB_HOLD_REG_WAKEUP_TIME_LOW:
                    set_wakeup_time_low(val);
                    updateReg = true;
                    break;
                case MB_HOLD_REG_I2C_ADDR:
                    updateReg = true;
                    break;
                case MB_HOLD_REG_I2C_READ:
                    updateReg = true;
                    break;
                case MB_HOLD_REG_I2C_WRITE:
                    updateReg = false;
                    useri2c_write_byte((uint8_t)val);
                    break;
                case MB_HOLD_REG_RGB_COUNT:
                    if (rgb_num_check(val)) {
                        set_rgb_arm_change_flag(true);
                        updateReg = true;
                    }
                    break;
                case MB_HOLD_REG_RGB_MODE:
                    if (rgb_mode_check(val)) {
                        if (val != g_mb_holding_regs[MB_HOLD_REG_RGB_MODE]) {
                            set_rgb_mode_change_flag(true);
                        }
                        updateReg = true;
                    }
                    break;
                case MB_HOLD_REG_LCD_BRIGHTNESS:
                    if (val <= 0xFF && val != g_mb_holding_regs[MB_HOLD_REG_LCD_BRIGHTNESS]) {
                        ssd1306_set_contrast(&gs_handle, val);
                        updateReg       = true;
                    }
                    break;
                case MB_HOLD_REG_LCD_MODE:
                    if (lcd_mode_check(val)) {
                        if (val != g_mb_holding_regs[MB_HOLD_REG_LCD_MODE]) {
                            set_lcd_mode_flag(true);
                        }
                        updateReg = true;
                    }
                    break;
                case MB_HOLD_REG_LCD_ASSCI:
                    if (char_check(val)) {
                        set_char_update_flag(true);
                        buffer_put_char(val);
                        updateReg = true;
                    }
                    break;
                case MB_HOLD_REG_ETH0_IP_HIGH:
                    set_ip_high(0, val);
                    updateReg = true;
                    break;
                case MB_HOLD_REG_ETH0_IP_LOW:
                    set_ip_low(0, val);
                    updateReg = true;
                    break;
                case MB_HOLD_REG_ETH1_IP_HIGH:
                    set_ip_high(1, val);
                    updateReg = true;
                    break;
                case MB_HOLD_REG_ETH1_IP_LOW:
                    set_ip_low(1, val);
                    updateReg = true;
                    break;
                case MB_HOLD_REG_WLAN_IP_HIGH:
                    set_ip_high(2, val);
                    updateReg = true;
                    break;
                case MB_HOLD_REG_WLAN_IP_LOW:
                    set_ip_low(2, val);
                    updateReg = true;
                    break;
                default:
                    updateReg = true;
                    break;
            }

            if (updateReg) {
                g_mb_holding_regs[regAddr] = val;
            }
        }
    } else {
        return MB_EINVAL;
    }
    update_flash();    // 更新flash
    update_rgb_arm();  // 更新RGB
    return MB_ENOERR;
}

// MB_EINVAL
