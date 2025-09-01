#include "SmartIOManager.h"

SmartIOManager::SmartIOManager() {}

SmartIOManager::~SmartIOManager() {
  for (auto &iohub : iohubs) {
    delete iohub; // 释放动态分配的 IOHub 对象 
  }
}

void SmartIOManager::configIOhub(KeyValue *kv_pairs) {
  char tempKey[16], lowerKey[16];
  char *tempStr;

  for (int i = 0; i < IO_PORT_NUM; i++) {
    sprintf(tempKey, "P%d", i + 1);
    const char *tempStr = get_value_by_key(kv_pairs, MAX_ENTRIES, tempKey);

    uint8_t ioPin = pin_map[i], ioIdx = i + 1;

    if (tempStr == NULL) {
      this->iohubs.push_back(new NanIOHub(ioPin, ioIdx));
      continue;
    }

    this->_str_to_lower_copy(tempStr, lowerKey, sizeof(lowerKey));

    SensorName name = this->_getIOModeFromString(lowerKey);

    // 根据类型设置不同的 IOHub 实例
    switch (name) {
    case IO_DIGITAL_OUT:
      this->iohubs.push_back(new DigitalOutIOHub(ioPin, ioIdx));
      break;
    case IO_ANALOG:
      this->iohubs.push_back(new AnalogIOHub(ioPin, ioIdx));
      break;
    case IO_DHT11:
      this->iohubs.push_back(new DHT11EspIOHub(ioPin, ioIdx));
      break;
    case IO_DS18B20:
      this->iohubs.push_back(new DS18B20IOHub(ioPin, ioIdx));
      break;
    case IO_SERVO180:
      this->iohubs.push_back(new Servo180IOHub(ioPin, ioIdx));
      break;
    case IO_SERVO360:
      this->iohubs.push_back(new Servo360IOHub(ioPin, ioIdx));
      break;
    case IO_WS2812:
      this->iohubs.push_back(new WS2812IOHub(ioPin, ioIdx));
      break;
    case IO_SERVO300:
      this->iohubs.push_back(new Servo300IOHub(ioPin, ioIdx));
      break;
    default:
      this->iohubs.push_back(new NanIOHub(ioPin, ioIdx));
      break;
    }
  }
}

/**
 * @brief 初始化所有 IOHub 实例
 */
void SmartIOManager::init() {
  for (auto &iohub : iohubs) {
    iohub->init();
  }
  // for (auto &iohub : ioConhubs) {
  //   iohub->init();
  // }
}

/**
 * @brief 处理所有传感器的数据采集
 */
void SmartIOManager::process() {
  String tempStr = "";
  for (auto &iohub : iohubs) {
    // if (iohub->getName() == IO_DHT11 || iohub->getName() == IO_DS18B20 ||
    //     iohub->getName() == IO_ANALOG) {
    if(iohub->getType() == IO_GRAB){
      iohub->callback();
      oled.setStaus(iohub->getIOIdx()-1, true);
      if (tempStr.length() > 0)
        tempStr += ",";
      tempStr += iohub->getDataJsonStr();
    }
  }
  if (tempStr.endsWith(",")) {
    tempStr.remove(tempStr.length() - 1);
  }
  this->JsonStr = tempStr;
}

// void SmartIOManager::continueIOProcess(){
//     String tempStr = "";

//     for (auto &iohub : ioConhubs) {
//       iohub->callback();
//       oled.setStaus(iohub->getIOIdx()-1, true);
//       tempStr += iohub->getDataJsonStr() + ",";
//     }
//     this->conJsonStr = tempStr;
// }

/**
 * @brief 处理 MQTT 消息，解析 JSON 并调用相应的 IOHub 方法
 * @param json MQTT 消息的 JSON 字符串
 */
void SmartIOManager::handleMQTTMessage(const char *json) {
  ArduinoJson::StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    printf("JSON decode failed.\n");
    return;
  }
  this->_handleMQTTMessage(doc);
}

void SmartIOManager::_handleMQTTMessage(JsonDocument &doc) {
  for (auto &iohub : iohubs) {
    char key[8];
    uint8_t io_idx = iohub->getIOIdx();
    sprintf(key, "p%d", io_idx);
    if (!doc.containsKey(key)) {
      sprintf(key, "P%d", io_idx);
      if (!doc.containsKey(key)) {
        continue;
      }
    }

    SensorName name = iohub->getName();
    uint8_t pin = iohub->getPin();

    oled.setStaus(io_idx - 1, true);

    if (name == IO_WS2812) {
      const char *value = doc[key];
      iohub->callback(value);
    } else {
      // 处理其他类型控制命令
      std::string valueStr;
      printf("tests %s\n", doc[key]);
      // 根据JSON值类型转换为字符串
      if (doc[key].is<const char *>())
        valueStr = doc[key].as<const char *>();
      else if (doc[key].is<int>())
        valueStr = std::to_string(doc[key].as<int>());
      else if (doc[key].is<float>()) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%.2f", doc[key].as<float>());
        valueStr = buf;
      } else
        continue;
      const char *valCStr = valueStr.c_str();
      printf("str: %s\n", valCStr);
      switch (name) {
      case IO_SERVO180:
        iohub->callback(valCStr);
        break;
      case IO_SERVO300:
        iohub->callback(valCStr);
        break;
      case IO_SERVO360:
        iohub->callback(valCStr);
        break;
      case IO_DIGITAL_OUT:
        iohub->callback(valCStr);
        break;
      default:
        break;
      }
    }
  }
}

/**
 * @brief 将字符串复制并转换为小写
 *
 * @param src 源字符串
 * @param dst 目标缓冲区
 * @param dstSize 目标缓冲区大小
 * @return char* 指向目标缓冲区的指针
 */
char *SmartIOManager::_str_to_lower_copy(const char *src, char *dst,
                                         size_t dstSize) {
  size_t i;
  for (i = 0; i < dstSize - 1 && src[i]; i++) {
    dst[i] = tolower((unsigned char)src[i]);
  }
  dst[i] = '\0';
  return dst;
}

/**
 * @brief 从字符串获取IO模式
 *
 * @details 将配置字符串映射为对应的传感器类型枚举值
 *
 * @param val 表示IO模式的字符串
 * @return SensorName 对应的传感器类型
 */
SensorName SmartIOManager::_getIOModeFromString(char *val) {
  if (!val)
    return IO_NULL;

  if (strcmp(val, "input") == 0) {
    return IO_ANALOG;
  } else if (strcmp(val, "output") == 0) {
    return IO_DIGITAL_OUT;
  } else if (strcmp(val, "ws2812") == 0) {
    return IO_WS2812;
  } else if (strcmp(val, "dht11") == 0) {
    return IO_DHT11;
  } else if (strcmp(val, "ds18b20") == 0) {
    return IO_DS18B20;
  } else if (strcmp(val, "servo180") == 0) {
    return IO_SERVO180;
  } else if (strcmp(val, "servo360") == 0) {
    return IO_SERVO360;
  } else if (strcmp(val, "servo300") == 0) {
    return IO_SERVO300;
  }
  return IO_NULL;
}