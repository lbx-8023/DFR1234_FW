/**
 * @file    esp32wifi.cpp
 * @brief   ESP32 WiFi连接管理实现
 *
 * @details 封装ESP32的WiFi连接逻辑，支持STA模式连接、状态查询、断开连接及事件回调，
 *          通过系统事件队列通知主程序WiFi状态变化，并集成OLED显示IP地址功能。
 */
#include "esp32wifi.h"
 
#include "Display.h"
// 外部声明的系统事件队列和OLED显示实例
extern QueueHandle_t xSystemEventQueue;
extern ScreenDisplay oled;

/**
 * @brief ESP32WiFi类构造函数
 *
 * @details 初始化WiFi模式为STA模式（Station模式），
 *          注册WiFi事件回调函数MyWiFiEvent处理状态变化。
 */
ESP32WiFi::ESP32WiFi()
{
    // WiFi.mode(WIFI_STA);
}

/**
 * @brief 连接到WiFi网络
 *
 * @param ssid WiFi网络名称
 * @param passwd WiFi密码（若为开放网络可传NULL）
 * @return bool 连接成功返回true，超时或失败返回false
 * @note 1. 自动生成设备主机名（基于芯片MAC地址）
 *       2. 最多尝试30次连接（每次间隔200ms，总超时6秒）
 */
bool ESP32WiFi::connect(char *ssid, char *passwd)
{
    // 生成唯一主机名（格式：DFRobot_DFR1234_<芯片ID>）
    uint32_t chipId = 0;
    for (int i = 0; i < 17; i = i + 8)
    {
        chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }
    String hostname = "DFRobot_DFR1234_" + String(chipId);
    WiFi.setHostname(hostname.c_str());
        // 启动WiFi连接
    WiFi.begin(ssid, passwd);
        // 等待连接完成（最多30次尝试，每次200ms）
    uint8_t i = 30;
    while (WiFi.status() != WL_CONNECTED && i != 0)
    {
        vTaskDelay(pdMS_TO_TICKS(200));
        i--;
    }
    if (WiFi.status() == WL_CONNECTED)
    {
        return true;
    }
    return false;
}


/**
 * @brief 查询WiFi连接状态
 * 
 * @return bool 已连接返回true，否则返回false
 */
bool ESP32WiFi::isConnect()
{
    if (WiFi.status() == WL_CONNECTED)
        return true;
    return false;
}

/**
 * @brief 断开WiFi连接
 * 
 * @details 调用ESP-IDF的WiFi.disconnect()断开连接，
 *          断开后WiFi状态变为WL_DISCONNECTED。
 */
void ESP32WiFi::disConnect()
{
    WiFi.disconnect();
}

/**
 * @brief WiFi事件回调函数
 * @param event WiFi事件类型
 * @details 处理WiFi状态变化事件，
 *          并通过系统事件队列通知主任务
 */
