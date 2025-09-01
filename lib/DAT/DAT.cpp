/**
 * @file    DAT.cpp
 * @brief   单点LED控制模块实现
 * 
 * @details 基于LiteLED库实现单点RGB LED的控制逻辑，
 *          支持颜色设置（RGB/十六进制）、亮度调节及状态管理，
 *          适用于ESP32等微控制器的状态指示场景。
 */
#include "DAT.h"

// 定义LED类型（WS2812，非RGBW模式）
#define LED_TYPE LED_STRIP_WS2812
#define LED_TYPE_IS_RGBW 0

/**
 * @brief 构造函数
 * 
 * @param pin 控制引脚
 * @details 初始化成员变量：
 *          - pin_: 控制引脚
 *          - led_: LiteLED实例（指定LED类型）
 *          - brightness_: 默认亮度64（25%左右）
 *          - color_: 默认颜色黑色（0x000000）
 */
DAT::DAT(uint8_t pin, uint8_t num) : pin_(pin), _num(num), led_(LED_TYPE, LED_TYPE_IS_RGBW,RMT_CHANNEL_2), brightness_(64), color_(0x000000) {}


/**
 * @brief 初始化LED硬件
 * 
 * @details 1. 调用LiteLED库初始化函数，设置引脚和LED数量（1个）
 *          2. 应用默认亮度
 *          3. 显示默认颜色（黑色）
 */
void DAT::begin() {
    led_.begin(pin_, _num);  // 初始化 1 个 LED
    led_.brightness(brightness_);
    led_.setPixel(0, color_, 1);  // 显示默认颜色
}

/**
 * @brief 设置RGB颜色（分量值）
 * 
 * @param r 红色分量（0-255）
 * @param g 绿色分量（0-255）
 * @param b 蓝色分量（0-255）
 * @details 1. 合并RGB分量为32位颜色值（格式：0x00RRGGBB）
 *          2. 更新LED像素颜色（不立即刷新，需调用show()）
 */
void DAT::setColor(uint8_t r, uint8_t g, uint8_t b) {
    color_ = (r << 16) | (g << 8) | b;
    for (uint8_t i = 0; i < _num; ++i){
        led_.setPixel(i, color_, 0);  // 不立即 show
    }
}

/**
 * @brief 设置十六进制颜色
 * 
 * @param hexColor 十六进制颜色值（如0xff0000表示红色）
 * @details 直接使用十六进制值更新颜色，不立即刷新
 */
void DAT::setColor(uint32_t hexColor) {
    color_ = hexColor;
    for (uint8_t i = 0; i < _num; ++i){
        led_.setPixel(i, color_, 0);  // 不立即 show
    }
}

/**
 * @brief 设置亮度
 * 
 * @param brightness 亮度值（0-255）
 * @details 1. 缓存亮度值到成员变量
 *          2. 调用LiteLED库设置亮度（不立即刷新）
 */
void DAT::setBrightness(uint8_t brightness) {
    brightness_ = brightness;
    led_.brightness(brightness_, 0);  // 不立即 show
}

/**
 * @brief 获取当前亮度
 * 
 * @return uint8_t 当前亮度值（0-255）
 */
uint8_t DAT::getBrightness() const {
    return brightness_;
}


/**
 * @brief 刷新LED显示
 * 
 * @details 同步所有未提交的颜色和亮度变更到硬件
 */
void DAT::show() {
    led_.show(); 
}

/**
 * @brief 熄灭LED
 * 
 * @details 设置颜色为黑色并立即刷新
 */
void DAT::off() {
    for (uint8_t i = 0; i < _num; ++i){
        led_.setPixel(i, 0x000000, 0);
    }
    led_.show();
}
