#include "IOHub.h"
#include "global.h"

void NanIOHub::init() {}

void NanIOHub::callback(const char *arg) {}

void AnalogIOHub::init() {}

void AnalogIOHub::callback(const char *arg) {
  char tempStr[32] = {0};
  float val = analogRead(this->_pin) / 4096.0f;
  this->_JsonStr =
      "\"p" + String(this->_ioIdx) + "_input_val\":" + String(val, 2) + "\n";
}

void DigitalOutIOHub::init() { pinMode(this->_pin, OUTPUT); }

void DigitalOutIOHub::callback(const char *arg) {
  if (arg == nullptr) {
    return;
  }
  uint8_t outLevel = LOW;

  // 忽略大小写
  char lowerVal[16];
  strncpy(lowerVal, arg, sizeof(lowerVal) - 1);
  lowerVal[sizeof(lowerVal) - 1] = '\0';
  for (char *ch = lowerVal; *ch != '\0'; ch++) {
    *ch = tolower(*ch);
  }
  // 判断控制命令
  if (strcmp(lowerVal, "on") == 0) {
    outLevel = HIGH;
  } else if (strcmp(lowerVal, "off") == 0) {
    outLevel = LOW;
  } else {
    float val = atof(lowerVal);
    if (val == 1.0f) {
      outLevel = HIGH;
    }
  }
  digitalWrite(this->_pin, outLevel);
}

void DHT11IOHub::init() {
  this->_dht = new DHT(this->_pin, DHT11);
  this->_dht->begin();
}

void DHT11IOHub::callback(const char *arg) {
  static int errcount = 0;
  float humidity = this->_dht->readHumidity();
  float temperature = this->_dht->readTemperature();
  this->_JsonStr = "\"errcount\":" + String(errcount) + ",";
  if (isnan(humidity) || isnan(temperature)) {
    errcount++;
    this->_JsonStr +=  "\"p" + String(this->_ioIdx) +
                     "_dht11_humi\":\"NAN\",\"p" + String(this->_ioIdx) +
                     "_dht11_temp\":\"NAN\"\n";
  } else {
    this->_JsonStr += "\"p" + String(this->_ioIdx) +
                     "_dht11_humi\":" + String(humidity, 2) + ",\"p" +
                     String(this->_ioIdx) +
                     "_dht11_temp\":" + String(temperature, 2) + "\n";
  }
}

void IDFDHT11Hub::init() {
  DHT11_init((gpio_num_t)this->_pin);
}

void IDFDHT11Hub::callback(const char *arg) {
  static int errcount = 0;
  float humidity = DHT11_read().humidity;
  float temperature = DHT11_read().temperature;
  this->_JsonStr = "\"errcount\":" + String(errcount) + ",";
  if (isnan(humidity) || isnan(temperature)) {
    errcount++;
    this->_JsonStr +=  "\"p" + String(this->_ioIdx) +
                     "_dht11_humi\":\"NAN\",\"p" + String(this->_ioIdx) +
                     "_dht11_temp\":\"NAN\"\n";
  } else {
    this->_JsonStr += "\"p" + String(this->_ioIdx) +
                     "_dht11_humi\":" + String(humidity, 2) + ",\"p" + 
                     String(this->_ioIdx) +
                     "_dht11_temp\":" + String(temperature, 2) + "\n";
  }
}

void DHT11EspIOHub::init() {
  this->_dht = new DHTesp();
  this->_dht->setup(this->_pin, DHTesp::DHT11);
}

void DHT11EspIOHub::callback(const char *arg) {
  static int errcount = 0;

  // 1. 保存当前优先级
  UBaseType_t original_priority = uxTaskPriorityGet(NULL);
  // 2. 临时设置为最高优先级
  vTaskPrioritySet(NULL, configMAX_PRIORITIES - 1);
  TempAndHumidity tempAndHumidity = this->_dht->getTempAndHumidity();
  // 4. 恢复原始优先级
  vTaskPrioritySet(NULL, original_priority);

  float humidity = tempAndHumidity.humidity;
  float temperature = tempAndHumidity.temperature;
//   this->_JsonStr = "\"errcount\":" + String(errcount) + ",";
  if (isnan(humidity) || isnan(temperature)) {
    // LOG_ERROR("DHT11 ERROR!\n");
    if(!this->_validData || xTaskGetTickCount() - this->_xLastNanTime >= pdMS_TO_TICKS(4000)){
        // errcount++;
        this->_JsonStr =  "\"p" + String(this->_ioIdx) +
                     "_dht11_humi\":\"NAN\",\"p" + String(this->_ioIdx) +
                     "_dht11_temp\":\"NAN\"\n";
    }else{
        this->_JsonStr = "\"p" + String(this->_ioIdx) +
                     "_dht11_humi\":" + String(this->_humidity, 2) + ",\"p" +
                     String(this->_ioIdx) +
                     "_dht11_temp\":" + String(this->_temperature, 2) + "\n";
    }

  } else {
    this->_validData = true;
    this->_xLastNanTime = xTaskGetTickCount();
    this->_humidity = humidity;
    this->_temperature = temperature;
    this->_JsonStr = "\"p" + String(this->_ioIdx) +
                     "_dht11_humi\":" + String(humidity, 2) + ",\"p" +
                     String(this->_ioIdx) +
                     "_dht11_temp\":" + String(temperature, 2) + "\n";
  }
}


void DFRobotDHT11IOHub::init() {
//   this->_dht = new DFRobot_DHT11();
}


void DFRobotDHT11IOHub::callback(const char *arg) {
    // int ret = this->_dht->read(this->_pin);
    // if(ret != 0){
    //     LOG_ERROR("DFRobot_DHT11 ........\n");
    //     return;
    // }
    // float humidity = this->_dht->humidity;
    // float temperature = this->_dht->temperature;
    float humidity = NAN, temperature = NAN;
    static int errcount = 0;
    this->_JsonStr = "\"errcount\":" + String(errcount) + ",";
    if(dht_read_float_data(DHT_TYPE_DHT11, (gpio_num_t)this->_pin, &humidity, &temperature)== ESP_OK){
         this->_JsonStr += "\"p" + String(this->_ioIdx) +
                    "_dht11_humi\":" + String(humidity, 2) + ",\"p" +
                    String(this->_ioIdx) +
                    "_dht11_temp\":" + String(temperature, 2) + "\n";
    }
    else{
        errcount++;

        this->_JsonStr +=  "\"p" + String(this->_ioIdx) +
                     "_dht11_humi\":\"NAN\",\"p" + String(this->_ioIdx) +
                     "_dht11_temp\":\"NAN\"\n";

    }   
    printf("%s\n", this->_JsonStr.c_str());
}


void DS18B20IOHub::init() {
  this->_oneWire = new OneWire(this->_pin);
//   this->_ds18b20 = new DallasTemperature(this->_oneWire);
//   this->_ds18b20->begin();
}
float DS18B20IOHub::getMyTemp(){
    byte data[12];
    byte addr[8];

    if ( !this->_oneWire->search(addr)) {
        //no more sensors on chain, reset search
        this->_oneWire->reset_search();
        return -127;
    }

    if ( OneWire::crc8( addr, 7) != addr[7]) {
        Serial.println("CRC is not valid!");
        return -127;
    }

    if ( addr[0] != 0x10 && addr[0] != 0x28) {
        Serial.print("Device is not recognized");
        return -127;
    }

    this->_oneWire->reset();
    this->_oneWire->select(addr);
    this->_oneWire->write(0x44,1); // start conversion, with parasite power on at the end

    byte present = this->_oneWire->reset();
    this->_oneWire->select(addr);
    this->_oneWire->write(0xBE); // Read Scratchpad


    for (int i = 0; i < 9; i++) { // we need 9 bytes
        data[i] = this->_oneWire->read();
    }

    this->_oneWire->reset_search();

    byte MSB = data[1];
    byte LSB = data[0];

    float tempRead = ((MSB << 8) | LSB); //using two's compliment
    float temperature = tempRead / 16;
    return temperature;
    
}
void DS18B20IOHub::callback(const char *arg) {
//   this->_ds18b20->begin();
//   this->_ds18b20->requestTemperatures();
//   float temperature = this->_ds18b20->getTempCByIndex(0);
//   if (temperature == -127) {
//     this->_JsonStr = "\"p" + String(this->_ioIdx) + "_ds18b20_temp\":\"NAN\"\n";
//   } else {
//     this->_JsonStr = "\"p" + String(this->_ioIdx) +
//                      "_ds18b20_temp\":" + String(temperature, 2) + "\n";
//   }
    /**
     * @brief 更改为这样的原因是使用DallasTemperature库会影响呼吸灯
     */
    float temperature = this->getMyTemp();
    if (temperature == -127) {
        this->_JsonStr = "\"p" + String(this->_ioIdx) + "_ds18b20_temp\":\"NAN\"\n";
    } else {
        this->_JsonStr = "\"p" + String(this->_ioIdx) +
                        "_ds18b20_temp\":" + String(temperature, 2) + "\n";
    }
}

void Servo180IOHub::init() { this->_servo = new Servo(); }

void Servo180IOHub::callback(const char *arg) {
  if (arg == nullptr) {
    return;
  }
  int angle = atoi(arg);
  // 限制角度范围为 0 到 180 度
  angle = constrain(angle, 0, 180);
  if (!this->_servo->attached()) {
    this->_servo->attach(this->_pin);
  }
  this->_servo->write(angle);
}

void Servo360IOHub::init() { this->_servo = new Servo(); }

void Servo360IOHub::callback(const char *arg) {
  if (arg == nullptr) {
    return;
  }
  int speed = atoi(arg);
  printf("SPEED: %d\n", speed);
  // 解析速度值并限制在有效范围内
  speed = constrain(speed, -100, 100);
  printf("con SPEED: %d\n", speed);

  // 舵机中点（停止）微调值（某些舵机可能不是刚好1500us）
  const int center_us = 1500;
  const int min_us = 1000; // 全速反转
  const int max_us = 2000; // 全速正转
  const int dead_zone = 2; // 死区范围 ±5（避免小范围抖动）

  if (!this->_servo->attached()) {
    this->_servo->setPeriodHertz(50);                 // 设置频率为50Hz
    this->_servo->attach(this->_pin, min_us, max_us); // 设置范围
  }
  int pulse_us;

  if (abs(speed) <= dead_zone) {
    pulse_us = center_us; // 停止
  } else if (speed > 0) {
    // 正转映射：center_us ~ max_us
    pulse_us = map(speed, dead_zone + 1, 100, center_us + 10, max_us); // 正转
  } else {
    // 反转映射：center_us ~ min_us
    pulse_us = map(-speed, dead_zone + 1, 100, center_us - 10, min_us);
  }
  this->_servo->writeMicroseconds(pulse_us);
}

void WS2812IOHub::init() {
  this->_led = new LiteLED(LED_TYPE, LED_TYPE_IS_RGBW);
  this->_led->begin(this->_pin, MAX_PIXELS);
  this->_led->brightness(160);
}

void WS2812IOHub::callback(const char *arg) {
  if (arg == nullptr) {
    return;
  }
  char buf[512];
  strncpy(buf, arg, sizeof(buf) - 1);
  buf[sizeof(buf) - 1] = '\0'; // 确保字符串以空字符结尾

  uint8_t values[MAX_CHANNELS];
  int idx = 0;
  char *token = strtok(buf, " "); // 使用逗号分隔字符串
  while (token != NULL && idx < MAX_CHANNELS) {
    values[idx++] = atoi(token); // 将字符串转换为整数
    token = strtok(NULL, " ");   // 继续分割字符串
  }

  int pixelCount = idx / 3; // 每个像素有3个值（RGB）
  if (pixelCount > MAX_PIXELS) {
    pixelCount = MAX_PIXELS; // 限制像素数量
  }

  for (int i = 0; i < pixelCount; i++) {
    uint8_t r = values[i * 3];     // R
    uint8_t g = values[i * 3 + 1]; // G
    uint8_t b = values[i * 3 + 2]; // B
    crgb_t color = (r << 16) | (g << 8) | b; // 组合颜色值
    this->_led->setPixel(i, color, false); // 设置像素颜色不自动 show
  }

  this->_led->show(); // 一次性更新所有像素

  this->_JsonStr = "OK " + String(pixelCount) + " LEDs on pin " + String(this->_pin) + "\n";
}

void Servo300IOHub::init() { this->_servo = new Servo(); }

void Servo300IOHub::callback(const char *arg) {
  if (arg == nullptr) {
    return;
  }
  
  int angle = atoi(arg);
  // 限制角度范围为 0 到 300 度
  angle = constrain(angle, 0, 300);
  printf("ANGLE: %d\n", angle);
  // 进行角度的转化，实际测试中发现发送的角度和实际转动角度有误差
  angle = (float)angle / 3.0 * 2;
  if (!this->_servo->attached()) {
    this->_servo->attach(this->_pin);
  }
  this->_servo->write(angle);
}
