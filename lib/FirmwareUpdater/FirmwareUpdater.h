/**
 * @file    FirmwareUpdater.h
 * @brief   固件升级管理模块头文件
 *
 * @details 提供从文件系统（如FFat）加载固件并执行升级的功能，
 */
#pragma once
// #ifndef FIRMWARE_UPDATER_H
// #define FIRMWARE_UPDATER_H

 #include "global.h"

#include "FFat.h"
#include <Update.h>
#include <FS.h>
/**
 * @brief 固件升级管理类
 *
 * @details 封装固件升级流程，支持从指定路径读取固件二进制文件，
 *          校验文件有效性后执行OTA升级，并提供调试日志功能。
 */
class FirmwareUpdater
{
public:
  /**
   * @brief 执行固件升级
   *
   * @param updatePath 固件文件路径（默认："/firmware.bin"）
   * @return bool 升级成功返回true，失败返回false
   * @note 升级过程中请勿断电，升级完成后设备将自动重启。
   */
  static bool performUpdate(const char *updatePath = "/firmware.bin");

  /**
   * @brief 设置调试输出回调函数
   *
   * @param debugFunc 回调函数指针，参数为调试信息字符串
   * @example setDebugOutput([](const String& msg){ Serial.println(msg); });
   */
  static void setDebugOutput(void (*debugFunc)(const String &));

private:
  /**
   * @brief 内部日志输出函数
   *
   * @param message 日志信息
   * @details 通过_debugOutput回调函数输出调试信息，若无回调则静默处理。
   */
  static void _log(const String &message);
  /** @brief 调试输出回调函数指针（用户自定义） */
  static void (*_debugOutput)(const String &);
};

// #endif