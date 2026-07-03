/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#include "lcd.h"
#include "ip.h"
#include "string.h"
#include "ina3221.h"
#include "driver_ssd1306_advance.h"

/**
 * @brief Predefined bitmap for M5Stack LOGO display.
 * @brief M5Stack LOGO 位图数据
 */
const static uint8_t m5stack_logo[512] = {
    0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
    0xFF, 0xFF, 0x7E, 0xF8, 0xF0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xF0, 0xF8, 0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
    0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xC3, 0xC3, 0xC3, 0xC3, 0x83, 0x83, 0x03, 0x00, 0x00, 0x7C, 0xFE, 0xFF,
    0xFF, 0xC7, 0xC3, 0xC3, 0x83, 0x83, 0x83, 0x07, 0x0F, 0x03, 0x00, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0xFF,
    0xFF, 0xFF, 0xFF, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00, 0x80, 0xE0, 0xFC, 0xFF, 0x7F, 0x0F, 0x0F, 0x7F,
    0xFF, 0xFC, 0xE0, 0x80, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xF8, 0xFC, 0xFE, 0x1F, 0x0F, 0x07, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x07, 0x0F, 0x0F, 0x06, 0x04, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0xC0, 0xE0, 0xF0, 0xFC, 0x3E, 0x1F,
    0x0F, 0x07, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x01, 0x07,
    0x0F, 0x3F, 0x7E, 0x7E, 0x3F, 0x0F, 0x07, 0x01, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x80, 0xF0, 0xE1, 0xE1,
    0xC1, 0xC1, 0xC1, 0xC1, 0xC1, 0xE3, 0xFF, 0xFF, 0x7F, 0x3E, 0x00, 0xE0, 0xF0, 0xE1, 0xC1, 0xC3, 0xC3, 0xC3, 0xC3,
    0xC7, 0xE7, 0xFF, 0xFF, 0x7E, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xC0, 0xF0, 0xFE, 0xFF, 0x3F, 0x1F, 0x1D, 0x1C, 0x1C, 0x1C, 0x1C, 0x1D, 0x1F, 0x3F, 0xFF, 0xFE,
    0xF0, 0xC0, 0x00, 0x03, 0x1F, 0x3F, 0x7F, 0xF8, 0xF0, 0xE0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xE0, 0xF0, 0xF0, 0x60,
    0x40, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x07, 0x03, 0x07, 0x1F, 0x3F, 0x7E, 0xFC, 0xF0, 0xE0, 0xC0, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
    0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00};

volatile uint8_t lcd_initialized       = 0;
static volatile uint8_t lcd_power_flag = 1;

/**
 * @brief Flag to indicate whether ARM map needs refreshing.
 * @brief ARM 映射图刷新标志
 */
static bool lcd_arm_flag = false;

/**
 * @brief Flag to indicate whether LCD mode needs refreshing.
 * @brief LCD 模式刷新标志
 */
static bool lcd_mode_flag = true;

static uint32_t last_refresh_time = 0;
static uint8_t current_power_type = POWER_PCIE;

static uint32_t last_ip_switch_time = 0;
static uint8_t current_ip_index     = 0;
static uint8_t rotate_idx           = 0;

static char display_buf[SCREEN_ROWS][SCREEN_COLS];  // 每行存放字符
static uint8_t dirty_row[SCREEN_ROWS];              // 每行脏标记：0-不刷新，1-需要刷新
static uint8_t cur_row         = 0;                 // 当前写入行位置
static uint8_t cur_col         = 0;                 // 当前写入列位置
static uint8_t need_full_clear = 0;                 // 是否需要整屏清除
static uint8_t need_scroll     = 0;                 // 是否发生滚动动作
bool lcd_char_flag             = false;             // 字符模式刷新标志

extern ina3221_t ina1;
extern ina3221_t ina2;

static void clear_gram_buffer(void)
{
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 8; j++) {
            gs_handle.gram[i][j] = 0x00;
        }
    }
}

/**
 * @brief Draw the M5Stack log bitmap on the screen.
 * @brief 绘制 M5Stack LOGO 位图
 */
void draw_log(void)
{
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 128; j++) {
            gs_handle.gram[j][i] = m5stack_logo[i * 128 + j];
        }
    }
    ssd1306_gram_update(&gs_handle);
}

/**
 * @brief Convert IP address uint32_t to string format.
 *
 * @param ip IP address as uint32_t
 * @param out Output buffer for IP string (should be large enough)
 *
 * @brief 将 IP 地址从 uint32_t 转换为字符串
 */
static void ip_to_string(uint32_t ip, char *out)
{
    unsigned char b0 = (ip >> 24) & 0xFF;
    unsigned char b1 = (ip >> 16) & 0xFF;
    unsigned char b2 = (ip >> 8) & 0xFF;
    unsigned char b3 = ip & 0xFF;
    sprintf(out, "%d.%d.%d.%d", b0, b1, b2, b3);  // 改为 IPv4 标准
}

/**
 * @brief Display IP. Skip empty, if all empty show "IP NULL".
 *        顶部显示接口名(12号)，中间显示IP(16号)
 *
 * @param index 0=eth0, 1=eth1, 2=wlan
 */
void draw_ip(uint8_t index)
{
    const char *if_name[3] = {"ETH0", "ETH1", "WLAN"};
    char ip_str[32];

    /* IP NULL 全屏显示 */
    if (index == IP_NULL_INDEX) {
        const char *msg     = "IP NULL";
        int len             = strlen(msg);
        const int FONT_W_24 = 12;
        const int FONT_H_24 = 24;
        int x               = (128 - len * FONT_W_24) / 2;
        int y               = (32 - FONT_H_24) / 2;

        ssd1306_clear(&gs_handle);
        ssd1306_advance_string(x, y, (char *)msg, len, 1, SSD1306_FONT_24);
        return;
    }

    /* 防御性：index 必须在 0..2 */
    if (index >= IP_DISPLAY_COUNT) {
        return;
    }

    /* 获取 IP 字符串（若为 0 则为空串） */
    ip_to_string(get_ip(index), ip_str);

    /* 顶部显示接口名 (12号字体) */
    int name_len        = strlen(if_name[index]);
    const int FONT_W_12 = 6;
    int name_x          = (128 - name_len * FONT_W_12) / 2;
    int name_y          = 0;

    /* 中间显示 IP (16号字体)，若为空则显示 "NONE" */
    const char *show_ip_str = ip_str;
    char none_buf[]         = "NONE";
    if (ip_str[0] == '\0') {
        show_ip_str = none_buf;
    }

    int ip_len          = strlen(show_ip_str);
    const int FONT_W_16 = 8;
    const int FONT_H_16 = 16;
    int ip_x            = (128 - ip_len * FONT_W_16) / 2;
    int ip_y            = (32 - FONT_H_16) / 2 + (12 / 2);

    clear_gram_buffer();
    ssd1306_gram_write_string(&gs_handle, name_x, name_y, (char *)if_name[index], name_len, 1, SSD1306_FONT_12);
    ssd1306_gram_write_string(&gs_handle, ip_x, ip_y, (char *)show_ip_str, ip_len, 1, SSD1306_FONT_16);
    ssd1306_gram_update(&gs_handle);
}

/**
 * @brief Convert power type and index to readable string showing voltage and current.
 *
 * @param type Power type (PCIe, USB, System Peripheral)
 * @param index Index of the power source (0 or 1)
 * @param out Output string buffer
 *
 * @brief 根据电源类型和索引转换为电压电流字符串
 */
static void power_to_str(uint8_t type, uint8_t index, char *out)
{
    uint16_t voltage = 0;
    uint16_t current = 0;

    if (type == POWER_PCIE) {
        if (index == 0) {
            ina3221_update_ch1(&ina2);
            voltage = ina2.voltage[1];
            current = ina2.current[1];
            sprintf(out, "PCIE1:%dmV %dmA", voltage, current);
        } else {
            ina3221_update_ch1(&ina1);
            voltage = ina1.voltage[1];
            current = ina1.current[1];
            sprintf(out, "PCIE2:%dmV %dmA", voltage, current);
        }
    } else if (type == POWER_USB) {
        if (index == 0) {
            ina3221_update_ch0(&ina1);
            voltage = ina1.voltage[0];
            current = ina1.current[0];
            sprintf(out, "USB1:%dmV %dmA", voltage, current);
        } else {
            ina3221_update_ch2(&ina2);
            voltage = ina2.voltage[2];
            current = ina2.current[2];
            sprintf(out, "USB2:%dmV %dmA", voltage, current);
        }
    } else if (type == POWER_SYS_PER) {
        if (index == 0) {
            ina3221_update_ch2(&ina1);
            voltage = ina1.voltage[2];
            current = ina1.current[2];
            sprintf(out, "INT:%dmV %dmA", voltage, current);
        } else {
            ina3221_update_ch0(&ina2);
            voltage = ina2.voltage[0];
            current = ina2.current[0];
            sprintf(out, "EXT:%dmV %dmA", voltage, current);
        }
    }
}

/**
 * @brief Draw power voltage/current info on LCD, centered vertically and horizontally.
 *
 * @param type Power type to display
 *
 * @brief 绘制电压电流信息，居中显示
 */
static void draw_power(uint8_t type)
{
    char str1[32], str2[32];
    power_to_str(type, 0, str1);
    power_to_str(type, 1, str2);

    /* 使用字体 12 重新设置宽度高度 */
    const int font_w  = 6;   // 12号字体等宽 6px
    const int font_h  = 12;  // 字体高度 12px
    const int spacing = 5;

    int len1 = strlen(str1);
    int len2 = strlen(str2);

    int str1_width = len1 * font_w;
    int str2_width = len2 * font_w;

    int scr_width  = 128;
    int scr_height = 32;

    /* 总高度：两行字体高度 + 行间距 */
    int total_text_height = 2 * font_h + spacing;
    int start_y           = (scr_height - total_text_height) / 2;

    /* 最大宽度用于水平居中 */
    int max_width = (str1_width > str2_width) ? str1_width : str2_width;
    int x_align   = (scr_width - max_width) / 2;

    int y1 = start_y;
    int y2 = y1 + font_h + spacing;

    clear_gram_buffer();

    /* 第一行 */
    ssd1306_gram_write_string(&gs_handle, x_align, y1, str1, len1, 1, SSD1306_FONT_12);

    /* 第二行 */
    ssd1306_gram_write_string(&gs_handle, x_align, y2, str2, len2, 1, SSD1306_FONT_12);

    ssd1306_gram_update(&gs_handle);
}

/**
 * @brief Draw ARM bitmap image on LCD.
 * @brief 绘制 ARM 映射图
 */
static void draw_lcd_arm(void)
{
    uint8_t *src = (uint8_t *)&g_mb_holding_regs[MB_HOLD_REG_LCD_RAM_START];

    for (int page = 0; page < 4; page++) {
        for (int col = 0; col < 128; col++) {
            gs_handle.gram[col][page] = *src++;
        }
    }

    ssd1306_gram_update(&gs_handle);
}

/**
 * @brief Scroll the display buffer up by one row.
 * @brief 显示缓冲区上滚一行
 */
static void scroll_up(void)
{
    memcpy(display_buf[0], display_buf[1], SCREEN_COLS);
    memset(display_buf[1], ' ', SCREEN_COLS);
    dirty_row[0] = 1;
    dirty_row[1] = 1;
    need_scroll  = 1;
    cur_row      = SCREEN_ROWS - 1;
    cur_col      = 0;
}

/**
 * @brief Clear the display buffer and mark all rows as dirty.
 * @brief 清空显示缓冲区并标记所有行为脏
 */
static void buffer_clear(void)
{
    for (uint8_t r = 0; r < SCREEN_ROWS; r++) {
        memset(display_buf[r], ' ', SCREEN_COLS);
        dirty_row[r] = 1;
    }
    cur_row         = 0;
    cur_col         = 0;
    need_full_clear = 1;  // 标记整屏清空请求
}

/**
 * @brief Refresh the display based on dirty rows and flags.
 * @brief 根据脏行和标志刷新显示
 */
static void display_refresh(void)
{
    if (need_full_clear || need_scroll) {
        need_full_clear = 0;
        need_scroll     = 0;
        dirty_row[0]    = 1;
        dirty_row[1]    = 1;
    }

    if (dirty_row[0]) {
        ssd1306_gram_write_string(&gs_handle, 0, 0, display_buf[0], SCREEN_COLS, 1, SSD1306_FONT_16);
    }

    if (dirty_row[1]) {
        ssd1306_gram_write_string(&gs_handle, 0, 16, display_buf[1], SCREEN_COLS, 1, SSD1306_FONT_16);
    }

    if (dirty_row[0] || dirty_row[1]) {
        ssd1306_gram_update(&gs_handle);
        dirty_row[0] = 0;
        dirty_row[1] = 0;
        return;
    }
}

/**
 * @brief Put a character into the display buffer, handling special commands.
 *
 * @param ch Character to put (ASCII code)
 *
 * @brief 将字符放入显示缓冲区，处理特殊命令
 */
void buffer_put_char(uint16_t ch)
{
    if (ch == 0x7F)  // 清屏命令
    {
        buffer_clear();
        return;
    }

    if (ch == 0x0A)  // 换行
    {
        cur_row++;
        cur_col = 0;
        if (cur_row >= SCREEN_ROWS) {
            scroll_up();
        }
        return;
    }

    if (ch >= 0x20 && ch <= 0x7E) {
        // 当本行已经满了，先换下一行
        if (cur_col == SCREEN_COLS) {
            cur_row++;
            cur_col = 0;
            if (cur_row >= SCREEN_ROWS) {
                scroll_up();
            }
        }

        // 写入当前行（保证 cur_col 一定 < SCREEN_COLS）
        display_buf[cur_row][cur_col++] = (char)ch;
        dirty_row[cur_row]              = 1;
    }
}

/**
 * @brief Initialize the LCD hardware with retry.
 * @brief 初始化 LCD 硬件，失败重试 3 次
 */
void lcd_init(void)
{
    uint8_t i;
    uint8_t res;
    memset(display_buf, ' ', SCREEN_SIZE);  // 全部填充空格字符
    for (i = 0; i < 3; i++) {
        res = ssd1306_advance_init(SSD1306_INTERFACE_IIC, SSD1306_ADDR_SA0_0);
        if (res == 0) {
            lcd_initialized = 1;
            draw_log();
            return;
        }
    }
}

/**
 * @brief Check if a given LCD mode is valid.
 * @param mode The LCD mode to check
 * @return true if mode is valid; false otherwise
 * @brief 检查 LCD 模式是否合法
 */
bool lcd_mode_check(lcd_mode_t mode)
{
    return mode < LCD_MODE_COUNT;
}

/**
 * @brief Set LCD mode update flag.
 * @param flag true to request refresh; false to clear flag
 * @brief 设置 LCD 模式刷新标志
 */
void set_lcd_mode_flag(bool flag)
{
    lcd_mode_flag = flag;
}

/**
 * @brief Set ARM map update flag.
 * @param flag true to request ARM map refresh; false to clear flag
 * @brief 设置 ARM 映射图刷新标志
 */
void set_lcd_arm_flag(bool flag)
{
    lcd_arm_flag = flag;
}

/**
 * @brief Set character mode update flag.
 * @param flag true to request character mode refresh; false to clear flag
 * @brief 设置字符模式刷新标志
 */
void set_char_update_flag(bool flag)
{
    lcd_char_flag = flag;
}

/**
 * @brief Check if a character is valid for display.
 *
 * Valid characters are in the range 0x20 to 0x7E, or special commands 0x0A (newline) and 0x7F (clear).
 *
 * @param ch Character to check
 * @return true if character is valid; false otherwise
 *
 * @brief 检查字符是否合法
 */
bool char_check(uint16_t ch)
{
    if ((ch >= 0x20 && ch <= 0x7E) || ch == 0x0A || ch == 0x7F) {
        return true;
    }
    return false;
}

/**
 * @brief Update LCD display based on current mode and flags.
 *
 * Includes handling log, voltage/current, IP, and ARM map modes.
 *
 * @brief 根据当前模式和标志刷新 LCD 显示内容
 */
void update_lcd(void)
{
    if ((!lcd_initialized) || (!lcd_power_flag)) {
        return;
    }
    switch (g_mb_holding_regs[MB_HOLD_REG_LCD_MODE]) {
        case LCD_MODE_LOG:
            if (lcd_mode_flag) {
                lcd_mode_flag = false;
                draw_log();
            }
            break;

        case LCD_MODE_VOLTAGE_CURRENT:
            if (lcd_mode_flag) {
                lcd_mode_flag     = false;
                last_refresh_time = HAL_GetTick();
                draw_power(current_power_type);
            } else if (HAL_GetTick() - last_refresh_time >= VOLTAGE_CURRENT_REFRESH_INTERVAL_MS) {
                last_refresh_time  = HAL_GetTick();
                current_power_type = (current_power_type + 1) % POWER_DISPLAY_COUNT;
                draw_power(current_power_type);
            }
            break;

        case LCD_MODE_IP:
            uint32_t now = HAL_GetTick();

            /* 构建有效 IP 列表（只包含非 0 的索引） */
            uint8_t valid_list[IP_DISPLAY_COUNT];
            uint8_t valid_count = 0;
            for (uint8_t i = 0; i < IP_DISPLAY_COUNT; i++) {
                if (get_ip(i) != 0) {
                    valid_list[valid_count++] = i;
                }
            }

            /* 如果全部为空，直接显示 IP NULL */
            if (valid_count == 0) {
                if (lcd_mode_flag == false && get_ip_updated() == 0) {
                    break;
                }
                set_ip_updated(0);
                lcd_mode_flag       = false;
                last_ip_switch_time = now;
                current_ip_index    = 0;
                draw_ip(IP_NULL_INDEX);
                break;
            }

            /* 只有一个有效 IP → 固定显示该 IP（不轮播） */
            if (valid_count == 1) {
                if (get_ip_updated() == 0 && lcd_mode_flag == false) {
                    break;
                }
                set_ip_updated(0);
                lcd_mode_flag       = false;
                last_ip_switch_time = now;
                current_ip_index    = valid_list[0];
                draw_ip(current_ip_index);
                break;
            }

            /* valid_count == 2 或 3：需要轮播 */
            if (lcd_mode_flag || get_ip_updated()) {
                lcd_mode_flag = false;
                set_ip_updated(0);
                last_ip_switch_time = now;
                current_ip_index    = valid_list[0];
                rotate_idx          = 0;
                draw_ip(current_ip_index);
            } else if (now - last_ip_switch_time >= IP_SWITCH_INTERVAL_MS) {
                last_ip_switch_time = now;
                rotate_idx          = (rotate_idx + 1) % valid_count;
                current_ip_index    = valid_list[rotate_idx];
                draw_ip(current_ip_index);
            }
            break;

        case LCD_MODE_ARM_MAP:
            if (lcd_arm_flag || lcd_mode_flag) {
                lcd_mode_flag = false;
                lcd_arm_flag  = false;
                draw_lcd_arm();
            }
            break;
        case LCD_MODE_CHAR:
            if (lcd_mode_flag) {
                lcd_mode_flag = false;
                clear_gram_buffer();
                set_char_update_flag(true);
                dirty_row[0] = 1;
                dirty_row[1] = 1;
            }
            if (lcd_char_flag) {
                lcd_char_flag = false;
                display_refresh();
            }
            break;
        default:
            break;
    }
}

void lcd_power_off(void)
{
    lcd_power_flag = 0;
    if ((!lcd_initialized)) {
        return;
    }
    clear_gram_buffer();
    ssd1306_gram_update(&gs_handle);
}

void lcd_power_on(void)
{
    lcd_power_flag = 1;
    set_lcd_mode_flag(true);
}
