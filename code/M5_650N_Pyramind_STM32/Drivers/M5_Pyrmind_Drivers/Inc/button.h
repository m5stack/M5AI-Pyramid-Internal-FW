#ifndef __BUTTON_H__
#define __BUTTON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define BUTTON_SCAN_PERIOD_MS         (5)
#define BUTTON_LONGPRESS_TIME_MS      (3000)
#define BUTTON_VERY_LONGPRESS_TIME_MS (15000)
#define BUTTON_DEBOUNCE_COUNT         (3)

#define BUTTON_TOP_PORT (GPIOC)
#define BUTTON_TOP_PIN  (EC_BUTTON_Pin)

#define BUTTON_LCD_PORT (GPIOC)
#define BUTTON_LCD_PIN  (EC_BUTTON2_Pin)

typedef struct {
    GPIO_TypeDef *port;  // GPIO端口
    uint16_t pin;        // GPIO引脚

    uint8_t input;            // 当前状态：1=按下，0=未按下
    uint8_t fall_event;       // 下降沿事件：按下瞬间置1，读取后自动清零
    uint8_t rise_event;       // 上升沿事件：松开瞬间置1，读取后自动清零
    uint8_t long_event;       // 长按事件：持续按下超过设定时间置1，读取后自动清零
    uint8_t very_long_event;  // 超长按事件：持续按下超过更长时间置1

    // 内部变量
    uint8_t last_stable_state;          // 上次稳定状态
    uint8_t debounce_counter;           // 消抖计数
    uint16_t press_time;                // 按下累计ms
    uint8_t long_event_triggered;       // 3秒事件已触发标志 (新增)
    uint8_t very_long_event_triggered;  // 15秒事件已触发标志 (新增)
} button_t;

extern button_t button_top;
extern button_t button_lcd;
extern volatile uint8_t btn_top_power_flag;

void button_init(void);
void button_scan(void);
uint8_t button_getInput(button_t *btn);
uint8_t button_getFallEvent(button_t *btn);
uint8_t button_getRiseEvent(button_t *btn);
uint8_t button_getLongEvent(button_t *btn);
void button_press_wakeup(void);

#ifdef __cplusplus
}
#endif

#endif /* __BUTTON_H__ */
