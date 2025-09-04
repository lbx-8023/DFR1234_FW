
/**
 * @file    SmartI2CManager.h
 * @brief   I2C设备管理中心头文件
 *
 * @details 定义I2C设备管理器接口，提供自动扫描、注册、数据处理及生命周期管理功能，
 *          支持多传感器接入和线程安全操作。
 */
#pragma once
#include "global.h"
#ifndef __SMARTI2CMANAGER_H_
#define __SMARTI2CMANAGER_H_


#include "I2CHub.h"
/**
 * @brief 设备包装结构体
 *
 * @details 存储I2C传感器指针及其故障计数，用于设备状态管理
 */
struct DeviceWrapper
{
    I2CHub *device;
    uint8_t fail_count;
};

/**
 * @brief I2C设备管理类
 *
 * @details 提供I2C总线设备的自动扫描、注册、数据处理及生命周期管理，
 *          通过互斥锁保证线程安全，支持多传感器接入。
 */
class I2CDeviceManager
{
public:
    /** @brief 构造函数，初始化互斥锁 */
    I2CDeviceManager();

    /** @brief 析构函数，释放所有注册的设备资源 */
    ~I2CDeviceManager();

    /**
     * @brief 注册I2C设备
     *
     * @param addr 设备I2C地址
     * @param device I2C传感器实例指针
     */
    void enqueue(uint8_t addr, I2CHub *device);

    /**
     * @brief 扫描I2C总线上的设备
     *
     * @details 遍历I2C地址范围，检测在线设备，自动注册新设备，
     *          并对离线设备进行故障计数管理。
     *          建议定期调用此函数（如每3秒一次）。
     */
    void searchI2CDevices();

    /**
     * @brief 处理所有已注册设备的数据采集
     *
     * @details 遍历设备队列，调用每个传感器的回调函数获取数据，
     *          并将结果拼接为JSON格式字符串存储在JsonStr中。
     */
    void process();

    /**
     * @brief 获取所有已注册设备的I2C地址
     *
     * @return std::vector<uint8_t> 设备地址列表
     */
    std::vector<uint8_t> getAllAddresses() const;

    /**
     * @brief 获取已注册的I2C设备数量
     *
     * @return uint8_t 设备数量（最大255）
     */
    uint8_t getI2cCNT();

    /**
     * @brief 获取设备地址的字符串表示
     *
     * @return String 地址字符串（格式如"0X50,0X51"）
     */
    String getAddrStr();

    /** @brief 存储所有传感器数据的JSON格式字符串 */
    String JsonStr;

private:
    /** @brief 设备映射表（键：I2C地址，值：设备包装结构体） */
    std::map<uint8_t, DeviceWrapper> deviceMap;

    /** @brief 设备处理队列（按注册顺序存储传感器指针） */
    std::vector<I2CHub *> deviceQueue;

    /** @brief 互斥锁（用于保护多线程环境下的共享资源） */
    SemaphoreHandle_t mutex;

    // 预定义需要扫描的设备地址列表
    const std::vector<uint8_t> kTargetDevices = {
        GestureFaceDetectionI2CHub::addr,
        GR10_30I2CHub::addr,
        BME280I2CHub::addr,
        URM09I2CHub::addr,
        ColorI2CHub::addr,
        AmbientLightI2CHub::addr,
        TripleAxisAccelerometerI2CHub::addr,
        mmWaveI2CHub::addr,
        UVI2CHub::addr,
        Bmx160I2CHub::addr,
        ENS160I2CHub::addr,
        MAX30102I2CHub::addr,
        SCD4XI2CHub::addr,
        BMI160I2CHub::addr,
    };
};

/**
 * @brief I2C扫描任务函数
 * 
 * @param args I2CDeviceManager实例指针
 * @details 用于创建独立的FreeRTOS任务，定期调用I2C扫描函数。
 *          示例用法：xTaskCreate(i2c_scan_task, "I2C_SCAN", 2048, &manager, 5, NULL);
 */
void i2c_scan_task(void *args);

#endif // __I2CHub_H_
