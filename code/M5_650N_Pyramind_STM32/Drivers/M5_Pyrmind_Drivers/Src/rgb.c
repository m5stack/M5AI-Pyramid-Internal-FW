/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#include <rgb.h>
#include <stdlib.h>
#include "string.h"
#include "demo.h"

static volatile uint8_t rgb_power                                 = 1;
static uint16_t *color_buf                                        = NULL;
static bool rgb_arm_change_flag                                   = false;
static bool rgb_mode_change_flag                                  = false;
static volatile bool rgb_output_busy                              = false;
static const uint8_t gradient_colors[RGB_GRADIENT_COLOR_COUNT][3] = {{255, 0, 0}, {255, 165, 0}, {255, 255, 0},
                                                                     {0, 255, 0}, {0, 0, 255},   {128, 0, 128}};

static inline void hsv_to_rgb(uint16_t h, uint8_t s, uint8_t v, uint8_t *r, uint8_t *g, uint8_t *b)
{
    h %= 360;
    uint8_t region     = h / 60;
    uint16_t remainder = (h % 60) * 255 / 60;
    uint8_t p          = (v * (255 - s)) / 255;
    uint8_t q          = (v * (255 - (s * remainder) / 255)) / 255;
    uint8_t t          = (v * (255 - (s * (255 - remainder)) / 255)) / 255;
    switch (region) {
        case 0:
            *r = v;
            *g = t;
            *b = p;
            break;
        case 1:
            *r = q;
            *g = v;
            *b = p;
            break;
        case 2:
            *r = p;
            *g = v;
            *b = t;
            break;
        case 3:
            *r = p;
            *g = q;
            *b = v;
            break;
        case 4:
            *r = t;
            *g = p;
            *b = v;
            break;
        default:
            *r = v;
            *g = p;
            *b = q;
            break;
    }
}

static inline void rgb565_to_rgb888(uint16_t rgb565, uint8_t *r, uint8_t *g, uint8_t *b)
{
    *r = ((rgb565 >> 11) & 0x1F) * 255 / 31;
    *g = ((rgb565 >> 5) & 0x3F) * 255 / 63;
    *b = (rgb565 & 0x1F) * 255 / 31;
}

void rgb888_to_rgb(uint32_t rgb888, uint8_t *r, uint8_t *g, uint8_t *b)
{
    *r = (uint8_t)(rgb888 >> 16);
    *g = (uint8_t)(rgb888 >> 8);
    *b = (uint8_t)(rgb888);
}

void rgb_set_index_buffer_updata(uint8_t index, uint32_t value)
{
    uint8_t r, g, b;
    rgb888_to_rgb(value, &r, &g, &b);
    uint32_t base = 3 + index * 24;

    for (uint8_t i = 0; i < 8; i++) {
        color_buf[base + 0 + i]  = (g & (1 << (7 - i))) ? BIT_1 : BIT_0;
        color_buf[base + 8 + i]  = (r & (1 << (7 - i))) ? BIT_1 : BIT_0;
        color_buf[base + 16 + i] = (b & (1 << (7 - i))) ? BIT_1 : BIT_0;
    }
}

void rgb_arm_buffer_update(void)
{
    uint16_t ledCount = g_mb_holding_regs[MB_HOLD_REG_RGB_COUNT];
    if (ledCount > RGB_NUM_MAX) ledCount = RGB_NUM_MAX;
    uint16_t baseIndex = MB_HOLD_REG_RGB1_ARM_START;
    uint8_t r, g, b;

    for (uint16_t i = 0; i < ledCount; i++) {
        uint16_t rgb565 = g_mb_holding_regs[baseIndex + i];
        rgb565_to_rgb888(rgb565, &r, &g, &b);

        uint32_t base = 3 + i * 24;
        for (uint8_t j = 0; j < 8; j++) {
            color_buf[base + 0 + j]  = (g & (1 << (7 - j))) ? BIT_1 : BIT_0;
            color_buf[base + 8 + j]  = (r & (1 << (7 - j))) ? BIT_1 : BIT_0;
            color_buf[base + 16 + j] = (b & (1 << (7 - j))) ? BIT_1 : BIT_0;
        }
    }
}

void rgb_clear_buffer(void)
{
    for (uint16_t i = 3; i < RGB_NUM_MAX * 24 + 3; i++) {
        color_buf[i] = BIT_0;
    }
}

void rgb_init(void)
{
    color_buf = (uint16_t *)calloc(3 + RGB_NUM_MAX * 24 + RGB_LED_TIMING_MAX, sizeof(uint16_t));
    rgb_clear_buffer();
    rgb_show();
}

void rgb_show(void)
{
    uint32_t start = HAL_GetTick();
    while (rgb_output_busy) {
        if (HAL_GetTick() - start > RGB_DMA_WAIT_TIMEOUT_MS) {
            return;
        }
    }
    rgb_output_busy = true;
    HAL_TIMEx_PWMN_Stop(&htim8, TIM_CHANNEL_4);
    HAL_TIMEx_PWMN_Stop_DMA(&htim8, TIM_CHANNEL_4);
    HAL_TIMEx_PWMN_Start_DMA(&htim8, TIM_CHANNEL_4, (uint32_t *)color_buf, 3 + RGB_NUM_MAX * 24 + RGB_LED_TIMING_MAX);
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM8) {
        HAL_TIMEx_PWMN_Stop(&htim8, TIM_CHANNEL_4);
        HAL_TIMEx_PWMN_Stop_DMA(&htim8, TIM_CHANNEL_4);
        rgb_output_busy = false;
    }
}

void rgb_power_start_indicate(void)
{
    rgb_set_index_buffer_updata(0, 0xFF0000);
    rgb_show();
}

void rgb_power_complete_indicate(void)
{
    rgb_clear_buffer();
    rgb_show();
}

bool rgb_num_check(uint16_t num)
{
    return (num >= RGB_NUM_MIN) && (num <= RGB_NUM_MAX);
}

void set_rgb_arm_change_flag(bool flag)
{
    rgb_arm_change_flag = flag;
}

bool get_rgb_arm_change_flag(void)
{
    return rgb_arm_change_flag;
}

bool rgb_mode_check(uint16_t mode)
{
    return mode <= RGB_MODE_RECORDING;
}

void set_rgb_mode_change_flag(bool flag)
{
    rgb_mode_change_flag = flag;
}

bool get_rgb_mode_change_flag(void)
{
    return rgb_mode_change_flag;
}

void rgb_gradient_step(void)
{
    static uint32_t last_tick = 0;
    static uint16_t step      = 0;
    if (HAL_GetTick() - last_tick < RGB_UPDATE_INTERVAL_MS) return;
    last_tick = HAL_GetTick();

    rgb_clear_buffer();
    for (uint16_t i = 0; i < RGB_DEFAULT_NUM; i++) {
        uint16_t hue = (step + i * (360 / RGB_DEFAULT_NUM)) % 360;
        uint8_t r, g, b;
        hsv_to_rgb(hue, 255, (uint8_t)(RGB_GRADIENT_BRIGHTNESS >> 1), &r, &g, &b);
        rgb_set_index_buffer_updata(i, ((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
    }
    rgb_show();
    step = (step + 1) % 360;
}

void rgb_blink_step(void)
{
    static uint32_t last_tick = 0;
    static float phase        = 0.0f;  // 亮度相位（弧度单位）
    static uint8_t colorIndex = 0;
    const float step          = 0.01f;  // 每次步进弧度，值越小越慢
    const float pi            = 3.1415926f;

    if (HAL_GetTick() - last_tick < RGB_UPDATE_INTERVAL_MS) return;
    last_tick = HAL_GetTick();

    phase += step;
    if (phase >= pi) {
        phase      = 0.0f;
        colorIndex = (colorIndex + 1) % RGB_GRADIENT_COLOR_COUNT;  // 一整次呼吸后变色
    }

    // 亮度曲线：sin(phase) 从 0→π => 0→1→0
    float brightness_f = sinf(phase);
    uint8_t brightness = (uint8_t)(brightness_f * RGB_BLINK_BRIGHTNESS_MAX);
    brightness >>= 3;  // 降低亮度以防过亮

    const uint8_t *base = gradient_colors[colorIndex];
    uint8_t r           = (uint8_t)((base[0] * brightness) / 255);
    uint8_t g           = (uint8_t)((base[1] * brightness) / 255);
    uint8_t b           = (uint8_t)((base[2] * brightness) / 255);
    uint32_t rgb888     = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;

    rgb_clear_buffer();
    for (uint16_t i = 0; i < RGB_DEFAULT_NUM; i++) {
        rgb_set_index_buffer_updata(i, rgb888);
    }
    rgb_show();
}

void rgb_rainflow_step(void)
{
    static uint32_t last_tick = 0;
    static uint16_t offset    = 0;
    if (HAL_GetTick() - last_tick < RGB_UPDATE_INTERVAL_MS) return;
    last_tick = HAL_GetTick();

    rgb_clear_buffer();
    offset = (offset + 1) % RGB_DEFAULT_NUM;

    for (uint16_t i = 0; i < RGB_DEFAULT_NUM; i++) {
        int16_t dist = (i - offset + RGB_DEFAULT_NUM) % RGB_DEFAULT_NUM;
        int16_t tmp  = 255 - dist * RGB_RAINFLOW_TAIL_STEP;
        if (tmp < 0) tmp = 0;
        uint8_t brightness = (uint8_t)(tmp >> 3);  // 修改: 亮度减半
        uint16_t hue       = (360 * i / RGB_DEFAULT_NUM + offset * 10) % 360;
        uint8_t r, g, b;
        hsv_to_rgb(hue, 255, brightness, &r, &g, &b);
        rgb_set_index_buffer_updata(i, ((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
    }
    rgb_show();
}

/**
 * @brief 流星雨模式：圆环保持青紫跑马灯，灯条改为流星雨
 */
void rgb_recording_step(void)
{
    static uint32_t last_tick   = 0;
    static uint8_t ring_offset  = 0;    // 圆环亮点位置
    static uint16_t hue_shift   = 0;    // 整体色相偏移
    const uint8_t ring_tail_len = 4;    // 圆环尾光长度
    const uint8_t ring_bg_level = 15;   // 圆环背景亮度
    const uint8_t sat_level     = 220;  // 饱和度
    const uint16_t hue_start    = 170;  // 起点色(青蓝)
    const uint8_t hue_span      = 60;   // Hue 跨度（青→紫）

    // ---- 流星雨参数 ----
    const uint8_t tail_len    = 6;               // 流星尾光长度
    const uint8_t bg_level    = 6;               // 灯条背景亮度
    const uint16_t meteor_hue = 190;             // 流星色相，偏冷色
    static uint8_t meteor_pos[RGB_STRIP_COUNT];  // 每条流星头部位置

    // 控制更新频率
    if (HAL_GetTick() - last_tick < 40) return;  // 每40ms刷新一帧
    last_tick = HAL_GetTick();
    rgb_clear_buffer();

    /* ========================================================
     *        圆环：沿用原来的青紫跑马灯
     * ======================================================== */
    for (uint8_t i = 0; i < RGB_RING_COUNT; i++) {
        int8_t dist = (i - ring_offset + RGB_RING_COUNT) % RGB_RING_COUNT;
        uint8_t brightness;
        if (dist < ring_tail_len)
            brightness = (uint8_t)(255 - dist * (255 / ring_tail_len));
        else
            brightness = ring_bg_level;

        brightness >>= 3;  // 修改：亮度减半

        uint16_t hue = (hue_start + dist * (hue_span / ring_tail_len)) % 360;
        uint8_t r, g, b;
        hsv_to_rgb(hue, sat_level, brightness, &r, &g, &b);
        rgb_set_index_buffer_updata(i, ((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
    }
    ring_offset = (ring_offset + 1) % RGB_RING_COUNT;

    /* ========================================================
     *               灯条：流星雨效果
     * ======================================================== */
    for (uint8_t s = 0; s < RGB_STRIP_COUNT; s++) {
        for (uint8_t j = 0; j < RGB_STRIP_LENGTH; j++) {
            int8_t dist = meteor_pos[s] - j;
            uint8_t bri;
            uint16_t hue;
            if (dist >= 0 && dist < tail_len) {
                bri = (uint8_t)(255 - dist * (255 / tail_len));
                hue = (meteor_hue + dist * 5 + hue_shift) % 360;
            } else {
                bri = bg_level;
                hue = (meteor_hue + hue_shift) % 360;
            }

            bri >>= 3;  // 修改：亮度减半

            uint8_t r, g, b;
            hsv_to_rgb(hue, sat_level, bri, &r, &g, &b);
            uint8_t index = 12 + s * RGB_STRIP_LENGTH + j;  // 灯条索引偏移
            rgb_set_index_buffer_updata(index, ((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
        }

        // 向前推进
        meteor_pos[s]++;
        if (meteor_pos[s] >= RGB_STRIP_LENGTH + tail_len) {
            meteor_pos[s] = 0;  // 跑完从头再来
        }
    }

    hue_shift = (hue_shift + 3) % 360;  // 微调色相变化
    rgb_show();
}

void rgb_download_step(void)
{
    static uint32_t last_tick   = 0;
    static uint8_t ring_offset  = 0;    // 圆环亮点位置（反向计数）
    static uint16_t hue_shift   = 0;    // 整体色相偏移
    const uint8_t ring_tail_len = 4;    // 圆环尾光长度
    const uint8_t ring_bg_level = 15;   // 圆环背景亮度
    const uint8_t sat_level     = 220;  // 饱和度
    const uint16_t hue_start    = 170;  // 起点色(青蓝)
    const uint8_t hue_span      = 60;   // Hue 跨度（青→紫）

    // ---- 流星雨参数 ----
    const uint8_t tail_len    = 6;               // 流星尾光长度
    const uint8_t bg_level    = 6;               // 灯条背景亮度
    const uint16_t meteor_hue = 190;             // 流星色相
    static uint8_t meteor_pos[RGB_STRIP_COUNT];  // 每条流星头部位置

    if (HAL_GetTick() - last_tick < 40) return;  // 每40ms刷新一帧
    last_tick = HAL_GetTick();
    rgb_clear_buffer();

    /* ========================================================
     *        圆环：青紫跑马灯（方向反转）
     * ======================================================== */
    for (uint8_t i = 0; i < RGB_RING_COUNT; i++) {
        // 方向反转，只要把偏移计算换成反向
        int8_t dist = (ring_offset - i + RGB_RING_COUNT) % RGB_RING_COUNT;
        uint8_t brightness;
        if (dist < ring_tail_len)
            brightness = (uint8_t)(255 - dist * (255 / ring_tail_len));
        else
            brightness = ring_bg_level;

        brightness >>= 3;  // 修改：亮度减半

        uint16_t hue = (hue_start + dist * (hue_span / ring_tail_len)) % 360;
        uint8_t r, g, b;
        hsv_to_rgb(hue, sat_level, brightness, &r, &g, &b);
        rgb_set_index_buffer_updata(i, ((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
    }

    // 因为方向反，所以 offset 递减
    if (ring_offset == 0)
        ring_offset = RGB_RING_COUNT - 1;
    else
        ring_offset--;

    /* ========================================================
     *             灯条：流星雨（方向反转）
     * ======================================================== */
    for (uint8_t s = 0; s < RGB_STRIP_COUNT; s++) {
        for (uint8_t j = 0; j < RGB_STRIP_LENGTH; j++) {
            int8_t dist = j - meteor_pos[s];  // 反向
            uint8_t bri;
            uint16_t hue;
            if (dist >= 0 && dist < tail_len) {
                bri = (uint8_t)(255 - dist * (255 / tail_len));
                hue = (meteor_hue + dist * 5 + hue_shift) % 360;
            } else {
                bri = bg_level;
                hue = (meteor_hue + hue_shift) % 360;
            }

            bri >>= 3;  // 修改：亮度减半

            uint8_t r, g, b;
            hsv_to_rgb(hue, sat_level, bri, &r, &g, &b);
            uint8_t index = 12 + s * RGB_STRIP_LENGTH + j;
            rgb_set_index_buffer_updata(index, ((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
        }

        // 方向反：位置递减
        if (meteor_pos[s] == 0)
            meteor_pos[s] = RGB_STRIP_LENGTH + tail_len - 1;
        else
            meteor_pos[s]--;
    }

    hue_shift = (hue_shift + 3) % 360;  // 继续保持色相微调
    rgb_show();
}

void update_rgb_custom(void)
{
    if (rgb_power == 0) {
        return;
    }
    switch (g_mb_holding_regs[MB_HOLD_REG_RGB_MODE]) {
        case RGB_MODE_GRADIENT:
            rgb_gradient_step();
            break;
        case RGB_MODE_BLINK:
            rgb_blink_step();
            break;
        case RGB_MODE_RAINFLOW:
            rgb_rainflow_step();
            break;
        case RGB_MODE_DOWNLOAD:
            rgb_download_step();
            break;
        case RGB_MODE_RECORDING:
            rgb_recording_step();
            break;
        default:
            break;
    }
}

void update_rgb_arm(void)
{
    if (g_mb_holding_regs[MB_HOLD_REG_RGB_MODE] == RGB_MODE_ARM_MAP) {
        if (rgb_arm_change_flag || rgb_mode_change_flag) {
            set_rgb_arm_change_flag(false);
            set_rgb_mode_change_flag(false);
            rgb_clear_buffer();
            rgb_arm_buffer_update();
            rgb_show();
        }
    }
}

void rgb_power_off(void)
{
    rgb_power = 0;
    rgb_clear_buffer();
    rgb_show();
    HAL_Delay(0);  // 确保DMA传输完成
}

void rgb_power_on(void)
{
    rgb_power = 1;
    update_rgb_arm();
}