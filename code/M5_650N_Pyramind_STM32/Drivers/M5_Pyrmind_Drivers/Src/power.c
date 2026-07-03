#include "power.h"
#include "max77812.h"

extern __IO uint32_t power_turn_off_time;

/******************************* PMIC 基础控制命令 *****************************/
/* 复位命令数组 [寄存器地址, 值] */
const uint8_t pmic_reset_cmd[2] = {
    0x00,  // 复位寄存器地址
    0x01   // 写入1触发软件复位
};

/* 关闭所有输出命令数组 [寄存器地址, 值] */
const uint8_t pmic_disable_all_cmd[2] = {
    0x06,  // 使能控制寄存器地址
    0x00   // 关闭全部输出
};

/******************************* PMIC1 配置 (2+2 Phase) **********************/
/* VDD Core (M1) - 0.8V */
const uint8_t pmic1_vdd_core_buffr[2] = {
    0x23,  // M1输出电压寄存器 (PMIC_REG_OUTPUT_CTRL + 0x03)
    0x6E   // (0.8-0.25)/0.005 = 0x6E
};

/* NNVDD (M3) - 0.6V */
const uint8_t pmic1_nnvdd_buffr[2] = {
    0x25,  // M3输出电压寄存器 (PMIC_REG_OUTPUT_CTRL + 0x05)
    0x46   // (0.6-0.25)/0.005 = 0x46
};

///* NNVDD (M3) - 0.65V */
// const uint8_t pmic1_nnvdd_buffr[2] = {
//     0x25,  // M3输出电压寄存器 (PMIC_REG_OUTPUT_CTRL + 0x05)
//     0x50   // (0.65-0.25)/0.005 = 0x50
// };

/******************************* PMIC2 配置 (2+1+1 Phase) ********************/
/* VDD CPU (M1) - 1.0V */
const uint8_t pmic2_vdd_cpu_buffr[2] = {
    0x23,  // M1输出电压寄存器
    0x96   // (1.0-0.25)/0.005 = 0x96
};

///* VDD CPU (M1) - 1.05V */
// const uint8_t pmic2_vdd_cpu_buffr[2] = {
//     0x23,  // M1输出电压寄存器
//     0xA0   // (1.05-0.25)/0.005 = 0xA0
// };

/* VDDR 1V1 (M3) - 1.1V */
const uint8_t pmic2_vddr_1v1_buffr[2] = {
    0x25,  // M3输出电压寄存器
    0xAA   // (1.1-0.25)/0.005 = 0xAA
};

/* VDDR 0V6 (M4) - 0.6V */
const uint8_t pmic2_vddr_0v6_buffr[2] = {
    0x26,  // M4输出电压寄存器
    0x46   // (0.6-0.25)/0.005 = 0x46
};

///* VDDR (M3) - 1.15V */
// const uint8_t pmic2_vddr_1v1_buffr[2] = {
//     0x25,  // M3输出电压寄存器
//     180   // (1.15-0.25)/0.005 = 180
// };
//
///* VDDR (M4) - 0.64V */
// const uint8_t pmic2_vddr_0v6_buffr[2] = {
//     0x26,  // M4输出电压寄存器
//     78   // (0.64-0.25)/0.005 = 78
// };

static bool vdd_cpu_offset_update_flag           = false;
static bool system_main_power_update_status      = false;
static bool peripheral_main_power_update_status  = false;
static bool peripheral_aux_power_update_status   = false;
static bool power_update_status                  = false;
static bool power_turn_off_ing                   = false;
static power_wakeup_status_t power_wakeup_status = POWER_WAKEUP_STATUS_NONE;

bool peripheral_aux_power_update_flag[MB_COIL_COUNT + 1] = {false};

extern __IO uint8_t page60_buffer[16];

static HAL_StatusTypeDef max77812_write_and_verify(max77812_pmic_t pmic, uint8_t reg, uint8_t value)
{
    uint8_t read_val = 0;
    HAL_StatusTypeDef status;

    while (1) {
        status = max77812_write_reg(pmic, reg, value);
        if (status != HAL_OK) {
#ifdef POWER_DEBUG
            printf("PMIC%d WRITE 0x%02X ERROR\r\n", pmic, reg);
#endif
            HAL_Delay(10);
            continue;
        }

        status = max77812_read_reg(pmic, reg, &read_val);
        if (status != HAL_OK) {
#ifdef POWER_DEBUG
            printf("PMIC%d READ 0x%02X ERROR\r\n", pmic, reg);
#endif
            HAL_Delay(10);
            continue;
        }

        if (read_val == value) {
#ifdef POWER_DEBUG
            printf("PMIC%d VERIFY SUCCESE REG = 0x%02X: WRITE 0x%02X == READ 0x%02X\r\n", pmic + 1, reg, value,
                   read_val);
#endif
            break;  // 写入成功并校验通过
        }

#ifdef POWER_DEBUG
        printf("PMIC%d VERIFY FAIL: WRITE 0x%02X != READ 0x%02X\r\n", pmic, value, read_val);
#endif
        HAL_Delay(10);
    }

    return HAL_OK;
}

static void wait_pmic_pok(max77812_pmic_t pmic)
{
    uint8_t read_val = 0;
    HAL_StatusTypeDef status;
    while (1) {
        status = max77812_read_reg(pmic, PMIC_POK_STATUS_REGISTER, &read_val);
        if (status == HAL_OK && read_val == 0) {
#ifdef POWER_DEBUG
            printf("PMIC%d POK\r\n", pmic + 1);
#endif
            break;
        }
        HAL_Delay(3);
    }
}

static void update_gpio_if_needed(uint16_t coil_index, GPIO_TypeDef *port, uint16_t pin, GPIO_PinState active_state)
{
    if (peripheral_aux_power_update_flag[coil_index]) {
        peripheral_aux_power_update_flag[coil_index] = false;
        HAL_GPIO_WritePin(port, pin, g_mb_coils[coil_index] ? active_state : !active_state);
    }
}

//* tage 1： 系统上电
//* 打开 **内部5V降压使能**， 等待10ms
//* 打开 **PMIC1 使能**和**PMIC2 使能**，并初始化电压配置
//* 打开 **1.8V降压使能**，等待 **1.8V PowerGood**，并等待1ms
//* 打开 **3.3V降压使能**，等待 **3.3V PowerGood**
//* 打开 **DDRV1V1**和**DDRV0V6**，并等待PG
//* 打开 **VDD CORE**，并等待PG
//* 打开 **VDD CPU**，并等待PG
//* 打开 **VDD NN**，并等待PG
//* 等待10ms，拉高 **SoC复位输出**
//* 打开风扇PWM
static void _system_power_sequence(bool boot_mode)
{
    uint16_t vdd_cpu_value = ((1000 + (g_mb_holding_regs[MB_HOLD_REG_VDD_CPU_OFFSET] - 20) * 5 - 250) / 5);  // VDD CPU
    // 打开 **内部5V降压使能**， 等待10ms
    HAL_GPIO_WritePin(V5V_PWR_EN_GPIO_Port, V5V_PWR_EN_Pin, GPIO_PIN_SET);  // 拉高 PA15
    HAL_Delay(100);

    // 打开 **PMIC1 使能**和**PMIC2 使能**，并初始化电压配置
    HAL_GPIO_WritePin(PMIC1_CE_GPIO_Port, PMIC1_CE_Pin, GPIO_PIN_SET);  // 拉高 PA4
    HAL_GPIO_WritePin(PMIC2_CE_GPIO_Port, PMIC2_CE_Pin, GPIO_PIN_SET);  // 拉高 PA1

    while (max77812_write_reg(MAX77812_PMIC1, pmic_reset_cmd[0], pmic_reset_cmd[1]) !=
           HAL_OK) {  // 初始化 PMIC1（复位）
#ifdef POWER_DEBUG
        printf("MAX77812_PMIC1 RESET ERROR\r\n");
#endif
        HAL_Delay(10);
    }

    max77812_write_and_verify(MAX77812_PMIC1, pmic_disable_all_cmd[0], pmic_disable_all_cmd[1]);  // 关闭输出
    max77812_write_and_verify(MAX77812_PMIC1, pmic1_vdd_core_buffr[0], pmic1_vdd_core_buffr[1]);  // VDD Core
    max77812_write_and_verify(MAX77812_PMIC1, pmic1_nnvdd_buffr[0], pmic1_nnvdd_buffr[1]);        // NNVDD

    while (max77812_write_reg(MAX77812_PMIC2, pmic_reset_cmd[0], pmic_reset_cmd[1]) !=
           HAL_OK) {  // 初始化 PMIC2（复位）
#ifdef POWER_DEBUG
        printf("MAX77812_PMIC2 RESET ERROR\r\n");
#endif
        HAL_Delay(10);
    }
    max77812_write_and_verify(MAX77812_PMIC2, pmic_disable_all_cmd[0], pmic_disable_all_cmd[1]);  // 关闭输出
    if (vdd_cpu_value > 170 || vdd_cpu_value < 130) {  // 防止意外使用默认的1.0V
        max77812_write_and_verify(MAX77812_PMIC2, pmic2_vdd_cpu_buffr[0], pmic2_vdd_cpu_buffr[1]);  // VDD CPU
    } else {
        max77812_write_and_verify(MAX77812_PMIC2, pmic2_vdd_cpu_buffr[0], vdd_cpu_value);  // VDD CPU
    }
    max77812_write_and_verify(MAX77812_PMIC2, pmic2_vddr_1v1_buffr[0], pmic2_vddr_1v1_buffr[1]);  // VDDR 1V1
    max77812_write_and_verify(MAX77812_PMIC2, pmic2_vddr_0v6_buffr[0], pmic2_vddr_0v6_buffr[1]);  // VDDR 0V6

    // 打开 **1.8V降压使能**，等待 **1.8V PowerGood**，并等待1ms
    HAL_GPIO_WritePin(VDD18_EN_GPIO_Port, VDD18_EN_Pin, GPIO_PIN_SET);           // 拉高 PB1
    while (HAL_GPIO_ReadPin(VDD18_PG_GPIO_Port, VDD18_PG_Pin) == GPIO_PIN_SET);  // 等待 PF1 稳定高
    HAL_Delay(10);

    // 打开 **3.3V降压使能**，等待 **3.3V PowerGood**，并等待1ms
    HAL_GPIO_WritePin(VDD3_EN_GPIO_Port, VDD3_EN_Pin, GPIO_PIN_SET);           // 拉高 PB0
    while (HAL_GPIO_ReadPin(VDD3_PG_GPIO_Port, VDD3_PG_Pin) == GPIO_PIN_SET);  // 等待 PB6 稳定高
    HAL_Delay(10);

    // 打开 **DDRV1V1**和**DDRV0V6**，并等待PG
    max77812_write_and_verify(MAX77812_PMIC2, PMIC_REG_EN_CTRL, DDRV1V1_PHASE | DDRV0V6_PHASE);
    HAL_Delay(1);
    wait_pmic_pok(MAX77812_PMIC2);

    // 打开 **VDD CORE**，并等待PG
    max77812_write_and_verify(MAX77812_PMIC1, PMIC_REG_EN_CTRL, VDD_CORE_PHASE);
    HAL_Delay(1);
    wait_pmic_pok(MAX77812_PMIC1);

    // 打开 **VDD CPU**，并等待PG
    max77812_write_and_verify(MAX77812_PMIC2, PMIC_REG_EN_CTRL, DDRV1V1_PHASE | DDRV0V6_PHASE | VDD_CPU_PHASE);
    HAL_Delay(1);
    wait_pmic_pok(MAX77812_PMIC2);

    // 打开 **VDD NN**，并等待PG
    max77812_write_and_verify(MAX77812_PMIC1, PMIC_REG_EN_CTRL, VDD_CORE_PHASE | VDD_NN_PHASE);
    HAL_Delay(1);
    wait_pmic_pok(MAX77812_PMIC1);

    // 等待10ms
    HAL_Delay(10);

    // SoC复位逻辑
    if (boot_mode) {
        if (HAL_GPIO_ReadPin(EC_BUTTON_GPIO_Port, EC_BUTTON_Pin) == GPIO_PIN_RESET) {
            HAL_GPIO_WritePin(DL_MODE_GPIO_Port, DL_MODE_Pin, GPIO_PIN_SET);
            HAL_Delay(100);
            HAL_GPIO_WritePin(SYS_RST_GPIO_Port, SYS_RST_Pin, GPIO_PIN_SET);
            HAL_Delay(500);
            HAL_GPIO_WritePin(DL_MODE_GPIO_Port, DL_MODE_Pin, GPIO_PIN_RESET);
            g_mb_holding_regs[MB_HOLD_REG_RGB_MODE] = RGB_MODE_DOWNLOAD;  // 上电进入下载模式配置对应的灯效
        } else {
            HAL_GPIO_WritePin(SYS_RST_GPIO_Port, SYS_RST_Pin, GPIO_PIN_SET);
        }
    } else {
        HAL_GPIO_WritePin(SYS_RST_GPIO_Port, SYS_RST_Pin, GPIO_PIN_SET);
    }

    set_fan_pwm(g_mb_holding_regs[MB_HOLD_REG_FAN_SPEED_PERCENT]);

    // HAL_Delay(100);
    // uint8_t read_val = 0;
    // HAL_StatusTypeDef status;
    // status = max77812_read_reg(MAX77812_PMIC1, PMIC_POK_STATUS_REGISTER, &read_val);
    // if (status == HAL_OK) {
    //     printf("PMIC1 POK register: 0x%02x var: 0x%02x\n", PMIC_POK_STATUS_REGISTER, read_val);
    // }
    // status = max77812_read_reg(MAX77812_PMIC2, PMIC_POK_STATUS_REGISTER, &read_val);
    // if (status == HAL_OK) {
    //     printf("PMIC2 POK register: 0x%02x var: 0x%02x\n", PMIC_POK_STATUS_REGISTER, read_val);
    // }
}

void system_power_boot(void)
{
    _system_power_sequence(true);
}

void system_power_on(void)
{
    _system_power_sequence(false);
}

void system_power_off(void)
{
    HAL_GPIO_WritePin(SYS_RST_GPIO_Port, SYS_RST_Pin, GPIO_PIN_RESET);  // 拉低关闭
    HAL_Delay(1);

    // 关闭 **3.3V降压使能**
    HAL_GPIO_WritePin(VDD3_EN_GPIO_Port, VDD3_EN_Pin, GPIO_PIN_RESET);  // 拉低 PB0
    HAL_Delay(1);

    // 关闭 **1.8V降压使能**
    HAL_GPIO_WritePin(VDD18_EN_GPIO_Port, VDD18_EN_Pin, GPIO_PIN_RESET);  // 拉低 PB1
    HAL_Delay(1);

    max77812_write_and_verify(MAX77812_PMIC2, pmic_disable_all_cmd[0], pmic_disable_all_cmd[1]);  // 关闭输出
    max77812_write_and_verify(MAX77812_PMIC1, pmic_disable_all_cmd[0], pmic_disable_all_cmd[1]);  // 关闭输出
    HAL_Delay(1);

    // 关闭 **PMIC1 使能**和**PMIC2 使能**
    HAL_GPIO_WritePin(PMIC1_CE_GPIO_Port, PMIC1_CE_Pin, GPIO_PIN_RESET);  // 拉低 PA4
    HAL_GPIO_WritePin(PMIC2_CE_GPIO_Port, PMIC2_CE_Pin, GPIO_PIN_RESET);  // 拉低 PA1
    HAL_Delay(1);

    // 关闭 **内部5V降压使能**， 等待10ms
    HAL_GPIO_WritePin(V5V_PWR_EN_GPIO_Port, V5V_PWR_EN_Pin, GPIO_PIN_RESET);  // 拉低 PA15
    HAL_Delay(1);

    set_fan_pwm(0);
}

//* Stage 2: 外设上电
//* 打开 **外部5V降压使能**，等待10ms
//* 拉高 **USB3.0 HUB复位**
//* 拉低 **USB3.0 DS1口输出使能**，**USB3.0 DS2口输出使能**和**USB3.0 DS3口输出使能**
//* 拉高 **USB3.0 DS1口大电流模式**和**USB3.0 DS2口大电流模式**
//* 拉低 **Grove Port1输出使能**和**Grove Port3输出使能**
//* 拉高 **NGFF PCIE时钟使能**，并等待10ms
//* 拉高 **NGFF1口输出使能**和**NGFF2口输出使能**
void peripheral_power_boot(void)
{
    //* 打开 **外部5V降压使能**，等待10ms
    if (!g_mb_coils[MB_COIL_EXT_5V_SUPPLY]) {
        return;
    }
    HAL_GPIO_WritePin(V5VE_PWR_EN_GPIO_Port, V5VE_PWR_EN_Pin, GPIO_PIN_SET);  // 拉高 PA11
    HAL_Delay(100);

    //* 拉高 **USB3.0 HUB复位**
    if (g_mb_coils[MB_COIL_GL3510_RESET]) {
        HAL_GPIO_WritePin(GL_RESET_J_GPIO_Port, GL_RESET_J_Pin, GPIO_PIN_SET);  // 拉高  PB11
    }

    //* 拉低 **USB3.0 DS1口输出使能**，**USB3.0 DS2口输出使能**和**USB3.0 DS3口输出使能**
    if (g_mb_coils[MB_COIL_DS1_POWER]) {
        HAL_GPIO_WritePin(DS1_PWR_EN_GPIO_Port, DS1_PWR_EN_Pin, GPIO_PIN_RESET);  // 拉低 PB13
    }

    if (g_mb_coils[MB_COIL_DS2_POWER]) {
        HAL_GPIO_WritePin(DS2_PWR_EN_GPIO_Port, DS2_PWR_EN_Pin, GPIO_PIN_RESET);  // 拉低 PB14
    }

    if (g_mb_coils[MB_COIL_DS3_POWER]) {
        HAL_GPIO_WritePin(DS3_PWR_EN_GPIO_Port, DS3_PWR_EN_Pin, GPIO_PIN_RESET);  // 拉低 PC11
    }

    //* 拉高 **USB3.0 DS1口大电流模式**和**USB3.0 DS2口大电流模式**
    if (g_mb_coils[MB_COIL_DS1_HIGH_CURRENT]) {
        HAL_GPIO_WritePin(DS1_HS_EN_GPIO_Port, DS1_HS_EN_Pin, GPIO_PIN_SET);  // 拉高 PB12
    }

    if (g_mb_coils[MB_COIL_DS2_HIGH_CURRENT]) {
        HAL_GPIO_WritePin(DS2_HS_EN_GPIO_Port, DS2_HS_EN_Pin, GPIO_PIN_SET);  // 拉高 PB15
    }

    //* 拉低 **Grove Port2输出使能**和**Grove Port3输出使能**
    if (g_mb_coils[MB_COIL_GROVE_PORT2_SUPPLY]) {
        HAL_GPIO_WritePin(PWR_GRV2_EN_GPIO_Port, PWR_GRV2_EN_Pin, GPIO_PIN_RESET);  // 拉低 PC14
    }

    if (g_mb_coils[MB_COIL_GROVE_PORT3_SUPPLY]) {
        HAL_GPIO_WritePin(PWR_GRV3_EN_GPIO_Port, PWR_GRV3_EN_Pin, GPIO_PIN_RESET);  // 拉低 PB7
    }

    //* 拉高 **NGFF PCIE时钟使能**，并等待10ms
    if (!g_mb_coils[MB_COIL_CLK_MODE_SELECT]) {
        HAL_GPIO_WritePin(PCIE_REFC_EN_GPIO_Port, PCIE_REFC_EN_Pin, GPIO_PIN_SET);  // 拉高 PB10
        HAL_Delay(10);
    }

    //* 拉高 **NGFF1口输出使能**和**NGFF2口输出使能**
    if (g_mb_coils[MB_COIL_PCIE1_POWER]) {
        HAL_GPIO_WritePin(NGFF1_PWR_EN_GPIO_Port, NGFF1_PWR_EN_Pin, GPIO_PIN_SET);  // 拉高 PB2
    }

    if (g_mb_coils[MB_COIL_PCIE2_POWER]) {
        HAL_GPIO_WritePin(NGFF2_PWR_EN_GPIO_Port, NGFF2_PWR_EN_Pin, GPIO_PIN_SET);  // 拉高 PB4
    }
}

void peripheral_power_on(void)
{
    peripheral_power_boot();
}

//* 外设下电
//* 关闭 **外部5V降压使能**，等待10ms
//* 拉低 **USB3.0 HUB复位**
//* 拉高 **USB3.0 DS1口输出使能**，**USB3.0 DS2口输出使能**和**USB3.0 DS3口输出使能**
//* 拉低 **USB3.0 DS1口大电流模式**和**USB3.0 DS2口大电流模式**
//* 拉高 **Grove Port1输出使能**和**Grove Port3输出使能**
//* 拉低 **NGFF PCIE时钟使能**，并等待10ms
//* 拉低 **NGFF1口输出使能**和**NGFF2口输出使能**
void peripheral_power_off(void)
{
    //* 关闭 **外部5V降压使能**，等待10ms
    HAL_GPIO_WritePin(V5VE_PWR_EN_GPIO_Port, V5VE_PWR_EN_Pin, GPIO_PIN_RESET);  // 拉低 PA11
    HAL_Delay(10);

    // * 拉低 **USB3.0 HUB复位**
    HAL_GPIO_WritePin(GL_RESET_J_GPIO_Port, GL_RESET_J_Pin, GPIO_PIN_RESET);  // 拉低 PB11

    //* 拉高 **USB3.0 DS1口输出使能**，**USB3.0 DS2口输出使能**和**USB3.0 DS3口输出使能**
    HAL_GPIO_WritePin(DS1_PWR_EN_GPIO_Port, DS1_PWR_EN_Pin, GPIO_PIN_SET);  // 拉高 PB13
    HAL_GPIO_WritePin(DS2_PWR_EN_GPIO_Port, DS2_PWR_EN_Pin, GPIO_PIN_SET);  // 拉高 PB14
    HAL_GPIO_WritePin(DS3_PWR_EN_GPIO_Port, DS3_PWR_EN_Pin, GPIO_PIN_SET);  // 拉高 PC11

    //* 拉低 **USB3.0 DS1口大电流模式**和**USB3.0 DS2口大电流模式**
    HAL_GPIO_WritePin(DS1_HS_EN_GPIO_Port, DS1_HS_EN_Pin, GPIO_PIN_RESET);  // 拉低 PB12
    HAL_GPIO_WritePin(DS2_HS_EN_GPIO_Port, DS2_HS_EN_Pin, GPIO_PIN_RESET);  // 拉低 PB15

    // * 拉高 **Grove Port2输出使能**和**Grove Port3输出使能**
    HAL_GPIO_WritePin(PWR_GRV2_EN_GPIO_Port, PWR_GRV2_EN_Pin, GPIO_PIN_SET);  // 拉高 PC14
    HAL_GPIO_WritePin(PWR_GRV3_EN_GPIO_Port, PWR_GRV3_EN_Pin, GPIO_PIN_SET);  // 拉高 PB7

    //* 拉低 **NGFF PCIE时钟使能**，并等待10ms
    HAL_GPIO_WritePin(PCIE_REFC_EN_GPIO_Port, PCIE_REFC_EN_Pin, GPIO_PIN_RESET);  // 拉低 PB10
    HAL_Delay(10);

    //* 拉低 **NGFF1口输出使能**和**NGFF2口输出使能**
    HAL_GPIO_WritePin(NGFF1_PWR_EN_GPIO_Port, NGFF1_PWR_EN_Pin, GPIO_PIN_RESET);  // 拉低 PB2
    HAL_GPIO_WritePin(NGFF2_PWR_EN_GPIO_Port, NGFF2_PWR_EN_Pin, GPIO_PIN_RESET);  // 拉低 PB4
}

bool system_power_off_time_check(uint16_t power_off_timer)
{
    if (power_off_timer >= SYSTEM_POWER_ON_TIMER_MIN) {
        return true;
    }
    return false;
}

bool vdd_core_offset_check(uint16_t vdd_core)
{
    if (vdd_core <= VDD_CPU_OFFSET_MAX) {
        return true;
    }
    return false;
}

void set_system_main_power_update_status(bool status)
{
    system_main_power_update_status = status;
}

void set_peripheral_main_power_update_status(bool status)
{
    peripheral_main_power_update_status = status;
}

void set_peripheral_aux_power_update_status(bool status)
{
    peripheral_aux_power_update_status = status;
}

void set_power_update_status(bool status)
{
    power_update_status = status;
}

void system_power_update(void)
{
    static bool waiting_for_power_off    = false;
    static uint32_t power_off_tick_start = 0;

    if (system_main_power_update_status) {
        system_main_power_update_status = false;

        if (!g_mb_coils[MB_COIL_INT_5V_MAIN_POWER]) {
            waiting_for_power_off = true;
            power_off_tick_start  = HAL_GetTick();
        }
    }

    if (waiting_for_power_off) {
        if (g_mb_coils[MB_COIL_INT_5V_MAIN_POWER] || power_turn_off_ing) {
            waiting_for_power_off = false;
            return;
        }

        if ((HAL_GetTick() - power_off_tick_start) >= g_mb_holding_regs[MB_HOLD_REG_MAIN_POWER_DELAY]) {
            waiting_for_power_off = false;
            rgb_power_off();
            lcd_power_off();
            system_power_off();
            set_power_wakeup_status(POWER_WAKEUP_STATUS_SYSTEM);
            btn_top_power_flag  = 0;
            power_turn_off_time = HAL_GetTick();
        }
    }
}

void peripheral_power_update(void)
{
    if (peripheral_main_power_update_status) {
        peripheral_main_power_update_status = false;
        if (g_mb_coils[MB_COIL_EXT_5V_SUPPLY]) {
            get_page60_buffer();
            for (uint8_t i = 4; i <= 14; i++) {
                g_mb_coils[i] = page60_buffer[i];
            }
            g_mb_coils[MB_COIL_CLK_MODE_SELECT] = 0;
            peripheral_power_on();
        } else {
            for (uint8_t i = 3; i <= 14; i++) {
                g_mb_coils[i] = 0;
            }
            peripheral_power_off();
        }
    }

    if (peripheral_aux_power_update_status) {
        peripheral_aux_power_update_status = false;
        if (g_mb_coils[MB_COIL_EXT_5V_SUPPLY]) {
            // GL3510 HUB Reset - 拉高为有效
            update_gpio_if_needed(MB_COIL_GL3510_RESET, GL_RESET_J_GPIO_Port, GL_RESET_J_Pin, GPIO_PIN_SET);

            // USB3.0 DS口输出使能 - 拉低为有效
            update_gpio_if_needed(MB_COIL_DS1_POWER, DS1_PWR_EN_GPIO_Port, DS1_PWR_EN_Pin, GPIO_PIN_RESET);
            update_gpio_if_needed(MB_COIL_DS2_POWER, DS2_PWR_EN_GPIO_Port, DS2_PWR_EN_Pin, GPIO_PIN_RESET);
            update_gpio_if_needed(MB_COIL_DS3_POWER, DS3_PWR_EN_GPIO_Port, DS3_PWR_EN_Pin, GPIO_PIN_RESET);

            // USB3.0 大电流模式 - 拉高有效
            update_gpio_if_needed(MB_COIL_DS1_HIGH_CURRENT, DS1_HS_EN_GPIO_Port, DS1_HS_EN_Pin, GPIO_PIN_SET);
            update_gpio_if_needed(MB_COIL_DS2_HIGH_CURRENT, DS2_HS_EN_GPIO_Port, DS2_HS_EN_Pin, GPIO_PIN_SET);

            // Grove Port 供电 - 拉低有效
            update_gpio_if_needed(MB_COIL_GROVE_PORT2_SUPPLY, PWR_GRV2_EN_GPIO_Port, PWR_GRV2_EN_Pin, GPIO_PIN_RESET);
            update_gpio_if_needed(MB_COIL_GROVE_PORT3_SUPPLY, PWR_GRV3_EN_GPIO_Port, PWR_GRV3_EN_Pin, GPIO_PIN_RESET);

            // NGFF PCIE时钟控制（特殊处理）
            if (peripheral_aux_power_update_flag[MB_COIL_CLK_MODE_SELECT] ||
                peripheral_aux_power_update_flag[MB_COIL_PCIE_CLK_OUTPUT]) {
                peripheral_aux_power_update_flag[MB_COIL_CLK_MODE_SELECT] = false;
                peripheral_aux_power_update_flag[MB_COIL_PCIE_CLK_OUTPUT] = false;
                bool clk_mode                                             = g_mb_coils[MB_COIL_CLK_MODE_SELECT];
                bool clk_out                                              = g_mb_coils[MB_COIL_PCIE_CLK_OUTPUT];
                if (!clk_mode || (clk_mode && clk_out)) {
                    HAL_GPIO_WritePin(PCIE_REFC_EN_GPIO_Port, PCIE_REFC_EN_Pin, GPIO_PIN_SET);
                } else {
                    HAL_GPIO_WritePin(PCIE_REFC_EN_GPIO_Port, PCIE_REFC_EN_Pin, GPIO_PIN_RESET);
                }
                HAL_Delay(10);  // 等待稳定
            }

            // NGFF供电控制 - 拉高有效
            update_gpio_if_needed(MB_COIL_PCIE1_POWER, NGFF1_PWR_EN_GPIO_Port, NGFF1_PWR_EN_Pin, GPIO_PIN_SET);
            update_gpio_if_needed(MB_COIL_PCIE2_POWER, NGFF2_PWR_EN_GPIO_Port, NGFF2_PWR_EN_Pin, GPIO_PIN_SET);
        } else {
            for (uint8_t i = 3; i <= 14; i++) {
                g_mb_coils[i]                       = 0;
                peripheral_aux_power_update_flag[i] = false;
            }
        }
    }
}

void power_update(void)
{
    static bool waiting_for_power_off    = false;
    static uint32_t power_off_tick_start = 0;

    if (power_update_status) {
        g_mb_coils[MB_COIL_POWER_OFF] = 0;
        power_update_status           = false;
        waiting_for_power_off         = true;
        power_turn_off_ing            = true;
        power_off_tick_start          = HAL_GetTick();
    }

    if (waiting_for_power_off) {
        if ((HAL_GetTick() - power_off_tick_start) >= g_mb_holding_regs[MB_HOLD_REG_MAIN_POWER_DELAY]) {
            waiting_for_power_off = false;
            power_turn_off_ing    = false;
            for (uint8_t i = 1; i <= 14; i++) {
                g_mb_coils[i] = 0;
            }
            rgb_power_off();
            lcd_power_off();
            peripheral_power_off();
            system_power_off();
            set_power_wakeup_status(POWER_WAKEUP_STATUS_SYSTEM_PERI);
            btn_top_power_flag  = 0;
            power_turn_off_time = HAL_GetTick();
        }
        system_main_power_update_status = false;
    }
}

void set_power_wakeup_status(power_wakeup_status_t status)
{
    power_wakeup_status = status;
}

power_wakeup_status_t get_power_wakeup_status(void)
{
    return power_wakeup_status;
}
