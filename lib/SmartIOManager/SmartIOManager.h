#ifndef __SMART_IO_MANAGER_H_
#define __SMART_IO_MANAGER_H_
#include "global.h"
#include "ConfigParser.h"
#include "ArduinoJson.h"

#include "IOHub.h"
#include "Display.h"

/** @brief 外部声明的OLED显示屏对象 */
extern ScreenDisplay oled;

class SmartIOManager {
public:
    SmartIOManager();

    ~SmartIOManager();

    /**
     * @brief 配置 IOHub 根据配置文件获取的键值对配置对应的IO口功能
     * @param kv_pairs 配置键值对数组
     */
    void configIOhub(KeyValue *kv_pairs);

    /**
     * @brief 初始化所有 IOHub 实例
     */
    void init();

    /**
     * @brief 处理所有传感器的数据采集
     */
    void process();

    /**
     * @brief 处理需要连续采集的传感器
     * 
     */
    void continueIOProcess();

    /**
     * @brief 处理 MQTT 消息，解析 JSON 并调用相应的 IOHub 方法
     * @param json MQTT 消息的 JSON 字符串
     */
    void handleMQTTMessage(const char *json);

    String JsonStr;
    String conJsonStr;
private:
    std::vector<IOHub *> iohubs; // 存储 IOHub 实例的指针数组

    std::vector<IOHub *> ioConhubs; // 存储持续运行的 IOHub 实例的指针数组

    void _handleMQTTMessage(JsonDocument &doc);

    char *_str_to_lower_copy(const char *src, char *dst, size_t dstSize);

    SensorName _getIOModeFromString(char *val);
};


#endif