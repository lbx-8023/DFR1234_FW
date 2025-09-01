/**
 * @file    app_task.cpp
 * @brief   应用任务管理实现
 *
 * @details 多任务调度，包含系统状态管理、WiFi连接、MQTT通信、
 */
#include "app_task.h"
#include "ConfigParser.h"
#include "DAT.h"
#include "FirmwareUpdater.h"
#include "I2cHub.h"
#include "MQTTAPP.h"
#include "esp32wifi.h"
#include "msc.h"
// #include "ioSensor.h"
#include "Display.h"
#include "mqtt_subscriber.h"

#include "SmartIOManager.h"

FirmwareUpdater firmwareUpdater; // 固件升级实例
// 任务句柄
TaskHandle_t main_handle, wifiConfig_handle, flushConfig_handle, display_handle, mqtt_handle, sensor_handle, mqttJson_handle,
    mqttPublish_handle;

String JsonSensorData = "";
SemaphoreHandle_t JsonDataMutex = xSemaphoreCreateMutex(); // MQTT互斥锁

// 全局实例化组件对象
ScreenDisplay oled;
ESP32WiFi esp32wifi;
I2CDeviceManager i2cDeviceManager;
// 外部声明USB-MSC实例
extern USB_MSC msc;
// 配置解析
KeyValue keyValue[MAX_ENTRIES];         // 键值对数组（存储配置文件内容）
SystemState sys_state = {0};            // 系统状态结构体（含互斥锁）
QueueHandle_t xSystemEventQueue = NULL; // 系统事件队列（任务间通信）
QueueHandle_t jsonPtrQueue;             // JSON数据队列（存储传感器数据）

SemaphoreHandle_t mqtt_mutex = xSemaphoreCreateMutex(); // MQTT互斥锁
// IOSensorHub ioSensorHub;                                // IO传感器中枢实例
SmartIOManager smartIOManager; // 智能IO管理器实例

float UpdateIntervalTime = 1.0; // 数据上报间隔（秒）

bool lastButtonState = HIGH;                                // 上一次按钮状态
SemaphoreHandle_t wakeSemaphore = xSemaphoreCreateBinary(); // 唤醒信号量
QueueHandle_t mqttSubQueue;                                 // MQTT消息队列
bool StateLED = true;                                       // 状态灯

static char *str_to_lower(const char *src, char *dst, size_t dstSize) {
  size_t i;
  for (i = 0; i < dstSize - 1 && src[i] != '\0'; i++) {
    dst[i] = tolower((unsigned char)src[i]);
  }
  dst[i] = '\0'; // 确保字符串以空字符结尾
  return dst;
}



/**
 * @brief mqtt发布数据
 * 
 * @param pvParameters 
 */
void mqttPublishTask(void *pvParameters) {
  while (1) {
    if (sys_state.mqtt_running) {
      // printf("%s\n", resStr.c_str());
      String resStr = "";
      char *jsonPtr = NULL;
      if (xSemaphoreTake(JsonDataMutex, portMAX_DELAY)) {
        jsonPtr = (char *)malloc(JsonSensorData.length() + 1);
        resStr = JsonSensorData;
        xSemaphoreGive(JsonDataMutex);
      }

      if (jsonPtr != NULL && sys_state.mqtt_running) {
        strcpy(jsonPtr, resStr.c_str());
        // LOG_INFO("%s/n", jsonPtr);
        xQueueSend(jsonPtrQueue, &jsonPtr, portMAX_DELAY);

      } else {
        LOG_ERROR("[Error]JSON\n");
      }
      // 限制上报间隔在有效范围内（0.1-10秒）
      if (UpdateIntervalTime < 0.02f || UpdateIntervalTime > 10.0f) {
        UpdateIntervalTime = 1.0f;
      }

      int16_t IntervalTime = 1000 * UpdateIntervalTime;
      // LOG_DEBUG("IntervalTime:%d\n", IntervalTime);
      vTaskDelay(pdTICKS_TO_MS(IntervalTime));
    } else {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
}

/**
 * @brief 任务启动函数
 *
 * @details 1. 固件升级检查
 *          2. 创建数据队列（JSON和MQTT消息）
 *          3. 启动各功能任务（主任务、显示、I2C扫描、MQTT等）
 * @note 任务优先级：数值越大优先级越高
 */
void start_task(void) {
  firmwareUpdater.performUpdate("/firmware.bin");
  // 检查并执行固件升级（从文件系统加载/firmware.bin）
  jsonPtrQueue = xQueueCreate(8, sizeof(char *));
  mqttSubQueue = xQueueCreate(8, sizeof(MQTTMessage));
  if (!jsonPtrQueue) {
    printf("Queue or mutex creation failed!\n");
    esp_restart();
  }
  xTaskCreate(app_main, "MAINAPP", 4096 * 3, NULL, 4, &main_handle);
  vTaskDelay(200);
  xTaskCreate(display_task, "DISPLAY", 4096 * 5, NULL, 3, &display_handle);
  vTaskDelay(10);
  xTaskCreate(i2c_scan_task, "i2c_scan_task", 4096, &i2cDeviceManager, 3, NULL);
  vTaskDelay(5);
  xTaskCreate(MQTT_ServerTask, "mqtt", 4096, NULL, 2, &mqtt_handle);
  vTaskDelay(20);
  xTaskCreate(SensorHub_Task, "sensor", 4096 * 4, NULL, 6, &sensor_handle);
  vTaskDelay(5);
  xTaskCreate(mqttPublishTask, "mqttPublishTask", 4096 * 4, NULL, 7, &mqttPublish_handle);
  vTaskDelay(5);

  xTaskCreate(mqttHandlerTask, "SubTask", 4096 * 5, NULL, 8, &mqttJson_handle);
  vTaskDelay(5);
  xTaskCreate(buttonPollTask, "buttonPollTask", 2048, NULL, 2, NULL);
  vTaskDelay(5);
  xTaskCreate(datLedTask, "datLedTask", 2048, NULL, 1, NULL);
}

/**
 * @brief 主应用任务（系统事件处理核心）
 *
 * @param args null
 * @details 1. 初始化系统状态和事件队列
 *          2. 处理系统事件
 */
void app_main(void *args) {

  sys_state.mutex = xSemaphoreCreateMutex();
  sys_state.wifi_connected = false;
  sys_state.mqtt_running = false;
  // 创建系统事件队列（最大4个事件缓存）
  xSystemEventQueue = xQueueCreate(8, sizeof(SystemEvent_t));
  SystemEvent_t event = CONFIG_UPDATED_EVENT;
  xQueueSend(xSystemEventQueue, &event, 0);
  // 注册WiFi事件回调函数
  WiFi.onEvent(MyWiFiEvent);
  while (1) {
    SystemEvent_t event;
    if (xQueueReceive(xSystemEventQueue, &event, portMAX_DELAY)) {
      xSemaphoreTake(sys_state.mutex, portMAX_DELAY);
      switch (event) {
      case MSC_WRITE_EVENT: {
        // USB-MSC写入事件（U盘数据更新，此处预留扩展）
        printf("MSC_WRITE_EVENT \n");
        break;
      }
      case CONFIG_UPDATED_EVENT: {
        // 配置更新事件（读取配置文件并重新连接WiFi/MQTT）
        printf("CONFIG_UPDATED_EVENT\n");
        esp32wifi.disConnect();
        String str;
        // 从USB-MSC获取配置字符串（假设msc.getConfigStr从U盘读取配置）
        msc.getConfigStr(FFat, str);
        uint8_t cnt = parse_kv(str.c_str(), keyValue, MAX_ENTRIES);
        // 设置数据上报间隔
        setUpdateInterval();
        char *ssid = get_value_case_insensitive(keyValue, cnt, "WiFi_Name");
        char *passwd = get_value_case_insensitive(keyValue, cnt, "WiFi_Password");
        if (ssid != NULL && passwd != NULL) {
          sys_state.config_wifi = true;
          sys_state.wifi_connected = esp32wifi.connect(ssid, passwd);
          if (sys_state.wifi_connected) {
            SystemEvent_t event = WIFI_CONNECTED_EVENT;
            xQueueSend(xSystemEventQueue, &event, 0);
          }
        } else {
          sys_state.config_wifi = false;
        }
        // 更新IO传感器配置
        // ioSensorHub.configIO(keyValue);
        smartIOManager.configIOhub(keyValue);
        smartIOManager.init();
        // 重置MQTT运行状态
        sys_state.mqtt_running = false;
        break;
      }
      case WIFI_CONNECTED_EVENT: {
        // WiFi连接成功事件
        printf("WIFI_CONNECTED_EVENT\n");
        sys_state.wifi_connected = true;

        break;
      }

      case WIFI_DISCONNECTED_EVENT: {
        // WiFi断开事
        sys_state.wifi_connected = false;
        printf("WIFI_DISCONNECTED_EVENT\n");
        break;
      }
      }
      xSemaphoreGive(sys_state.mutex);
    }
  }
}

/**
 * @brief 显示任务（屏幕刷新与交互处理）
 *
 * @param args 任务参数（未使用）
 * @details 1. 初始化OLED屏幕
 *          2. 更新IO标签、I2C设备信息和屏幕显示
 *          3. 处理按钮事件和屏幕超时休眠
 */
void display_task(void *args) {
  vTaskDelay(10);
  oled.init();
  IOStatusUpdate update;

  while (1) {
    // 从配置键值对更新IO标签显示
    oled.updateHeaderFromKeyValue(keyValue, MAX_ENTRIES);
    // 更新I2C设备数量和地址字符串
    oled.setI2cStr(i2cDeviceManager.getI2cCNT(), i2cDeviceManager.getAddrStr());
    oled.loop(); // 统一由此负责刷新
    vTaskDelay(pdMS_TO_TICKS(45));
  }
}

/**
 * @brief 传感器中枢任务（数据采集与JSON生成）
 *
 * @param arg null
 * @details 1. 初始化I2C总线,定期采集I2C和IO传感器数据
 *          3. 生成JSON格式数据并通过队列传输
 */
void SensorHub_Task(void *arg) {
  vTaskDelay(pdMS_TO_TICKS(2345));
  Wire1.setPins(1, 2);
  Wire1.begin();
  const TickType_t xMinInterval = pdMS_TO_TICKS(20);
  TickType_t xLastWakeTime = xTaskGetTickCount();

  while (1) {
    TickType_t xStartTime = xTaskGetTickCount();
    // 处理I2C设备数据
    i2cDeviceManager.process();
    // 处理IO传感器数据
    // ioSensorHub.process();
    smartIOManager.process();
    String resStr = i2cDeviceManager.JsonStr + smartIOManager.JsonStr;
    if (resStr.startsWith(",")) {
      resStr.remove(0);
    }
    if (resStr.endsWith(",")) {
      resStr.remove(resStr.length() - 1);
    }
    while (resStr.indexOf(",,") != -1) {
      resStr.replace(",,", ",");
    }
    resStr = "{" + resStr + "}";
    UBaseType_t original_priority = uxTaskPriorityGet(NULL);
    // 2. 临时设置为最高优先级
    vTaskPrioritySet(NULL, configMAX_PRIORITIES - 1);
    if (xSemaphoreTake(JsonDataMutex, portMAX_DELAY)) {
      JsonSensorData = resStr;
      xSemaphoreGive(JsonDataMutex);
    }
    // 4. 恢复原始优先级
    vTaskPrioritySet(NULL, original_priority);

    // 计算实际耗时
    TickType_t xExecutionTime = xTaskGetTickCount() - xStartTime;

    // 计算下一周期间隔(执行时间×1.2作为安全余量)
    TickType_t xNextInterval = (xExecutionTime * 6) / 5; // 乘以1.2
    // 确保不小于最小间隔
    if (xNextInterval < xMinInterval) {
      xNextInterval = xMinInterval;
    }
    // 限制上报间隔在有效范围内（0.1-10秒）
    if (UpdateIntervalTime < 0.02f || UpdateIntervalTime > 10.0f) {
      UpdateIntervalTime = 1.0f;
    }

    int16_t IntervalTime = 1000 * UpdateIntervalTime;
    // LOG_DEBUG("IntervalTime:%d\n", IntervalTime);
    // vTaskDelay(pdTICKS_TO_MS(IntervalTime));
    if (IntervalTime > xNextInterval) {
      xNextInterval = (TickType_t)IntervalTime;
    }
    // 精确周期延迟
    vTaskDelayUntil(&xLastWakeTime, xNextInterval);
  }
}

/**
 * @brief MQTT服务器任务（网络通信管理）
 *
 * @param arg 任务参数（未使用）
 * @details 1. 根据WiFi状态启动/关闭MQTT服务
 *          2. 连接成功后更新屏幕显示的IP地址
 */
void MQTT_ServerTask(void *arg) {
  LOG_INFO("MQTT_ServerTask start\n");
  while (1) {

    if (sys_state.mqtt_running == true) {
      // MQTT运行中，检查WiFi连接状态
      if (sys_state.wifi_connected == false) {
        LOG_INFO("[MQTT] server close.\n");
        esp_restart(); // WiFi断开时重启设备
        sys_state.mqtt_running = false;
        vTaskDelay(200);
      } else {
        // LOG_INFO("[MQTT] server running.\n");
        // 更新屏幕显示的IP地址
        String ipStr = WiFi.localIP().toString();
        ipStr = "IP:" + ipStr;
        oled.setIPStr(ipStr);
      }
    } else {
      if (sys_state.wifi_connected == true) {
        // MQTT未运行，检查WiFi连接状态
        LOG_INFO("[MQTT] server create.\n");
        vTaskDelay(20);
        mqtt_server_init();
        sys_state.mqtt_running = true;
        vTaskResume(sensor_handle);
      }
    }
    vTaskDelay(pdTICKS_TO_MS(100));
  }
}

/**
 * @brief MQTT消息处理任务
 *
 * @param pvParameters NULL
 * @details 阻塞式接收MQTT消息队列，调用传感器中枢处理指令
 */
void mqttHandlerTask(void *pvParameters) {
  MQTTMessage msg;
  for (;;) {
    if (xQueueReceive(mqttSubQueue, &msg, portMAX_DELAY)) {
      // ioSensorHub.handleMQTTMessage(msg.message);
      smartIOManager.handleMQTTMessage(msg.message);
    }
  }
}

/**
 * @brief WiFi事件回调函数
 *
 * @param event WiFi事件类型
 */
void MyWiFiEvent(WiFiEvent_t event) {
  switch (event) {
  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED: {
    SystemEvent_t event = WIFI_DISCONNECTED_EVENT;
    xQueueSend(xSystemEventQueue, &event, 0);
    break;
  }

  case ARDUINO_EVENT_WIFI_STA_CONNECTED: {

    break;
  }

  case ARDUINO_EVENT_WIFI_STA_GOT_IP: {
    // 获取IP地址后发送连接成功事件，并更新屏幕显示
    SystemEvent_t event = WIFI_CONNECTED_EVENT;
    xQueueSend(xSystemEventQueue, &event, 0);
    String ipStr = WiFi.localIP().toString();
    ipStr = "IP:" + ipStr;
    oled.setIPStr(ipStr);
    break;
  }

  default:

    break;
  }
}

/**
 * @brief 设置数据上报间隔函数（从配置中读取间隔值）
 *
 * @return bool 配置有效返回true，否则返回false
 */
bool setUpdateInterval() {
  const char *keys[] = {"Sending_Interval(0.02-10s)", "Sending_Interval"};
  const float defaultVal = 1.0f;
  char buffer[32];
  UpdateIntervalTime = defaultVal; // 初始化间隔为默认值
  // 尝试从配置中读取间隔值（不区分大小写）
  for (int i = 0; i < 2; i++) {
    char *res = get_value_case_insensitive(keyValue, MAX_ENTRIES, keys[i]);
    if (res != NULL) {
      memset(buffer, 0, sizeof(buffer));
      strncpy(buffer, res, sizeof(buffer) - 1);

      size_t len = strlen(buffer);
      if (len > 0 && (buffer[len - 1] == 's' || buffer[len - 1] == 'S')) {
        buffer[len - 1] = '\0';
      }

      float val = atof(buffer);
      // 转换为浮点数并验证范围
      if (val >= 0.02f && val <= 10.0f) {
        UpdateIntervalTime = val;
        return true;
      } else {
        printf("defaultVal %.1f\n", keys[i], val, defaultVal);
        return false;
      }
    }
  }
  printf("[Info]defaultVal %.1f\n", defaultVal);
  return false;
}

/**
 * @brief I2C设备扫描任务
 *
 * @param args NULL
 * @details 周期性扫描I2C总线，更新设备列表
 */
void i2c_scan_task(void *args) {
  I2CDeviceManager *manager = static_cast<I2CDeviceManager *>(args);
  Wire1.setPins(1, 2);
  Wire1.begin();
  while (1) {
    manager->searchI2CDevices();
    vTaskDelay(pdMS_TO_TICKS(120)); // 每 3 秒刷新设备列表
  }
}

/**
 * @brief 物理按键轮询任务
 *
 * @param param 未使用
 * @details 检测WAKE_BUTTON_PIN引脚电平变化
 */
void buttonPollTask(void *param) {
  while (1) {
    bool currentState = digitalRead(WAKE_BUTTON_PIN);
    if (lastButtonState == HIGH && currentState == LOW) {
      oled.buttonPressed = true;
    }
    lastButtonState = currentState;
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

/**
 * @brief LED状态指示任务（当前实现）
 *
 * @param args 未使用
 * @details 根据系统状态控制LED显示
 */
void datLedTask(void *args) {
  DAT led(DAT_LED_PIN, 3); // 创建对象（GPIO 9）
  led.begin();

  // 正弦呼吸灯相关变量
  // const float PI = 3.1415926535f; // 定义π值
  float angle = 0.0f;                 // 正弦角度
  const float angleIncrement = 0.05f; // 角度增量（控制速度）

  // 状态颜色配置
  const rgb_t stateColors[] = {
      {255, 0, 0},   // 红色 (config_wifi=false)
      {0, 255, 0},   // 绿色 (mqtt_running=true)
      {255, 255, 0}, // 黄色 (wifi_connected=false)
      {127, 52, 90}  // 粉色 (其他状态)
  };

  const char *keys[] = {"State_LED"};
  char *res = get_value_case_insensitive(keyValue, MAX_ENTRIES, keys[0]);
  if (res != NULL) {
    char lower[16];
    str_to_lower(res, lower, sizeof(lower));
    StateLED = (strcmp(lower, "on") == 0);
  }

  while (1) {
    if (StateLED) {
      // 使用正弦函数计算亮度（范围0.0-1.0）
      float brightness = (sinf(angle) + 1.0f) / 2.0f; // 映射到0-1范围

      // 更新角度（确保在0-2π范围内）
      angle += angleIncrement;
      if (angle > 2 * PI) {
        angle -= 2 * PI;
      }

      // 根据状态选择颜色
      rgb_t color;
      if (!sys_state.config_wifi) {
        color = stateColors[0];
      } else if (sys_state.mqtt_running) {
        color = stateColors[1];
      } else if (!sys_state.wifi_connected) {
        color = stateColors[2];
      } else {
        color = stateColors[3];
      }

      // 应用正弦亮度效果
      led.setColor(static_cast<uint8_t>(color.r * brightness), static_cast<uint8_t>(color.g * brightness),
                   static_cast<uint8_t>(color.b * brightness));
      led.show();

      // 延时（控制呼吸速度）
      vTaskDelay(pdMS_TO_TICKS(20)); // 20ms延时，约50Hz刷新率
    } else {
      // LED关闭状态
      led.off();
      vTaskDelay(pdMS_TO_TICKS(50));
    }
  }
}