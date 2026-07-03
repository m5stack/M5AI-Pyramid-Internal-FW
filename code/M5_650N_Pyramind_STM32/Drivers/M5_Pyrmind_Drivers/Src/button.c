#include "button.h"
#include "demo.h"

//==================== 全局变量 ====================
button_t button_top;
button_t button_lcd;

static uint32_t last_check_time     = 0;  // 上次检测时间
volatile uint8_t btn_top_power_flag = 0;

//==================== 内部函数 ====================
static void button_update(button_t *btn)
{
    uint8_t raw_state = (HAL_GPIO_ReadPin(btn->port, btn->pin) == GPIO_PIN_RESET) ? 1 : 0;  // 按下为低电平->1

    // 消抖处理
    if (raw_state != btn->last_stable_state) {
        btn->debounce_counter++;
        if (btn->debounce_counter >= BUTTON_DEBOUNCE_COUNT) {
            btn->last_stable_state = raw_state;
            btn->debounce_counter  = 0;
            btn->input             = raw_state;

            // 边沿事件
            if (raw_state == 1) {
                btn->fall_event                = 1;
                btn->press_time                = 0;
                btn->long_event_triggered      = 0;
                btn->very_long_event_triggered = 0;
            } else {
                btn->rise_event = 1;
                btn->press_time = 0;
            }
        }
    } else {
        btn->debounce_counter = 0;
        btn->input            = btn->last_stable_state;
    }

    // 长按判断
    if (btn->input == 1) {
        // 累加按下时间
        if (btn->press_time < BUTTON_VERY_LONGPRESS_TIME_MS) {
            btn->press_time += BUTTON_SCAN_PERIOD_MS;
        }
        // 3秒长按判断
        if (!btn->long_event_triggered && btn->press_time >= BUTTON_LONGPRESS_TIME_MS) {
            btn->long_event           = 1;
            btn->long_event_triggered = 1;  // 标记已触发

            if (btn->pin == EC_BUTTON2_Pin) {  // 切换LCD模式
                g_mb_holding_regs[MB_HOLD_REG_LCD_MODE]++;
                if (g_mb_holding_regs[MB_HOLD_REG_LCD_MODE] >= LCD_MODE_COUNT) {
                    g_mb_holding_regs[MB_HOLD_REG_LCD_MODE] = 0;
                }
                set_lcd_mode_flag(true);
            }
        }

        // 15秒长按判断
        if (!btn->very_long_event_triggered && btn->press_time >= BUTTON_VERY_LONGPRESS_TIME_MS) {
            btn->very_long_event           = 1;
            btn->very_long_event_triggered = 1;  // 标记已触发
            if (btn->pin == EC_BUTTON_Pin) {
                btn_top_power_flag = 1;
                set_power_update_status(true);
            }
        }
    }
}

//==================== 初始化 ====================
void button_init(void)
{
    // 初始化 button_top
    button_top.port              = GPIOC;
    button_top.pin               = EC_BUTTON_Pin;
    button_top.input             = 0;
    button_top.fall_event        = 0;
    button_top.rise_event        = 0;
    button_top.long_event        = 0;
    button_top.last_stable_state = 0;
    button_top.debounce_counter  = 0;
    button_top.press_time        = 0;

    // 初始化 button_lcd
    button_lcd     = button_top;
    button_lcd.pin = EC_BUTTON2_Pin;

    last_check_time = HAL_GetTick();
}

//==================== 扫描函数 (自动限速) ====================
void button_scan(void)
{
    uint32_t now = HAL_GetTick();
    if ((now - last_check_time) < BUTTON_SCAN_PERIOD_MS) return;

    last_check_time = now;

    button_update(&button_top);
    button_update(&button_lcd);
}

//==================== 事件读取 ====================
uint8_t button_getInput(button_t *btn)
{
    return btn->input;
}

uint8_t button_getFallEvent(button_t *btn)
{
    uint8_t val     = btn->fall_event;
    btn->fall_event = 0;
    return val;
}

uint8_t button_getRiseEvent(button_t *btn)
{
    uint8_t val     = btn->rise_event;
    btn->rise_event = 0;
    return val;
}

uint8_t button_getLongEvent(button_t *btn)
{
    uint8_t val     = btn->long_event;
    btn->long_event = 0;
    return val;
}

void button_press_wakeup(void)
{
    static uint32_t last_check_time  = 0;
    static uint8_t stable_count      = 0;
    static uint8_t last_button_state = GPIO_PIN_SET;  // 记录上一次按钮状态
    static uint8_t initialized       = 0;             // 初始化标志

    if (get_power_wakeup_status() == POWER_WAKEUP_STATUS_NONE) {
        return;
    }

    uint32_t now = HAL_GetTick();
    if ((now - last_check_time) < BUTTON_SCAN_PERIOD_MS) {
        return;
    }
    last_check_time = now;

    uint8_t current_state = HAL_GPIO_ReadPin(EC_BUTTON_GPIO_Port, EC_BUTTON_Pin);

    // 首次进入时，同步当前按键状态
    if (!initialized) {
        last_button_state = current_state;
        initialized       = 1;
    }

    // 检测下降沿：从高电平变为低电平
    if (last_button_state == GPIO_PIN_SET && current_state == GPIO_PIN_RESET) {
        stable_count = 1;  // 检测到下降沿，开始消抖计数
    } else if (current_state == GPIO_PIN_RESET && stable_count > 0) {
        // 持续低电平，继续消抖
        if (stable_count < BUTTON_DEBOUNCE_COUNT) {
            stable_count++;
        }
    } else if (current_state == GPIO_PIN_SET) {
        // 恢复高电平，复位计数
        stable_count = 0;
    }

    last_button_state = current_state;  // 更新状态

    if (stable_count >= BUTTON_DEBOUNCE_COUNT) {
        get_page60_buffer();
        for (uint8_t i = 4; i < 15; i++) {
            g_mb_coils[i] = page60_buffer[i];
        }

        g_mb_coils[MB_COIL_EXT_5V_SUPPLY]     = 1;
        g_mb_coils[MB_COIL_INT_5V_MAIN_POWER] = 1;
        g_mb_coils[MB_COIL_CLK_MODE_SELECT]   = 0;

        system_power_on();
        peripheral_power_on();
        rgb_power_on();
        lcd_power_on();
        set_power_wakeup_status(POWER_WAKEUP_STATUS_NONE);

        stable_count = 0;
        initialized  = 0;
    }
}
