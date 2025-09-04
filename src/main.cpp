/**
 * @file    main.cpp
 * @brief   主程序入口
 *
 * @details 芯片信息打印及任务启动，
 */
#include <Arduino.h>
#include "FFat.h"
#include <WiFi.h>
#include <iostream>
#include "msc.h"
#include "app_task.h"
#include <vector>
#include "global.h"
#include <string>
#include <esp_idf_version.h>
#include "SmartI2CManager.h"
#ifndef ARDUINO_USB_MODE
#error This ESP32 SoC has no Native USB interface
#elif ARDUINO_USB_MODE == 1
#warning This sketch should be used when USB is in OTG mode
void setup() {}
void loop() {}
#else

uint32_t chipId = 0;

USB_MSC msc;

void setup()
{

  HWSerial.begin(115200);
  HWSerial.setDebugOutput(false);
  for (int i = 0; i < 17; i = i + 8)
  {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  HWSerial.printf("ESP32 Chip model = %s Rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
  HWSerial.printf("This chip has %d cores\n", ESP.getChipCores());
  HWSerial.print("Chip ID: ");
  HWSerial.println(chipId);
    // 初始化 USB-MSC 功能（使 ESP32 可作为 U 盘被电脑识别）

  msc.setup();
  // 启动应用任务
  start_task();

}
void loop()
{
}

#endif