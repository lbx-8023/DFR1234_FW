/**
 * @file    ioSensor.cpp
 * @brief   IO传感器模块实现
 *
 * @details 实现了传感器类型配置、数据采集和控制功能，
 *
 */
#include "ioSensor.h"
#include "Display.h"
/** @brief WS2812 LED灯带数组 */
WS2812Strip strips[MAX_STRIPS] = {};

/** @brief 外部声明的OLED显示屏对象 */
extern ScreenDisplay oled;

/** @brief 180度舵机实例数组 */
Servo servo180Instances[IO_PORT_NUM];
/** @brief 360度舵机实例数组 */
Servo servo360Instances[IO_PORT_NUM];

/**
 * @brief IOSensorHub构造函数
 */
IOSensorHub::IOSensorHub()
{
}

/**
 * @brief IOSensorHub析构函数
 */
IOSensorHub::~IOSensorHub()
{
}

/**
 * @brief 根据配置键值对配置IO传感器
 *
 * @details 从配置中读取IO端口类型，初始化相应的传感器回调函数
 *
 * @param kv_pairs 配置键值对数组
 */
void IOSensorHub::configIO(KeyValue *kv_pairs)
{
    // 映射IO索引到物理引脚
    const uint8_t pin_map[IO_PORT_NUM] = {
        IO1_PORT, IO2_PORT, IO3_PORT, IO4_PORT, IO5_PORT, IO6_PORT};

    char tempKey[16], lower[16];
    char *tempStr;
    // 配置每个IO端口
    for (uint8_t i = 0; i < IO_PORT_NUM; i++)
    {
        sprintf(tempKey, "IO%d", i + 1);
        const char *tempStr = get_value_case_insensitive(kv_pairs, MAX_ENTRIES, tempKey);
        // 设置基本参数
        ioSensorNode[i].pin = pin_map[i];
        ioSensorNode[i].io_idx = i + 1;

        // 如果配置不存在，设为NULL类型
        if (tempStr == NULL)
        {
            ioSensorNode[i].type = IO_NULL;
            continue;
        }
        // 转换为小写并确定IO类型
        _str_to_lower_copy(tempStr, lower, sizeof(lower));
        ioSensorNode[i].type = _getIOModeFromString(lower);

        // 根据类型设置回调函数
        switch (ioSensorNode[i].type)
        {
        case IO_DIGITAL_OUT:
            ioSensorNode[i].callBack = DIGITAL_OUT_CALLBACK;
            break;
        case IO_ANALOG:
            ioSensorNode[i].callBack = ANALOG_CALLBACK;
            break;
        case IO_DHT11:
            ioSensorNode[i].callBack = DHT11_CALLBACK;
            break;
        case IO_DS18B20:
            ioSensorNode[i].callBack = DS18B20_CALLBACK;
            break;
        case IO_SERVO180:
            ioSensorNode[i].callBack = SERVO180_CALLBACK;
            break;

        case IO_SERVO360:
            ioSensorNode[i].callBack = SERVO360_CALLBACK;
            break;
        case IO_WS2812:
            ioSensorNode[i].callBack = WS2812_CALLBACK;
            break;
        default:
            ioSensorNode[i].type = IO_NULL;
            ioSensorNode[i].callBack = nullptr;
            break;
        }
    }
}

/**
 * @brief 从字符串获取IO模式
 *
 * @details 将配置字符串映射为对应的传感器类型枚举值
 *
 * @param val 表示IO模式的字符串
 * @return SensorName 对应的传感器类型
 */
SensorName IOSensorHub::_getIOModeFromString(char *val)
{
    if (!val)
        return IO_NULL;

    if (strcmp(val, "input") == 0)
    {
        return IO_ANALOG;
    }
    else if (strcmp(val, "output") == 0)
    {
        return IO_DIGITAL_OUT;
    }
    else if (strcmp(val, "ws2812") == 0)
    {
        return IO_WS2812;
    }
    else if (strcmp(val, "dht11") == 0)
    {
        return IO_DHT11;
    }
    else if (strcmp(val, "ds18b20") == 0)
    {
        return IO_DS18B20;
    }
    else if (strcmp(val, "servo180") == 0)
    {
        return IO_SERVO180;
    }
    else if (strcmp(val, "servo360") == 0)
    {
        return IO_SERVO360;
    }
    return IO_NULL;
}

/**
 * @brief 将字符串复制并转换为小写
 *
 * @param src 源字符串
 * @param dst 目标缓冲区
 * @param dstSize 目标缓冲区大小
 * @return char* 指向目标缓冲区的指针
 */
char *IOSensorHub::_str_to_lower_copy(const char *src, char *dst, size_t dstSize)
{
    size_t i;
    for (i = 0; i < dstSize - 1 && src[i]; i++)
    {
        dst[i] = tolower((unsigned char)src[i]);
    }
    dst[i] = '\0';
    return dst;
}

/**
 * @brief 处理所有传感器数据采集
 *
 * @details 轮询所有传感器，采集数据并更新状态
 */
void IOSensorHub::process()
{
    char arg[16], data[32];
    for (uint8_t i = 0; i < IO_PORT_NUM; i++)
    {
        // 只处理需要主动采集数据的传感器类型
        if (ioSensorNode[i].type == IO_ANALOG || ioSensorNode[i].type == IO_DHT11 || ioSensorNode[i].type == IO_DS18B20)
        {

            oled.setStaus(ioSensorNode[i].io_idx-1, true);
            ioSensorNode[i].callBack(ioSensorNode[i].pin, ioSensorNode[i].io_idx, ioSensorNode[i].type, arg, ioSensorNode[i].value);
        }
    }
    // 将所有传感器数据转换为JSON格式
    toJsonStr();
}

/**
 * @brief 将所有传感器数据转换为JSON字符串
 *
 * @details 构建包含所有传感器数据的JSON字符串
 */
void IOSensorHub::toJsonStr()
{
    JsonStr = "";
    bool first = true;
    // 拼接所有传感器数据
    for (uint8_t i = 0; i < IO_PORT_NUM; i++)
    {
        if (strlen(ioSensorNode[i].value) > 0)
        {
            if (!first)
                JsonStr += ",";
            JsonStr += String(ioSensorNode[i].value);
            first = false;
        }
    }
    // 移除末尾多余的逗号
    while (JsonStr.endsWith(","))
        JsonStr = JsonStr.substring(0, JsonStr.length() - 1);
}

/**
 * @brief 处理MQTT控制消息
 *
 * @details 解析JSON格式的控制消息并执行相应操作
 *
 * @param json JSON格式的控制消息
 */
void IOSensorHub::handleMQTTMessage(const char *json)
{
    // 使用静态JSON文档解析消息
    ArduinoJson::StaticJsonDocument<512> doc; // 仍然使用静态分配，但避免类名弃用的警告
    DeserializationError error = deserializeJson(doc, json);
    if (error)
    {
        printf("JSON decode failed\n ");
        return;
    }
    // 处理控制命令
    _handleControlJson(doc);
}

/**
 * @brief 处理控制JSON消息
 *
 * @details 解析控制命令并执行相应操作
 *
 * @param doc 解析后的JSON文档
 */
void IOSensorHub::_handleControlJson(JsonDocument &doc)
{
    // 遍历所有IO端口，查找控制命令
    for (uint8_t i = 0; i < IO_PORT_NUM; i++)
    {
        char key[8];
        snprintf(key, sizeof(key), "io%d", i + 1);

        if (!doc.containsKey(key))
            continue;
        // 获取传感器信息
        SensorName name = ioSensorNode[i].type;
        uint8_t pin = ioSensorNode[i].pin;
        uint8_t idx = ioSensorNode[i].io_idx;
        oled.setStaus(idx-1, true);

        // 处理WS2812灯带控制
        if (type == IO_WS2812)
        {
            const char *value = doc[key];
            WS2812_CALLBACK(pin, idx, type, value, nullptr);
        }
        else
        {
            // 处理其他类型控制命令
            std::string valueStr;
            // 根据JSON值类型转换为字符串
            if (doc[key].is<const char *>())
                valueStr = doc[key].as<const char *>();
            else if (doc[key].is<int>())
                valueStr = std::to_string(doc[key].as<int>());
            else if (doc[key].is<float>())
            {
                char buf[32];
                snprintf(buf, sizeof(buf), "%.2f", doc[key].as<float>());
                valueStr = buf;
            }
            else
                continue;
            const char *valCStr = valueStr.c_str();

            // 根据传感器类型执行相应控制
            switch (type)
            {
            case IO_SERVO180:
                SERVO180_CALLBACK(pin, idx, type, valCStr, nullptr);
                break;
            case IO_SERVO360:
                SERVO360_CALLBACK(pin, idx, type, valCStr, nullptr);
                break;
            case IO_DIGITAL_OUT:
                DIGITAL_OUT_CALLBACK(pin, idx, type, valCStr, nullptr);
                break;
            default:
                break;
            }
        }
    }
}

/**
 * @brief 数字输出回调函数
 *
 * @details 控制数字输出引脚的高低电平
 *
 * @param pin GPIO引脚号
 * @param io_idx IO索引
 * @param type 传感器类型
 * @param arg 控制参数（"on"/"off"或数字值）
 * @param res 返回结果（未使用）
 */
void DIGITAL_OUT_CALLBACK(uint8_t pin, uint8_t io_idx, SensorName name, const char *arg, char *res)
{
    pinMode(pin, OUTPUT);
    bool setHigh = false;
    char lowerVal[16];
    // 转换为小写进行比较
    strncpy(lowerVal, arg, sizeof(lowerVal));
    for (char *p = lowerVal; *p; ++p)
        *p = tolower(*p);
    // 判断控制命令
    if (strcmp(lowerVal, "on") == 0)
    {
        setHigh = true;
    }
    else if (strcmp(lowerVal, "off") == 0)
    {
        setHigh = false;
    }
    else
    {
        // 如果不是 "on" 或 "off"，尝试将其转换为浮点数
        float val = atof(arg);
        if (val == 1.0f)
            setHigh = true;
    }
    // 设置引脚电平
    digitalWrite(pin, setHigh ? HIGH : LOW);
}

/**
 * @brief 模拟输入回调函数
 *
 * @details 读取模拟输入引脚的值并格式化输出
 *
 * @param pin GPIO引脚号
 * @param io_idx IO索引
 * @param type 传感器类型
 * @param arg 控制参数（未使用）
 * @param res 返回结果（格式化的模拟值）
 */
void ANALOG_CALLBACK(uint8_t pin, uint8_t io_idx, SensorName name, const char *arg, char *res)
{
    char tempStr[32];
    // 读取模拟值并归一化到0-1范围
    float val = analogRead(pin) / 4096.0;
    sprintf(res, "\"io%d_input_val\":%.2f", io_idx, val);
}

/**
 * @brief DHT11传感器回调函数
 *
 * @details 读取DHT11温湿度传感器数据
 *
 * @param pin GPIO引脚号
 * @param io_idx IO索引
 * @param type 传感器类型
 * @param arg 控制参数（未使用）
 * @param res 返回结果（温湿度数据）
 */
void DHT11_CALLBACK(uint8_t pin, uint8_t io_idx, SensorName name, const char *arg, char *res)
{
    LOG_DEBUG("DHT11_CALLBACK\n");
    DHT dht(pin, 11);
    dht.begin();
    float humidity = dht.readHumidity();       // 读取湿度
    float temperature = dht.readTemperature(); // 读取温度

    if (isnan(humidity) || isnan(temperature))
    {
        sprintf(res, "\"io%d_dht11_humi\":\"NAN\",\"io%d_dht11_temp\":\"NAN\"", io_idx, io_idx);
    }
    else
    {
        sprintf(res, "\"io%d_dht11_humi\":%.2f,\"io%d_dht_temp\":%.2f", io_idx, humidity, io_idx, temperature);
    }
}

/**
 * @brief DS18B20传感器回调函数
 *
 * @details 读取DS18B20温度传感器数据
 *
 * @param pin GPIO引脚号
 * @param io_idx IO索引
 * @param type 传感器类型
 * @param arg 控制参数（未使用）
 * @param res 返回结果（温度数据）
 */
void DS18B20_CALLBACK(uint8_t pin, uint8_t io_idx, SensorName name, const char *arg, char *res)
{
    LOG_DEBUG("DS18B20_CALLBACK\n");
    // 初始化DS18B20传感器
    OneWire oneWire(pin); // 初始连接在单总线上的单总线设备
    DallasTemperature sensors(&oneWire);
    sensors.begin();
    sensors.requestTemperatures();
    float temperature = sensors.getTempCByIndex(0);
    sprintf(res, "\"io%d_ds18b20_temp\":%.2f", io_idx, temperature);

    if (temperature == -127)
    {
        sprintf(res, "\"io%d_ds18b20_temp\":\"NAN\"", io_idx);
    }
}

/**
 * @brief 180度舵机回调函数
 *
 * @details 控制180度舵机转动到指定角度
 *
 * @param pin GPIO引脚号
 * @param io_idx IO索引
 * @param type 传感器类型
 * @param arg 控制参数（角度值，0-180）
 * @param res 返回结果（未使用）
 */
void SERVO180_CALLBACK(uint8_t pin, uint8_t io_idx, SensorName name, const char *arg, char *res)
{
    LOG_DEBUG("SERVO180_CALLBACK\n");
    int angle = atoi(arg);
    // 解析角度值并限制在有效范围内
    angle = constrain(angle, 0, 180);
    // 获取对应舵机实例并控制
    Servo &servo = servo180Instances[io_idx - 1];
    if (!servo.attached())
    {
        servo.attach(pin);
    }
    servo.write(angle);
}

/**
 * @brief 360度舵机回调函数
 *
 * @details 控制连续旋转舵机的转速和方向,存在死区值，需要值够大
 *
 * @param pin GPIO引脚号
 * @param io_idx IO索引
 * @param type 传感器类型
 * @param arg 控制参数（速度值，-100到100）
 * @param res 返回结果（未使用）
 */
void SERVO360_CALLBACK(uint8_t pin, uint8_t io_idx, SensorName name, const char *arg, char *res)
{
    LOG_DEBUG("SERVO360_CALLBACK %d:%s \n", pin, arg);
    int speed = atoi(arg);
    // 解析速度值并限制在有效范围内
    speed = constrain(speed, -100, 100); // 限制输入范围

    // 舵机中点（停止）微调值（某些舵机可能不是刚好1500us）
    const int center_us = 1500;
    const int min_us = 1000; // 全速反转
    const int max_us = 2000; // 全速正转
    const int dead_zone = 2; // 死区范围 ±5（避免小范围抖动）

    // 获取对应舵机实例
    Servo &servo = servo360Instances[io_idx - 1];

    if (!servo.attached())
    {
        servo.setPeriodHertz(50);      // 舵机频率为 50Hz
        servo.attach(pin, 1000, 2000); // 设置舵机脉宽范围
    }

    int pulse_us;

    // 死区处理：微小值不驱动舵机，避免抖动
    if (abs(speed) <= dead_zone)
    {
        pulse_us = center_us; // 静止
    }
    else if (speed > 0)
    {
        // 正转映射：center_us ~ max_us
        pulse_us = map(speed, dead_zone + 1, 100, center_us + 10, max_us);
    }
    else
    {
        // 反转映射：center_us ~ min_us
        pulse_us = map(-speed, dead_zone + 1, 100, center_us - 10, min_us);
    }
    // 设置舵机脉冲宽度
    servo.writeMicroseconds(pulse_us);
}

/**
 * @brief 根据引脚获取或创建WS2812灯带实例
 *
 * @details 如果指定引脚的灯带已初始化，则返回其指针；
 *          否则创建新的灯带实例并初始化
 *
 * @param pin GPIO引脚号
 * @return WS2812Strip* 灯带实例指针，失败时返回nullptr
 */
WS2812Strip *getStripByPin(uint8_t pin)
{
    for (int i = 0; i < MAX_STRIPS; ++i)
    {
        if (strips[i].led && strips[i].pin == pin)
        {
            return &strips[i];
        }
    }

    for (int i = 0; i < MAX_STRIPS; ++i)
    {
        if (!strips[i].led)
        {
            strips[i].pin = pin;
            strips[i].led = new LiteLED(LED_TYPE, LED_TYPE_IS_RGBW);
            strips[i].led->begin(pin, MAX_PIXELS);
            strips[i].led->brightness(160);
            strips[i].initialized = true;
            return &strips[i];
        }
    }

    return nullptr;
}

/**
 * @brief WS2812 LED灯带回调函数
 *
 * @details 控制WS2812 RGB LED灯带显示指定颜色
 *
 * @param pin GPIO引脚号
 * @param io_idx IO索引
 * @param type 传感器类型
 * @param arg 控制参数（RGB值字符串，格式："R G B R G B ..."）
 * @param res 返回结果（操作状态）
 */
void WS2812_CALLBACK(uint8_t pin, uint8_t io_idx, SensorName name, const char *arg, char *res)
{
    // 获取或创建灯带实例
    WS2812Strip *s = getStripByPin(pin);
    if (!s || !s->led)
    {
        if (res)
            snprintf(res, 64, "WS2812 init failed");
        return;
    }

    // 解析 RGB 数据
    char buf[512];
    strncpy(buf, arg, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    uint8_t values[MAX_CHANNELS];
    int idx = 0;
    char *token = strtok(buf, " ");
    while (token && idx < MAX_CHANNELS)
    {
        values[idx++] = atoi(token);
        token = strtok(NULL, " ");
    }

    int pixelCount = idx / 3;
    if (pixelCount > MAX_PIXELS)
        pixelCount = MAX_PIXELS;

    // 设置像素颜色
    for (int i = 0; i < pixelCount; ++i)
    {
        uint8_t r = values[i * 3 + 0];
        uint8_t g = values[i * 3 + 1];
        uint8_t b = values[i * 3 + 2];
        crgb_t color = (r << 16) | (g << 8) | b;
        s->led->setPixel(i, color, 0); // 0: 不自动 show
    }

    s->led->show(); // 批量刷新

    if (res)
        snprintf(res, 64, "OK %d LEDs on pin %d", pixelCount, pin);
}
