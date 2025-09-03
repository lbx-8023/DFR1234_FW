/**
 * @file    I2cHub.cpp
 * @brief   I2C设备管理中心实现
 *
 * @details 提供I2C总线设备的自动扫描、注册、数据处理及生命周期管理，
 *          支持多传感器接入，通过互斥锁保证线程安全，使用故障计数机制管理设备连接状态。
 */
#include "I2cHub.h"


// 最大连续故障次数（超过则移除设备）
#define MAX_FAIL_COUNT 3

/**
 * @brief I2C设备管理类构造函数
 *
 * @details 初始化互斥锁，用于保护多线程环境下的设备列表操作
 */
I2CDeviceManager::I2CDeviceManager()
{
    mutex = xSemaphoreCreateMutex();
}

/**
 * @brief I2C设备管理类析构函数
 *
 * @details 释放所有注册的I2C设备实例，清空设备映射表和队列
 */
I2CDeviceManager::~I2CDeviceManager()
{
    for (auto &pair : deviceMap)
    {
        delete pair.second.device;
    }
    deviceMap.clear();
    deviceQueue.clear();
}

/**
 * @brief 向管理器注册I2C设备
 *
 * @param addr 设备I2C地址
 * @param device I2C传感器实例指针
 * @details 若地址未注册，则初始化设备并加入队列和映射表；
 *          若地址已存在，释放新传入的设备实例（避免内存泄漏）
 */
void I2CDeviceManager::enqueue(uint8_t addr, I2CSensor *device)
{
    if (deviceMap.find(addr) == deviceMap.end())
    {
        if (device->init()){
            deviceQueue.push_back(device);
            deviceMap[addr] = {device, 0};
        }else{
            delete device;
        }
    }
    else
    {
        delete device;
    }
}

/**
 * @brief 扫描I2C总线上的设备
 *
 * @details 1. 遍历I2C地址范围（1-126），检测在线设备
 *          2. 对新发现的设备自动创建对应传感器实例并注册
 *          3. 对连续丢失的设备进行故障计数，超过阈值则移除
 */
void I2CDeviceManager::searchI2CDevices()
{
    std::set<uint8_t> foundAddrs;

    // for (uint8_t addr = 1; addr < 127; ++addr)
    for(const auto addr : this->kTargetDevices)
    {
        if (xSemaphoreTake(mutex, portMAX_DELAY)) {//  申请锁
            Wire1.beginTransmission(addr);
            if (Wire1.endTransmission() == 0)
            {
                foundAddrs.insert(addr);
                // 若设备未注册，则根据地址创建对应传感器实例
                if (deviceMap.find(addr) == deviceMap.end())
                {
                    switch (addr)
                    {
                    case GestureFaceDetectionSensor::addr:
                        enqueue(addr, new GestureFaceDetectionSensor());
                        break;
                    case GR10_30Sensor::addr:
                        enqueue(addr, new GR10_30Sensor());
                        break;
                    case BME280Sensor::addr:
                        enqueue(addr, new BME280Sensor());
                        break;
                    case URM09Sensor::addr:
                        enqueue(addr, new URM09Sensor());
                        break;
                    case ColorSensor::addr:
                        enqueue(addr, new ColorSensor());
                        break;
                    case AmbientLightSensor::addr:
                        enqueue(addr, new AmbientLightSensor());
                        break;
                    case TripleAxisAccelerometerSensor::addr:
                        enqueue(addr, new TripleAxisAccelerometerSensor());
                        break;
                    case mmWaveSensor::addr:
                        enqueue(addr, new mmWaveSensor());
                        break;
                    case UVSensor::addr:
                        enqueue(addr, new UVSensor());
                        break;
                    case ENS160Sensor::addr:
                        enqueue(addr, new ENS160Sensor());
                        break;
                    case MAX30102Sensor::addr:
                        enqueue(addr, new MAX30102Sensor());
                        break;
                    case SCD4XSensor::addr:
                        enqueue(addr, new SCD4XSensor());
                        break;
                    case BMI160Sensor::addr:
                        enqueue(addr, new BMI160Sensor());
                        break;
                    // case 0x76:
                    // case Axis9OrientationSensor::addr:
                    //     enqueue(Axis9OrientationSensor::addr, new Axis9OrientationSensor());
                    //     break;
                    // 上面的九轴替换为如下的新传感器
                    case Bmx160Sensor::addr:
                        enqueue(addr, new Bmx160Sensor());
                        break;
                    default:
                        break;
                    }
                }
                else
                {
                    // 设备在线，重置故障计数
                    deviceMap[addr].fail_count = 0;
                }
            }
            xSemaphoreGive(mutex); //  释放锁/
        }
    }

    // 处理离线设备：故障计数，超过阈值则移除
    for (auto it = deviceMap.begin(); it != deviceMap.end();)
    {
        if (foundAddrs.find(it->first) == foundAddrs.end())
        {
            it->second.fail_count++; // 离线一次，故障计数加一
            if (it->second.fail_count >= MAX_FAIL_COUNT)
            {
                // 从队列中删除对应的设备指针
                deviceQueue.erase(std::remove(deviceQueue.begin(), deviceQueue.end(), it->second.device), deviceQueue.end());
                // 释放设备实例内存
                delete it->second.device;
                // 从映射表中删除该设备
                it = deviceMap.erase(it);
                continue;
            }
        }
        ++it;
    }
}

/**
 * @brief 处理所有已注册设备的数据采集
 * 
 * @details 1. 遍历设备队列，通过互斥锁保证同一时间只有一个设备访问总线
 *          2. 调用传感器回调函数获取数据，并拼接成JSON格式字符串
 */
void I2CDeviceManager::process()
{
    String tempStr = "";
    for (auto device : deviceQueue)
    {
        if (xSemaphoreTake(mutex, portMAX_DELAY)) //  申请锁
        {
            device->callback();
            tempStr += device->getResStr() + ",";
            xSemaphoreGive(mutex); //  释放锁
        }
    }
    JsonStr = tempStr;
}

/**
 * @brief 获取所有已注册设备的I2C地址
 * 
 * @return std::vector<uint8_t> 设备地址列表
 */
std::vector<uint8_t> I2CDeviceManager::getAllAddresses() const
{
    std::vector<uint8_t> addresses;
    for (const auto &pair : deviceMap)
    {
        addresses.push_back(pair.first);
    }
    return addresses;
}

/**
 * @brief 获取已注册的I2C设备数量
 * 
 * @return uint8_t 设备数量
 */
uint8_t I2CDeviceManager::getI2cCNT()
{
    return static_cast<uint8_t>(deviceQueue.size());
}

/**
 * @brief 获取设备地址的字符串表示（如"0X50,0X51"）
 * 
 * @return String 地址字符串
 */
String I2CDeviceManager::getAddrStr()
{
    String addressString;
    bool first = true;
    char buf[5];

    for (const auto &pair : deviceMap)
    {
        if (!first)
            addressString += ",";
        snprintf(buf, sizeof(buf), "0X%02X", pair.first);
        addressString += String(buf);
        first = false;
    }
    return addressString;
}