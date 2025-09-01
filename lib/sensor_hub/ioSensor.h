/**
 * @file    ioSensor.h
 * @brief   传感器采集与IO控制模块
 *
 * @details 定义了多种传感器类型、IO节点结构和传感器集线器类，
 *          用于管理和处理各类传感器数据采集与输出控制。
 */
#pragma once
// #ifndef _IO_COLLECT_H_
// #define _IO_COLLECT_H_
#include "global.h"
#include "ConfigParser.h"
#include "ArduinoJson.h"
#include "OneWire.h"
#include "DallasTemperature.h"
#include "DHT.h"
#include <ESP32Servo.h>
#include "LiteLED.h"
/**
 * @brief 传感器类型枚举
 *
 * @details 定义系统支持的各类传感器和IO设备类型
 */
typedef enum
{
    IO_NULL,        ///< 空类型（未配置）
    IO_ANALOG,      ///< 模拟输入
    IO_DIGITAL_OUT, ///< 数字输出
    IO_DHT11,       ///< DHT11温湿度传感器
    IO_WS2812,      ///< WS2812 RGB LED灯带
    IO_DS18B20,     ///< DS18B20温度传感器
    IO_SERVO180,    ///< 180度舵机
    IO_SERVO360     ///< 360度连续旋转舵机
} SensorName;

/**
 * @brief IO传感器节点结构体
 *
 * @details 存储单个IO设备或传感器的配置信息和状态
 */
typedef struct IOSensorNode
{
    SensorName name; ///< 传感器类型
    uint8_t pin;     ///< 连接的GPIO引脚
    uint8_t mode;    ///< 工作模式（特定于传感器类型）
    char value[64];  ///< 传感器当前值（字符串形式）
    uint8_t io_idx;  ///< IO索引（用于标识）

    /**
     * @brief 传感器回调函数指针
     *
     * @param pin GPIO引脚号
     * @param io_idx IO索引
     * @param type 传感器类型
     * @param arg 控制参数（输入）
     * @param res 返回结果（输出）
     */
    void (*callBack)(uint8_t pin, uint8_t io_idx, SensorName name, const char *arg, char *res);
} IOSensorNode;

/**
 * @brief IO传感器集线器类
 *
 * @details 管理多个IO传感器节点，提供初始化、配置和数据处理功能
 */
class IOSensorHub
{
public:
    /** @brief IO传感器节点数组 */
    IOSensorNode ioSensorNode[IO_PORT_NUM];

    /** @brief 构造函数 */
    IOSensorHub();

    /** @brief 析构函数 */
    virtual ~IOSensorHub();

    /** @brief 初始化所有传感器 */
    void init();

    /**
     * @brief 根据配置键值对配置IO传感器
     *
     * @param kv_pairs 配置键值对数组
     */
    void configIO(KeyValue *kv_pairs);

    /** @brief 处理所有传感器数据采集 */
    void process();

    /** @brief JSON格式的传感器数据字符串 */
    String JsonStr;

    /** @brief 将所有传感器数据转换为JSON字符串 */
    void toJsonStr();

    /**
     * @brief 处理MQTT控制消息
     *
     * @param json JSON格式的控制消息
     */
    void handleMQTTMessage(const char *json);

private:
    /**
     * @brief 将字符串复制并转换为小写
     *
     * @param src 源字符串
     * @param dst 目标缓冲区
     * @param dstSize 目标缓冲区大小
     * @return char* 指向目标缓冲区的指针
     */
    char *_str_to_lower_copy(const char *src, char *dst, size_t dstSize);

    /**
     * @brief 从字符串获取IO模式
     *
     * @param val 表示IO模式的字符串
     * @return SensorName 对应的传感器类型
     */
    SensorName _getIOModeFromString(char *val);

    /**
     * @brief 处理控制JSON消息
     *
     * @param doc 解析后的JSON文档
     */
    void _handleControlJson(JsonDocument &doc);
};

/** @brief 最大支持的WS2812灯带数量 */
#define MAX_STRIPS IO_PORT_NUM
#define MAX_PIXELS 32
#define LED_TYPE LED_STRIP_WS2812
#define LED_TYPE_IS_RGBW 0
#define MAX_CHANNELS (MAX_PIXELS * 3)

/**
 * @brief WS2812 LED灯带结构体
 * 
 * @details 存储WS2812灯带的配置和状态信息
 */
struct WS2812Strip
{
     uint8_t pin;          ///< 连接的GPIO引脚
    bool initialized;     ///< 是否已初始化
    LiteLED* led;         ///< LiteLED库实例指针
};

/**
 * @brief 数字输出回调函数
 * 
 * @param pin GPIO引脚号
 * @param io_idx IO索引
 * @param type 传感器类型
 * @param arg 控制参数（输入）
 * @param res 返回结果（输出->NULL
 */
void DIGITAL_OUT_CALLBACK(uint8_t pin, uint8_t io_idx, SensorName name, const char *arg, char *res);

/**
 * @brief 模拟输入回调函数
 * 
 * @param pin GPIO引脚号
 * @param io_idx IO索引
 * @param type 传感器类型
 * @param arg 控制参数（输入）null
 * @param res 返回结果（输出）
 */
void ANALOG_CALLBACK(uint8_t pin, uint8_t io_idx, SensorName name, const char *arg, char *res);

/**
 * @brief DHT11传感器回调函数
 * 
 * @param pin GPIO引脚号
 * @param io_idx IO索引
 * @param type 传感器类型
 * @param arg 控制参数（输入）
 * @param res 返回结果（输出）
 */
void DHT11_CALLBACK(uint8_t pin, uint8_t io_idx, SensorName name, const char *arg, char *res);

/**
 * @brief DHT11传感器回调函数
 * 
 * @param pin GPIO引脚号
 * @param io_idx IO索引
 * @param type 传感器类型
 * @param arg 控制参数（输入）
 * @param res 返回结果（输出） NULL
 */
void DS18B20_CALLBACK(uint8_t pin, uint8_t io_idx, SensorName name, const char *arg, char *res);

/**
 * @brief 360度舵机回调函数
 * 
 * @param pin GPIO引脚号
 * @param io_idx IO索引
 * @param type 传感器类型
 * @param arg 控制参数（输入）
 * @param res 返回结果（输出） NULL
 */
void SERVO360_CALLBACK(uint8_t pin, uint8_t io_idx, SensorName name, const char *arg, char *res);

/**
 * @brief 180度舵机回调函数
 * 
 * @param pin GPIO引脚号
 * @param io_idx IO索引
 * @param type 传感器类型
 * @param arg 控制参数（输入）
 * @param res 返回结果（输出）NULL
 */
void SERVO180_CALLBACK(uint8_t pin, uint8_t io_idx, SensorName name, const char *arg, char *res);

/**
 * @brief WS2812 LED灯带回调函数
 * 
 * @param pin GPIO引脚号
 * @param io_idx IO索引
 * @param type 传感器类型
 * @param arg 控制参数（输入）
 * @param res 返回结果（输出）NULL
 */
void WS2812_CALLBACK(uint8_t pin, uint8_t io_idx, SensorName name, const char *arg, char *res);

// #endif
