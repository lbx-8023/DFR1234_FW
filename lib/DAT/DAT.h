/**
 * @file    DAT.h
 * @brief   状态灯控制模块头文件
 *
 * @details 封装单点RGB LED的控制逻辑，用于状态指示
 */
#pragma once
// #ifndef DAT_H
// #define DAT_H

#include "LiteLED.h"
#include "global.h"
/**
 * @brief 单点LED控制类
 *
 * @details 提供单点RGB LED的初始化、颜色设置、亮度调节和显示控制功能，
 */
class DAT
{
public:
    /**
     * @brief 构造函数
     *
     *
     * @param pin 控制引脚（默认：38号GPIO）
     * @note 引脚需支持PWM输出，建议使用ESP32的RMT通道引脚
     */
    DAT(uint8_t pin = DAT_LED_PIN, uint8_t num=1);

    /**
     * @brief 初始化LED
     *
     * @param num 数量
     * @details 调用LiteLED库初始化函数，设置LED类型为单像素模式
     */
    void begin();

    /**
     * @brief 设置RGB颜色（分量值）
     *
     * @param r 红色分量（0-255）
     * @param g 绿色分量（0-255）
     * @param b 蓝色分量（0-255）
     * @note 颜色值会自动根据当前亮度调整
     */
    void setColor(uint8_t r, uint8_t g, uint8_t b); // 设置颜色（RGB分量）

    /**
     * @brief 设置十六进制颜色
     *
     * @param hexColor 十六进制颜色值（如0xff0000表示红色）
     * @example setColor(0x00ff00); // 绿色
     */
    void setColor(uint32_t hexColor);

    /**
     * @brief 设置亮度
     *
     * @param brightness 亮度值（0-255，0为熄灭，255为最大亮度）
     */
    void setBrightness(uint8_t brightness);

    /**
     * @brief 获取当前亮度
     *
     * @return uint8_t 当前亮度值（0-255）
     */
    uint8_t getBrightness() const;
    
    /**
     * @brief 刷新LED显示
     *
     * @details 将当前颜色和亮度值输出到硬件引脚
     */
    void show();
    
    /**
     * @brief 熄灭LED
     *
     * @details 设置亮度为0并刷新显示
     */
    void off();

private:
    LiteLED led_;        ///< LiteLED库实例
    uint8_t pin_;        ///< 控制引脚
    uint8_t brightness_; ///< 当前亮度（0-255）
    uint32_t color_;     ///< 存储颜色值（RGB格式）
    uint8_t _num;
};

// #endif // DAT_H
