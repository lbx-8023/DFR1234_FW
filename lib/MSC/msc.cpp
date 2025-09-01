#include "msc.h"
#include "global.h"
// #include "FirmwareUpdater.h"

// FirmwareUpdater firmwareUpdater; // 固件升级实例

// 存储物理驱动号
uint8_t s_pdrv = 0;
// 存储磁盘块大小
int s_disk_block_size = 0;
// 系统事件队列句柄
extern QueueHandle_t xSystemEventQueue;
/**
 * @brief MSC设备写操作处理函数
 * @param lba 逻辑块地址
 * @param offset 块内偏移量
 * @param buffer 数据缓冲区
 * @param bufsize 数据大小（字节）
 * @return 实际写入字节数
 * @note 按磁盘块大小分割数据并调用底层驱动写入，完成后发送系统事件
 */

 /**
 * @brief MSC设备写操作处理函数
 * @param lba 逻辑块地址
 * @param offset 块内偏移量
 * @param buffer 数据缓冲区
 * @param bufsize 数据大小（字节）
 * @return 实际写入字节数
 * @note 按磁盘块大小分割数据并调用底层驱动写入，完成后发送系统事件
 */
int32_t USB_MSC::onWrite(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize)
{
    // 计算需要写入的磁盘块数量
  const uint32_t block_count = bufsize / s_disk_block_size;
  disk_write(s_pdrv, (BYTE *)buffer, lba, block_count);
  HWSerial.printf("MSC onWrite\n");
  SystemEvent_t event = MSC_WRITE_EVENT;
  xQueueSend(xSystemEventQueue, &event, 0);
  return bufsize;
}

/**
 * @brief MSC设备读操作处理函数
 * @param lba 逻辑块地址
 * @param offset 块内偏移量
 * @param buffer 数据缓冲区
 * @param bufsize 数据大小（字节）
 * @return 实际读取字节数
 * @note 按磁盘块大小分割数据并调用底层驱动读取，完成后打印调试信息
 */
int32_t USB_MSC::onRead(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
{

  const uint32_t block_count = bufsize / s_disk_block_size;
  disk_read(s_pdrv, (BYTE *)buffer, lba, block_count);
  HWSerial.printf("MSC onRead\n");
  return bufsize;
}

/**
 * @brief MSC设备启动/停止事件处理函数
 * @param power_condition 电源状态（具体含义取决于硬件）
 * @param start 是否启动（true=启动，false=停止）
 * @param load_eject 是否加载/弹出介质（true=加载，false=弹出）
 */
bool USB_MSC::onStartStop(uint8_t power_condition, bool start, bool load_eject)
{
  HWSerial.printf("MSC START/STOP: power: %u, start: %u, eject: %u\n", power_condition, start, load_eject);
  return true;
}

/**
 * @brief USB事件回调函数
 * @param arg 回调参数（未使用）
 * @param event_base 事件基类型（此处为Arduino USB事件）
 * @param event_id 事件ID
 * @param event_data 事件数据指针
 * @note 处理USB连接状态变化
 */
void USB_MSC::usbEventCallback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  if (event_base == ARDUINO_USB_EVENTS)
  {
    arduino_usb_event_data_t *data = (arduino_usb_event_data_t *)event_data;
    switch (event_id)
    {
    case ARDUINO_USB_STARTED_EVENT:
      HWSerial.println("USB PLUGGED");
      break;
    case ARDUINO_USB_STOPPED_EVENT:
      HWSerial.println("USB UNPLUGGED");
      break;
    case ARDUINO_USB_SUSPEND_EVENT:
      HWSerial.printf("USB SUSPENDED: remote_wakeup_en: %u\n", data->suspend.remote_wakeup_en);
      break;
    case ARDUINO_USB_RESUME_EVENT:
      HWSerial.println("USB RESUMED");
      break;

    default:
      break;
    }
  }
}

/**
 * @brief USB_MSC类初始化函数
 * @note 完成文件系统挂载、固件升级、磁盘信息获取、CRC表生成、配置文件管理及USB MSC设备初始化
 */
void USB_MSC::setup()
{
//   FFat.format();
  if (!FFat.begin())
  {
    HWSerial.println("FFat mount failed!");
    FFat.format();
    FFat.begin();
  }
//   firmwareUpdater.performUpdate("/firmware.bin");

  uint32_t block_count;
  uint16_t block_size;
  disk_ioctl(s_pdrv, GET_SECTOR_COUNT, &block_count);
  disk_ioctl(s_pdrv, GET_SECTOR_SIZE, &block_size);
  s_disk_block_size = block_size;

  config_crc32 = 0x00;
  generate_crc32_table(crc32_table);
  deleteConfigVariants();

  if (!config_file_exists())
  {

    // 构建配置信息字符串
    String config =
        "WiFi_Name:ssid\n"
        "WiFi_Password:passwd\n"
        "Sending_Interval(0.02-10s): 1s\n"
        "P1: output\n"
        "P2: output\n"
        "P3: output\n"
        "P4: output\n"
        "P5: servo180\n"
        "P6: servo180\n"
        "State_LED: on\n";
    const char *configMessage = config.c_str();
    writeFile(FFat, "/config.txt", configMessage);
  }

  USB.onEvent(USB_MSC::usbEventCallback);
  MSC.vendorID("DFROBOT");    // max 8 chars
  MSC.productID("DFR1234");   // max 16 chars
  MSC.productRevision("1.0"); // max 4 chars
  MSC.onStartStop(USB_MSC::onStartStop);
  MSC.onRead(USB_MSC::onRead);
  MSC.onWrite(USB_MSC::onWrite);
  MSC.mediaPresent(true);
  MSC.begin(block_count, block_size);

  USB.begin();
}

void USB_MSC::listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{

  HWSerial.printf("Listing directory: %s\r\n", dirname);

  File root = fs.open(dirname);
  if (!root)
  {
    LOG_INFO("- failed to open directory\n");
    return;
  }
  if (!root.isDirectory())
  {
    LOG_INFO(" - not a directory\n");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      HWSerial.print("  DIR : ");
      HWSerial.println(file.name());
      if (levels)
      {
        listDir(fs, file.path(), levels - 1);
      }
    }
    else
    {
      HWSerial.print("  FILE: ");
      HWSerial.print(file.name());
      HWSerial.print("\tSIZE: ");
      HWSerial.println(file.size());
    }
    file = root.openNextFile();
  }
}

String USB_MSC::readFile(fs::FS &fs, const char *path)
{
  String fileContent = "";
  // HWSerial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if (!file || file.isDirectory())
  {
    HWSerial.println("- failed to open file for reading");
    return fileContent; // 返回空字符串
  }

  static const size_t bufferSize = 64;
  uint8_t buffer[bufferSize]; // 修改为 uint8_t 类型
  size_t bytesRead;
  while ((bytesRead = file.read(buffer, bufferSize)) > 0)
  {
    // HWSerial.write((char*)buffer, bytesRead); // 转换为 char* 类型以便打印
    fileContent += String((const char *)buffer, bytesRead); // 转换为字符串
  }
  file.close();
  return fileContent; // 返回文件内容字符串
}

void USB_MSC::writeFile(fs::FS &fs, const char *path, const char *message)
{
  HWSerial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    HWSerial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    HWSerial.println("- file written");
  }
  else
  {
    HWSerial.println("- write failed");
  }
  file.close();
}

bool USB_MSC::isConfigChange()
{
  if (config_file_exists())
  {
    String str;
    generate_crc32_table(crc32_table);
    getConfigStr(FFat, str);
    uint32_t tempCRC32 = calculate_string_crc32(str, crc32_table);
    printf("CRC32:0x%04x\n", tempCRC32);
    if (tempCRC32 != config_crc32)
    {
      config_crc32 = tempCRC32;
      return true;
    }

    else
    {
      config_crc32 = tempCRC32;
      return false;
    }
  }
  else
    return false;
}
void USB_MSC::appendFile(fs::FS &fs, const char *path, const char *message)
{
  HWSerial.printf("Appending to file: %s\r\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file)
  {
    HWSerial.println("- failed to open file for appending");
    return;
  }
  if (file.print(message))
  {
    HWSerial.println("- message appended");
  }
  else
  {
    HWSerial.println("- append failed");
  }
  file.close();
}

void USB_MSC::renameFile(fs::FS &fs, const char *path1, const char *path2)
{
  HWSerial.printf("Renaming file %s to %s\r\n", path1, path2);
  if (fs.rename(path1, path2))
  {
    HWSerial.println("- file renamed");
  }
  else
  {
    HWSerial.println("- rename failed");
  }
}

void USB_MSC::deleteFile(fs::FS &fs, const char *path)
{
  HWSerial.printf("Deleting file: %s\r\n", path);
  if (fs.remove(path))
  {
    HWSerial.println("- file deleted");
  }
  else
  {
    HWSerial.println("- delete failed");
  }
}

bool USB_MSC::getConfigStr(fs::FS &fs, String &str)
{

  String fileContent = "";

  File file = fs.open("/config.txt");
  if (!file || file.isDirectory())
  {
    HWSerial.println("- failed to open file for reading");
    return false; // 返回空字符串
  }

  static const size_t bufferSize = 64;
  uint8_t buffer[bufferSize]; // 修改为 uint8_t 类型
  size_t bytesRead;
  while ((bytesRead = file.read(buffer, bufferSize)) > 0)
  {
    fileContent += String((const char *)buffer, bytesRead); // 转换为字符串
  }
  str = fileContent;
  file.close();
  return true; // 返回文件内容字符串
}
bool USB_MSC::config_file_exists()
{
  File file = FFat.open("/config.txt");
  if (!file || file.isDirectory())
  {
    HWSerial.println("- failed to open file for reading");
    return false;
  }
  file.close();
  return true;
}

// 生成查找表
void USB_MSC::generate_crc32_table(uint32_t crc32_table[256])
{
  uint32_t crc;
  for (int i = 0; i < 256; i++)
  {
    crc = i;
    for (int j = 0; j < 8; j++)
    {
      if (crc & 1)
      {
        crc = (crc >> 1) ^ CRC32_POLYNOMIAL;
      }
      else
      {
        crc >>= 1;
      }
    }
    crc32_table[i] = crc;
  }
}
void USB_MSC::deleteConfigVariants()
{
  HWSerial.println("Deleting config.txt variants...");
  
  File root = FFat.open("/");
  if (!root)
  {
    HWSerial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    HWSerial.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      HWSerial.print("  DIR : ");
      HWSerial.println(file.name());
    }
    else
    {
      String filename = String(file.name());
      HWSerial.printf("file: %s\n", filename.c_str());
      LOG_INFO("filename: %s\n", file.name());
      LOG_INFO("filename.startsWith: %d\n", filename.startsWith("CONFIG.TXT"));

      if (filename.endsWith(".SB-")){
        if (FFat.remove("/" + filename)){
            HWSerial.printf("Deleted: %s\n", filename.c_str());
        }else{
            HWSerial.printf("Failed to delete: %s\n", filename.c_str());
        }
      }
    //   if (filename.startsWith("config.txt"))
    //   {
    //     if (!filename.equals("config.txt"))
    //     {
    //       if (FFat.remove("/" + filename))
    //       {
    //         HWSerial.printf("Deleted: %s\n", filename.c_str());
    //       }
    //       else
    //       {
    //         HWSerial.printf("Failed to delete: %s\n", filename.c_str());
    //       }
    //     }
    //   }
    }
    file = root.openNextFile();
  }
}

// 计算CRC32值
uint32_t USB_MSC::crc32(const uint8_t *data, size_t length, uint32_t crc32_table[256])
{
  uint32_t crc = 0xFFFFFFFF;
  for (size_t i = 0; i < length; i++)
  {
    crc = (crc >> 8) ^ crc32_table[(crc ^ data[i]) & 0xFF];
  }
  return crc ^ 0xFFFFFFFF;
}

uint32_t USB_MSC::calculate_string_crc32(String str, uint32_t crc32_table[256])
{
  const char *c_str = str.c_str(); // 获取C风格字符串
  size_t length = str.length();    // 获取字符串长度
  return crc32((const uint8_t *)c_str, length, crc32_table);
}