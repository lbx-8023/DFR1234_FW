#pragma once

#ifndef MY_GLOBAL_H__
#define MY_GLOBAL_H__

// 系统和Arduino基础库
#include "Arduino.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <vector>
#include <queue>
#include <set>
#include <map>
#include <string>
#include <algorithm>
#include <iostream>

// ESP32特定库
#include "driver/rmt.h"
#include "driver/i2c.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "esp_system.h"
#include "esp_heap_caps.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/message_buffer.h"
#include "sys/stat.h"

// 传感器和外设库


// 加密和JSON处理

#include <ArduinoJson.h>

// 文件系统和存储
#include "ff.h"


// 避免循环包含（虽然这里包含自身没意义，但保留原始结构）

// 常量定义
#define HWSerial Serial0
#define USBSerial Serial

#define LOG_INFO(...)   printf("[INFO] " __VA_ARGS__)
#define LOG_ERROR(...)  printf("[ERROR] " __VA_ARGS__)
#define LOG_DEBUG(...)  printf("[DEBUG] " __VA_ARGS__)

#define SCREEN_TIMEOUT_MS 60000UL

#define IO_PORT_NUM 6

#define WAKE_BUTTON_PIN   0

#define OLED_SCL 48
#define OLED_SDA 47

#define IO1_PORT 4
#define IO2_PORT 5
#define IO3_PORT 6
#define IO4_PORT 7
#define IO5_PORT 15
#define IO6_PORT 16

// #define DAT_LED_PIN 9
#define DAT_LED_PIN 8

const uint8_t pin_map[IO_PORT_NUM] = {IO1_PORT, IO2_PORT, IO3_PORT, IO4_PORT, IO5_PORT, IO6_PORT};


#endif
