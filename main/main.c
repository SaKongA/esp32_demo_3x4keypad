#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define ROW1_PIN    19
#define ROW2_PIN    18
#define ROW3_PIN    16
#define ROW4_PIN    4
#define COL1_PIN    21
#define COL2_PIN    32
#define COL3_PIN    33

static const char* TAG = "KEYPAD";

// 定义按键映射表
const char keymap[4][3] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}
};

// GPIO配置
void keypad_init(void) {
    // 配置行引脚为输出
    gpio_config_t row_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = (1ULL<<ROW1_PIN) | (1ULL<<ROW2_PIN) | 
                       (1ULL<<ROW3_PIN) | (1ULL<<ROW4_PIN)
    };
    gpio_config(&row_conf);

    // 配置列引脚为输入
    gpio_config_t col_conf = {
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = (1ULL<<COL1_PIN) | (1ULL<<COL2_PIN) | (1ULL<<COL3_PIN)
    };
    gpio_config(&col_conf);
}

// 扫描按键
char scan_keypad(void) {
    const int row_pins[4] = {ROW1_PIN, ROW2_PIN, ROW3_PIN, ROW4_PIN};
    const int col_pins[3] = {COL1_PIN, COL2_PIN, COL3_PIN};
    
    // 逐行扫描
    for (int row = 0; row < 4; row++) {
        // 将当前行设置为低电平
        for (int i = 0; i < 4; i++) {
            gpio_set_level(row_pins[i], i == row ? 0 : 1);
        }
        
        // 延时一小段时间等待电平稳定
        vTaskDelay(pdMS_TO_TICKS(5));
        
        // 检查每一列
        for (int col = 0; col < 3; col++) {
            if (gpio_get_level(col_pins[col]) == 0) {
                // 按键被按下，等待按键释放
                while (gpio_get_level(col_pins[col]) == 0) {
                    vTaskDelay(pdMS_TO_TICKS(10));
                }
                return keymap[row][col];
            }
        }
    }
    
    return 0; // 没有按键按下
}

void keypad_task(void *pvParameters) {
    char key;
    
    while (1) {
        key = scan_keypad();
        if (key != 0) {
            ESP_LOGI(TAG, "Key Pressed: %c", key);
        }
        vTaskDelay(pdMS_TO_TICKS(50)); // 延时50ms
    }
}

void app_main(void) {
    // 初始化键盘
    keypad_init();
    
    // 创建键盘扫描任务
    xTaskCreate(keypad_task, "keypad_task", 2048, NULL, 10, NULL);
}
