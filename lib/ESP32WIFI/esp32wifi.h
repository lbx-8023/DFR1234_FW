// #ifndef __ESP32WIFI_H_
// #define __ESP32WIFI_H__
#pragma once
  
#include <WiFi.h>

/**
 * 
 * @brief WiFi事件处理函数
 *
 * @param event WiFi事件类型
 *
 * @details 处理WiFi连接状态变化等事件
 */
// void MyWiFiEvent(WiFiEvent_t event);

class ESP32WiFi{
    public:
    ESP32WiFi();
    bool connect(char *ssid, char *passwd);
    bool isConnect();
    void disConnect();
};
// #endif