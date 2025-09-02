/**
 * @file    app_task.h
 * @brief   应用任务管理头文件
 * 
 * @details 定义系统状态结构体、事件类型枚举及任务函数原型，
 *          基于FreeRTOS实现多任务协作，包含WiFi状态管理、MQTT服务、传感器处理等功能。
 */
#pragma once
// #ifndef _APP_TASK_H_
// #define _APP_TASK_H_
 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "WiFi.h"



/**
 * @brief 系统状态结构体
 * 
 * @details 存储系统关键组件的运行状态，包含互斥锁用于线程安全访问。
 */
typedef struct
{
    bool wifi_connected;
    bool mqtt_running;
    bool config_valid;
    bool config_wifi;
    SemaphoreHandle_t mutex;
} SystemState;

/**
 * @brief 系统事件枚举
 * 
 * @details 定义系统级事件类型，用于事件队列和事件组通信。
 */
typedef enum
{
    MSC_WRITE_EVENT,
    WIFI_CONNECTED_EVENT,
    WIFI_DISCONNECTED_EVENT,
    CONFIG_UPDATED_EVENT
} SystemEvent_t;
const int WIFI_CONNECTED_BIT = BIT0;

/**
 * @brief 任务启动函数
 * 
 * @details 初始化系统状态并创建所有应用任务，作为任务调度入口。
 */
void start_task(void);

/**
 * @brief 主应用任务
 * 
 * @param args NULL
 * @details 系统核心任务
 */
void app_main(void *args);

/**
 * @brief 显示任务
 * 
 * @param args null
 * @details 负责OLED屏幕的刷新
 */
void display_task(void *args);

/**
 * @brief 重新配置定时器回调函数
 * 
 * @param xTimer 定时器句柄
 * @details 定时触发配置重新加载或系统状态检查。
 */
// void vReconfigTimerCallback(TimerHandle_t xTimer);

/**
 * @brief MQTT服务器任务
 * 
 * @param arg null
 * @details 启动MQTT服务器
 */
void MQTT_ServerTask(void *arg);

/**
 * @brief 传感器中枢任务
 * 
 * @param arg null
 * @details 管理I传感器的数据采集和事件处理。
 */
void SensorHub_Task(void *arg);

/**
 * @brief MQTT消息处理任务
 * 
 * @param pvParameters null
 * @details 处理MQTT消息的接收、解析和响应。
 */
void mqttHandlerTask(void *pvParameters);

/**
 * @brief 设置更新间隔函数
 * 
 * @return bool 更新间隔设置成功与否
 * @details 从配置文件读取或重置数据上报间隔。
 */
bool setUpdateInterval();

/**
 * @brief 按钮轮询任务
 * 
 * @param param 任务参数（通常为按钮引脚编号）
 * @details 检测物理按钮状态变化，触发页面切换或功能调用。
 */
void buttonPollTask(void *param);

/**
 * @brief DAT LED控制任务
 * 
 * @param args 任务参数（通常为DAT实例指针）
 * @details 管理状态指示灯的颜色、亮度和闪烁模式。
 */
void datLedTask(void *args);

/**
 * @brief WiFi事件回调函数
 * 
 * @param event WiFi事件类型（通过WiFiEvent_t枚举定义）
 * @details 处理WiFi状态变化事件，更新系统状态并发送事件通知。
 */
void MyWiFiEvent(WiFiEvent_t event);

/**
 * @brief I2C设备扫描任务
 * 
 * @param args 任务参数（通常为I2CDeviceManager实例指针）
 * @details 定期扫描I2C总线，发现并管理传感器设备。
 */
void i2c_scan_task(void *args);


// #endif