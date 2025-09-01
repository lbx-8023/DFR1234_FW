#ifndef __IOHUB_H_
#define __IOHUB_H_

#include "global.h"

#include "ConfigParser.h"
#include "ArduinoJson.h"
#include "OneWire.h"
#include "DallasTemperature.h"
#include "DHT.h"
#include <ESP32Servo.h>
#include "LiteLED.h"
#include "DHTesp.h"
#include "DFRobot_DHT11.h"
#include "IDFDHT11.h"


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
    IO_SERVO360,    ///< 360度连续旋转舵机
    IO_SERVO300,    ///< 300度舵机
} SensorName;


typedef enum{
    IO_CONTROL,
    IO_GRAB
} SensorType;
/** @brief 最大支持的WS2812灯带数量 */
#define MAX_STRIPS IO_PORT_NUM
#define MAX_PIXELS 36
#define LED_TYPE LED_STRIP_WS2812
#define LED_TYPE_IS_RGBW 0
#define MAX_CHANNELS (MAX_PIXELS * 3)

class IOHub {
public:
    IOHub(uint8_t pin, uint8_t ioIdx, SensorName name=IO_NULL, SensorType type=IO_GRAB): _pin(pin), _ioIdx(ioIdx), _name(name), _type(type) {}
    ~IOHub() {}
    virtual void init() = 0;
    virtual void callback(const char *arg = nullptr) = 0;
    String getDataJsonStr() {return this->_JsonStr;}
    void setPin(uint8_t pin) { this->_pin = pin; }
    void setName(SensorName name) { this->_name = name; }
    void setIOIdx(uint8_t ioIdx) { this->_ioIdx = ioIdx; }
    uint8_t getPin() { return this->_pin; }
    SensorName getName() { return this->_name; }
    uint8_t getIOIdx() { return this->_ioIdx; }
    void setType(SensorType type) { this->_type = type; }
    SensorType getType() { return this->_type; }

protected:
    uint8_t _pin; // IO 口编号
    uint8_t _ioIdx; // IO 功能编号，用于区分不同的 IO 功能
    SensorName _name; // 传感器类型
    String _JsonStr; // 存储 JSON 数据的字符串
    SensorType _type; // 传感器类型
};


class NanIOHub : public IOHub {
public:
    NanIOHub(uint8_t pin, uint8_t ioIdx, SensorName name=IO_NULL, SensorType type=IO_GRAB):IOHub(pin, ioIdx, name, type){}
    ~NanIOHub() {}
    void init() override;
    void callback(const char *arg = nullptr) override;
};

class AnalogIOHub : public IOHub {
public:
    AnalogIOHub(uint8_t pin, uint8_t ioIdx, SensorName name=IO_ANALOG, SensorType type=IO_GRAB):IOHub(pin, ioIdx, name, type){}
    ~AnalogIOHub() {}
    void init() override; 
    void callback(const char *arg = nullptr) override;
};

class DigitalOutIOHub : public IOHub {
public:
    DigitalOutIOHub(uint8_t pin, uint8_t ioIdx, SensorName name=IO_DIGITAL_OUT, SensorType type=IO_GRAB):IOHub(pin, ioIdx, name, type){}
    ~DigitalOutIOHub() {}
    void init() override;
    void callback(const char *arg = nullptr) override;
};

/**
 * 这里有多个DHT11是因为dht11的时序比较严格，在这个系统中调度有点问题，无论使用哪种都会有概率出现错误数据，加锁目前也没有生效。
 * 目前用的是DHT11EspIOHub
 */
class DHT11IOHub : public IOHub {
public:
    DHT11IOHub(uint8_t pin, uint8_t ioIdx, SensorName name=IO_DHT11, SensorType type=IO_GRAB):IOHub(pin, ioIdx, name, type){}
    ~DHT11IOHub() { delete this->_dht;}
    void init() override;
    void callback(const char *arg = nullptr) override;

private:
    DHT *_dht; // DHT11 传感器实例
};

class IDFDHT11Hub : public IOHub {
public:
    IDFDHT11Hub(uint8_t pin, uint8_t ioIdx, SensorName name=IO_DHT11, SensorType type=IO_GRAB):IOHub(pin, ioIdx, name, type){}
    ~IDFDHT11Hub() {}
    void init() override;
    void callback(const char *arg = nullptr) override;
};

class DHT11EspIOHub : public IOHub {
public:
    DHT11EspIOHub(uint8_t pin, uint8_t ioIdx, SensorName name=IO_DHT11, SensorType type=IO_GRAB):IOHub(pin, ioIdx, name, type){}
    ~DHT11EspIOHub() { delete this->_dht;}
    void init() override;
    void callback(const char *arg = nullptr) override;

private:
    DHTesp *_dht; // DHT11 传感器实例
    float _humidity = NAN;
    float _temperature = NAN;
    TickType_t _xLastNanTime;
    bool _validData = false;
};

class DFRobotDHT11IOHub : public IOHub {
public:
    DFRobotDHT11IOHub(uint8_t pin, uint8_t ioIdx, SensorName name=IO_DHT11, SensorType type=IO_GRAB):IOHub(pin, ioIdx, name, type){}
    ~DFRobotDHT11IOHub() {}
    void init() override;
    void callback(const char *arg = nullptr) override;
private:
    // DFRobot_DHT11 *_dht; // DHT11 传感器实例
};

class DS18B20IOHub : public IOHub {
public:
    DS18B20IOHub(uint8_t pin, uint8_t ioIdx, SensorName name=IO_DS18B20, SensorType type=IO_GRAB):IOHub(pin, ioIdx, name, type){}
    ~DS18B20IOHub() {  delete this->_ds18b20; delete this->_oneWire; }
    float getMyTemp();
    void init() override;
    void callback(const char *arg = nullptr) override;

private:
    OneWire *_oneWire; // OneWire 实例
    DallasTemperature *_ds18b20; // DS18B20 传感器实例
};

class Servo180IOHub : public IOHub {
public:
    Servo180IOHub(uint8_t pin, uint8_t ioIdx, SensorName name=IO_SERVO180, SensorType type=IO_CONTROL):IOHub(pin, ioIdx, name, type){}
    ~Servo180IOHub() {  delete this->_servo;}
    void init() override;
    void callback(const char *arg = nullptr) override;

private:
    Servo *_servo; // 舵机实例
};

class Servo360IOHub : public IOHub {
public:
    Servo360IOHub(uint8_t pin, uint8_t ioIdx, SensorName name=IO_SERVO360, SensorType type=IO_CONTROL):IOHub(pin, ioIdx, name, type){}
    ~Servo360IOHub() {  delete this->_servo;}
    void init() override;
    void callback(const char *arg = nullptr) override;

private:
    Servo *_servo; // 舵机实例
};

class WS2812IOHub : public IOHub {
public:
    WS2812IOHub(uint8_t pin, uint8_t ioIdx, SensorName name=IO_WS2812, SensorType type=IO_CONTROL):IOHub(pin, ioIdx, name, type){}
    ~WS2812IOHub() {  delete this->_led;}
    void init() override;
    void callback(const char *arg = nullptr) override;

private:
    LiteLED *_led; // LiteLED 实例
};

class Servo300IOHub : public IOHub {
public:
    Servo300IOHub(uint8_t pin, uint8_t ioIdx, SensorName name=IO_SERVO300, SensorType type=IO_CONTROL):IOHub(pin, ioIdx, name, type){}
    ~Servo300IOHub() {  delete this->_servo;}
    void init() override;
    void callback(const char *arg = nullptr) override;

private:
    Servo *_servo; // 300度舵机实例
};

#endif