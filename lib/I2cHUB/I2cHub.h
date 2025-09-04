/**
 * @file    sensors.h
 * @brief   I2C传感器抽象与具体传感器实现
 *
 * @details 定义了I2C传感器的抽象基类和多种具体传感器的实现，
 *
 */
#pragma once
#include "global.h"

// #ifndef __I2CHUB_H_
// #define __I2CHUB_H_

#include "DFRobot_LIS2DH12.h"
#include "DFRobot_BNO055.h"
#include "DFRobot_UVIndex240370Sensor.h"
#include "DFRobot_GR10_30.h"
#include "DFRobot_BME280.h"
#include "DFRobot_GestureFaceDetection.h"
#include "DFRobot_LIS.h"
#include "DFRobot_TCS34725.h"
#include "DFRobot_URM09.h"
#include "DFRobot_VEML7700.h"
#include "DFRobot_C4001.h"
#include "DFRobot_BMX160.h"
#include "DFRobot_ENS160.h"
#include "SmartIOManager.h"
#include "DFRobot_SCD4X.h"
#include "DFRobot_BloodOxygen_S.h"
#include "DFRobot_BMI160.h"

class I2CHub
{
public:
    /**
     * @brief 初始化传感器
     *
     * @details 执行传感器的初始化配置，如设置通信速率、校准参数等。
     */
    virtual bool init() = 0;

    /** @brief 传感器I2C地址（静态常量） */
    static const uint8_t addr;

    /**
     * @brief 获取传感器I2C地址
     *
     * @return uint8_t 传感器的I2C地址
     */
    virtual uint8_t getAddr() const = 0;

    /** @brief 传感器名称 */
    String name;

    /**
     * @brief 传感器数据采集回调函数
     *
     * @details 执行传感器数据采集并更新内部数据缓存。
     */
    virtual void callback() = 0;

    /**
     * @brief 获取传感器数据字符串
     *
     * @return String 格式化的传感器数据字符串
     */

    String getResStr();

    /** @brief 虚析构函数，确保正确释放派生类资源 */
    virtual ~I2CHub() = default;

protected:
    /** @brief 传感器数据缓存 */
    String data;
};

class GestureFaceDetectionI2CHub : public I2CHub
{
public:
    static const uint8_t addr = 0x72;
    uint8_t getAddr() const override { return 0x72; }
    virtual bool init() override;
    void callback() override;

private:
    DFRobot_GestureFaceDetection_I2C *gfd;
};

class GR10_30I2CHub : public I2CHub
{
public:
    static const uint8_t addr = 0x73;
    uint8_t getAddr() const override { return 0x73; }
    virtual bool init() override;
    void callback() override;

private:
    DFRobot_GR10_30 *gr10_30;
};
class BME280I2CHub : public I2CHub
{
public:
    static const uint8_t addr = 0x77;
    uint8_t getAddr() const override { return 0x77; }
    virtual bool init() override;
    void callback() override;

private:
    DFRobot_BME280_IIC *bme;
};
class URM09I2CHub : public I2CHub
{
public:
    static const uint8_t addr = 0x11;
    uint8_t getAddr() const override { return 0x11; }
    virtual bool init() override;
    void callback() override;

private:
    DFRobot_URM09 *URM09;
};

class ColorI2CHub : public I2CHub
{
public:
    static const uint8_t addr = 0x29;
    uint8_t getAddr() const override { return 0x29; }
    virtual bool init() override;
    void callback() override;

private:
    DFRobot_TCS34725 *tcs;
};
class AmbientLightI2CHub : public I2CHub
{
public:
    static const uint8_t addr = 0x10;
    uint8_t getAddr() const override { return 0x10; }
    virtual bool init() override;
    void callback() override;

private:
    DFRobot_VEML7700 *als;
};

class TripleAxisAccelerometerI2CHub : public I2CHub
{
public:
    static const uint8_t addr = 0x18;
    uint8_t getAddr() const override { return 0x18; }
    virtual bool init() override;
    void callback() override;

private:
    DFRobot_LIS2DH12 *acce;
};

class mmWaveI2CHub : public I2CHub
{
public:
    static const uint8_t addr = 0x2A;
    uint8_t getAddr() const override { return 0x2A; }
    virtual bool init() override;
    void callback() override;

private:
    DFRobot_C4001_I2C *radar;
};

class UVI2CHub : public I2CHub
{
public:
    static const uint8_t addr = 0x23;
    uint8_t getAddr() const override { return 0x23; }
    virtual bool init() override;
    void callback() override;

private:
    DFRobot_UVIndex240370Sensor *UVIndex240370Sensor;
};

class Axis9OrientationI2CHub : public I2CHub
{
public:
    static const uint8_t addr = 0x28;
    uint8_t getAddr() const override { return 0x28; }
    virtual bool init() override;
    void callback() override;

private:
    DFRobot_BNO055_IIC *bno;
};

class Bmx160I2CHub : public I2CHub
{
public:
    static const uint8_t addr = 0x68;
    uint8_t getAddr() const override { return 0x68; }
    virtual bool init() override;
    void callback() override;

private:
    DFRobot_BMX160 *bmx;
};

class ENS160I2CHub : public I2CHub
{
public:
    static const uint8_t addr = 0x52;
    uint8_t getAddr() const override { return 0x52; }
    virtual bool init() override;
    void callback() override;

private:
    DFRobot_ENS160 *ens160;
};

class MAX30102I2CHub : public I2CHub
{
public:
    static const uint8_t addr = 0x57;
    uint8_t getAddr() const override { return 0x57; }
    virtual bool init() override;
    void callback() override;

private:
    DFRobot_BloodOxygen_S_I2C *max30102;
};

class SCD4XI2CHub : public I2CHub
{
public:
    static const uint8_t addr = 0x62;
    uint8_t getAddr() const override { return 0x62; }
    virtual bool init() override;
    void callback() override;
     DFRobot_SCD4X::sSensorMeasurement_t scd4xData;
private:
    DFRobot_SCD4X *scd4x;
   
};

class BMI160I2CHub : public I2CHub
{
public:
    static const uint8_t addr = 0x69;
    uint8_t getAddr() const override { return 0x69; }
    virtual bool init() override;
    void callback() override;
private:
    DFRobot_BMI160 *bmi160;
   
};
// #endif