/**
 * @file    ScreenDisplay.h
 * @brief   OLED屏幕显示管理头文件
 *
 * @details 封装OLED屏幕的初始化、多页面显示、状态更新及滚动消息功能，
 *          支持I2C设备数量显示、IP地址更新、IO状态监控，基于U8g2库实现。
 */
#pragma once
#include "global.h"

// #ifndef __SCREENDISPLAY_H_
// #define __SCREENDISPLAY_H_
#include "ConfigParser.h"
#include "U8g2lib.h"
/**
 * @brief 标签结构体
 *
 * @details 存储显示文本及其状态（用于IO状态显示）
 */
typedef struct labels
{
    String text;
    uint8_t staus;
    unsigned long blinkUntil = 0; // 新增：记录需要闪烁到的时间戳（ms）
    bool blinking = false;        // 新增：是否处于闪烁中

} labels;

/**
 * @brief IO状态更新结构体
 *
 * @details 用于传递IO端口的索引和状态变化
 */
struct IOStatusUpdate
{
    uint8_t ioIndex;
    bool status;
};

/**
 * @brief 屏幕显示管理类
 *
 * @details 提供OLED屏幕的初始化、多页面切换、数据更新及节能管理功能，
 *          支持IO状态显示、I2C设备数量、IP地址、配置信息等内容的渲染。
 */
class ScreenDisplay
{
public:
    /** @brief 初始化屏幕显示（配置U8g2库、设置默认参数） */
    void init();
    /** @brief 主循环更新函数（处理屏幕刷新、超时休眠、滚动消息） */
    void loop();

    /**
     * @brief 更新IO状态
     *
     * @param ioCNT IO端口数量
     * @param state 端口状态（true表示激活，false表示关闭）
     */
    void setStaus(uint8_t ioCNT, bool state);

    /**
     * @brief 设置IP地址显示字符串
     *
     * @param str IP地址字符串
     */
    void setIPStr(String str);

    /**
     * @brief 切换显示页面
     *
     * @param page 页面索引（0-based，0和1分别对应不同页面内容）
     */
    void showPage(int page);

    /**
     * @brief 从键值对更新头部显示
     *
     * @param kv_pairs 键值对数组指针
     * @param max_entries 最大条目数
     * @details 用于显示配置信息或传感器数据（如温度、湿度等）
     */
    void updateHeaderFromKeyValue(KeyValue *kv_pairs, int max_entries);

    /**
     * @brief 设置I2C设备信息显示
     *
     * @param cnt I2C设备数量
     * @param str I2C地址字符串（如"0X50,0X51"）
     */
    void setI2cStr(uint8_t cnt, String str);

    /** @brief 按钮按下标志（外部可设置，用于触发页面切换等操作） */
    bool buttonPressed = false;

private:
    labels ioLabel[6];                                     ///< IO状态标签数组（最多支持6个IO端口）
    int currentPage = 0;                                   ///< 当前显示页面索引
    const int totalPages = 2;                              ///< 总页面数（0和1）
    bool screenOn = true;                                  ///< 屏幕开关状态（用于节能）
    uint8_t i2cCNT = 0;                                    ///< I2C设备数量
    String ipStr = "DFRobot DFR1234";                      ///< IP地址显示字符串
    unsigned long lastActivityTime;                        ///< 最后活动时间（用于超时休眠）
    const unsigned long screenTimeout = SCREEN_TIMEOUT_MS; ///< 屏幕超时时间（1分钟）

    // 滚动消息相关参数
    unsigned long lastScrollTime = 0;         ///< 上次滚动时间戳
    const unsigned long scrollInterval = 200; ///< 滚动间隔（毫秒）
    int scrollOffset = 0;                     ///< 滚动偏移量
    bool shouldScroll = false;                ///< 是否需要滚动（用于长文本显示）
    String scrollMessage = "";                ///< 待滚动显示的消息
    int scrollMessageWidth = 0;               ///< 消息宽度（像素）

    U8G2 u8g2;
};
// #endif