/**
 * @file    FirmwareUpdater.cpp
 * @brief   固件升级管理模块实现
 *
 * @details 实现从文件系统加载固件文件并执行升级的功能，
 *          支持通过FFat文件系统读取固件二进制文件，基于Arduino Update库完成升级流程，
 *          包含错误处理、进度跟踪和自动重启机制。
 */
#include "FirmwareUpdater.h"

// 初始化静态成员：调试输出回调函数指针
void (*FirmwareUpdater::_debugOutput)(const String &) = nullptr;

/**
 * @brief 执行固件升级流程
 *
 * @param updatePath 固件文件路径（默认："/firmware.bin"）
 * @return bool 升级成功返回true，失败返回false
 * @note 流程说明：
 *       1. 检查固件文件是否存在
 *       2. 打开文件并校验文件大小
 *       3. 初始化升级缓冲区并校验空间
 *       4. 逐流写入固件数据
 *       5. 完成升级并触发设备重启
 *       6. 自动删除已使用的固件文件
 */
bool FirmwareUpdater::performUpdate(const char *updatePath)
{
      // 1. 检查固件文件是否存在于文件系统
  if (!FFat.exists(updatePath))
  {
    _log("No update file found");
    return false;
  }
  // 2. 打开固件文件
  File file = FFat.open(updatePath, "r");
  if (!file)
  {
    _log("Failed to open file");
    return false;
  }
  // 3. 获取文件大小并校验
  size_t size = file.size();
  if (size == 0)
  {
    _log("File is empty");
    file.close();
    return false;
  }
  // 4. 初始化OTA升级模块，申请缓冲区空间
  if (!Update.begin(size))
  {
    _log("Insufficient space");
    file.close();
    return false;
  }
  // 5. 逐流写入固件数据（支持大文件分块传输）
  size_t written = Update.writeStream(file);
  if (written != size)
  {
    _log("Write failed");
    file.close();
    return false;
  }
  // 6. 完成升级并校验完整性
  if (!Update.end())
  {
    _log("Update error: " + String(Update.getError()));
    file.close();
    return false;
  }
  // 7. 清理固件文件并重启设备
  file.close();
  if (Update.isFinished())
  {
    FFat.remove(updatePath);
    _log("Update success, restarting...");
    ESP.restart(); // 触发设备重启以加载新固件
  }
  return true;
}

/**
 * @brief 内部日志输出函数（支持用户自定义回调）
 * 
 * @param message 日志信息
 * @details 优先通过用户设置的回调函数输出日志，
 *          若未设置则使用默认的HWSerial输出。
 */
void FirmwareUpdater::_log(const String &message)
{

  HWSerial.println(message); // 默认使用Serial
}
/**
 * @brief 设置调试输出回调函数
 * 
 * @param debugFunc 回调函数指针，参数为调试信息字符串
 * @example 
 *   // 使用Lambda表达式设置回调
 *   FirmwareUpdater::setDebugOutput([](const String& msg){
 *       SerialUSB.println("DEBUG: " + msg);
 *   });
 */
void FirmwareUpdater::setDebugOutput(void (*debugFunc)(const String &))
{
  _debugOutput = debugFunc;
}