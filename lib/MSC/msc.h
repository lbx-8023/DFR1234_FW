#pragma once
// #ifndef MYMSC_H
// #define MYMSC_H
#include <Arduino.h>
#include "FS.h"
#include "USBMSC.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "USB.h"
#include "esp_partition.h"
#include "FFat.h"
#include "app_task.h"
#include <inttypes.h>
#include "ff.h"
#include "diskio.h"
#include <Update.h>

#define HWSerial Serial0
#define USBSerial Serial

class USB_MSC
{
private:
    USBMSC MSC;
    // CRC32校验表（用于快速计算CRC值）
    uint32_t crc32_table[256];
    // 私有成员函数声明
    /**
     * @brief 生成CRC32校验表
     * @param crc32_table 输出参数，存储生成的256项CRC表
     * @note 使用标准CRC32多项式（0xEDB88320）生成查表法所需的校验表
     */
    void generate_crc32_table(uint32_t crc32_table[256]);
    uint32_t crc32(const uint8_t *data, size_t length, uint32_t crc32_table[256]);
    // 配置文件CRC校验值
    uint32_t config_crc32;
    /**
     * @brief 计算字符串的CRC32值
     * @param str 输入字符串（自动转换为字节流）
     * @param crc32_table CRC32校验表（256项）
     * @return 32位CRC校验值（通过crc32函数计算）
     */
    uint32_t calculate_string_crc32(String str, uint32_t crc32_table[256]);
    static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize);
    static int32_t onRead(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize);
    static bool onStartStop(uint8_t power_condition, bool start, bool load_eject);
    static void usbEventCallback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

public:
    void setup();
    void listDir(fs::FS &fs, const char *dirname, uint8_t levels);
    String readFile(fs::FS &fs, const char *path);
    void writeFile(fs::FS &fs, const char *path, const char *message);
    void appendFile(fs::FS &fs, const char *path, const char *message);
    void renameFile(fs::FS &fs, const char *path1, const char *path2);
    void deleteFile(fs::FS &fs, const char *path);
    bool getConfigStr(fs::FS &fs, String &str);
    /**
     * @brief 删除非标准配置文件变体
     * @details 删除文件名以"config.txt"开头 MAC
     */
    void deleteConfigVariants();
    /**
     * @brief 检查配置文件是否存在
     * @return true=存在，false=不存在或无法访问
     * @note 静态函数，可在未实例化类时调用
     */
    static bool config_file_exists();
    /**
     * @brief 检测配置文件是否发生变更
     * @return true=发生变更，false=未变更或无文件
     * @details 通过对比当前内容CRC32与记录值实现变更检测
     */
    bool isConfigChange();
};
#define CRC32_POLYNOMIAL 0xEDB88320

// #endif
