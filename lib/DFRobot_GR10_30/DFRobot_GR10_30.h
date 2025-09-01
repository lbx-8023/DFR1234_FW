/*!
 * @file DFRobot_GR10_30.h
 * @brief This file demostrates the method for using GR10_30
 * @copyright   Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author [zhixinliu](zhixinliu@dfrobot.com)
 * @version  V0.1
 * @date  2022-07-24
 * @url https://github.com/DFRobor/DFRobot_GR10_30
 */
#ifndef DFROBOT_GR10_30_H
#define DFROBOT_GR10_30_H

#include "Arduino.h"
#include "Wire.h"
#include "DFRobot_RTU.h"
#include "String.h"
#if (defined ARDUINO_AVR_UNO) && (defined ESP8266)
#include "SoftwareSerial.h"
#else
#include "HardwareSerial.h"
#endif

//#define ENABLE_DBG ///< Enable this macro to see the detailed running process of the program
#ifdef ENABLE_DBG
#define DBG(...) {Serial.print("[");Serial.print(__FUNCTION__); Serial.print("(): "); Serial.print(__LINE__); Serial.print(" ] "); Serial.println(__VA_ARGS__);}
#else
#define DBG(...)
#endif

class DFRobot_GR10_30:public DFRobot_RTU{
public:
  #define GR10_30_DEVICE_ADDR          0x73
  //Input Register
  #define GR10_30_INPUTREG_PID         0x00   ///< Device PID
  #define GR10_30_INPUTREG_VID         0x01   ///< VID of the device, fixed to be 0x3343
  #define GR10_30_INPUTREG_ADDR        0x02   ///< Device address of the module
  #define GR10_30_INPUTREG_BAUDRATE    0x03   ///< Serial baud rate
  #define GR10_30_INPUTREG_STOPBIT     0x04   ///< Serial check bit and stop bit
  #define GR10_30_INPUTREG_VERSION     0x05   ///< Firmware version information
  #define R_DATA_READY                 0X06   ///< Data ready register
  #define R_INTERRUPT_STATE            0X07   ///< Register for interrupt status
  #define R_EXIST_STATE                0X08   ///< Register for object presence 

  //Holding register
  #define R_INTERRUPT_MODE        0X09    ///< The gesture that can trigger interrupt
  #define R_LRUP_WIN              0X0A    ///< The detection window you want
  #define R_L_RANGE               0X0B    ///< Distance of moving to left
  #define R_R_RANGE               0X0C    ///< Distance of moving to right
  #define R_U_RANGE               0X0D    ///< Distance of moving up
  #define R_D_RANGE               0X0E    ///< Distance of moving down 
  #define R_FORWARD_RANGE         0X0F    ///< Distance of moving forward
  #define R_BACKUP_RANGE          0X10    ///< Distance of moving backward
  #define R_WAVE_COUNT            0X11    ///< Number of waves
  #define R_HOVR_WIN              0X12    ///< Hover detection window
  #define R_HOVR_TIMER            0X13    ///< Hovering time
  #define R_CWS_ANGLE             0X14    ///< Clockwise rotation angle, each value represents 22.5 degrees
  #define R_CCW_ANGLE             0X15    ///< Counterclockwise rotation angle, each value represents 22.5 degrees
  #define R_CWS_ANGLE_COUNT       0X16    ///< Continuous clockwise rotation angle, each value represents 22.5 degrees
  #define R_CCW_ANGLE_COUNT       0X17    ///< Continuous counterclockwise rotation angle, each value represents 22.5 degrees
  #define R_RESET                 0X18    ///< Reset sensor
  

  #define GESTURE_UP                      (1<<0)
  #define GESTURE_DOWN                    (1<<1)
  #define GESTURE_LEFT                    (1<<2)
  #define GESTURE_RIGHT                   (1<<3)
  #define GESTURE_FORWARD                 (1<<4)
  #define GESTURE_BACKWARD                (1<<5)
  #define GESTURE_CLOCKWISE               (1<<6)
  #define GESTURE_COUNTERCLOCKWISE        (1<<7)
  #define GESTURE_WAVE                    (1<<8)
  #define GESTURE_HOVER                   (1<<9)
  #define GESTURE_UNKNOWN                 (1<<10)
  #define GESTURE_CLOCKWISE_C             (1<<14)   ///< Rotate clockwise continuously
  #define GESTURE_COUNTERCLOCKWISE_C      (1<<15)   ///< Rotate counterclockwise continuously

  /**
   * @fn DFRobot_GR10_30
   * @brief DFRobot_GR10_30 constructor
   * @param pWire I2C pointer to the TowWire stream, which requires calling begin in the demo to init Arduino I2C config.
   * @param addr  I2C communication address of SEN0543 device
   */
  DFRobot_GR10_30(uint8_t addr, TwoWire *pWire = &Wire);

  /**
   * @fn DFRobot_GR10_30
   * @brief DFRobot_GR10_30 constructor
   * @param addr: The device address of the communication between the host computer and SEN0543 slave device
   * @n     SEN0543_DEVICE_ADDR or 115(0X73): Default address of SEN0543 device, if users do not change the device address, it's default to 115.
   * @param s   : The serial port pointer to the Stream, which requires calling begin in the demo to init communication serial port config of Arduino main controller, in line with that of SEN0543 device slave.
   * @n SEN0543 serial port config: 9600 baud rate, 8-bit data bit, no check bit, 1 stop bit, the parameters can't be changed.
   */
  DFRobot_GR10_30(uint8_t addr, Stream *s);
  ~DFRobot_GR10_30(){};

  /**
   * @fn begin
   * @brief Init SEN0543 device
   * @return Return value init status
   * @retval 0  Succeed
   * @retval -1 Failed
   */
  int8_t begin(void);

  /**
   * @fn enGestures
   * @brief Set what gestures the module can recognize to trigger interrupt
   * @param gestures
   *  GESTURE_UP
   *  GESTURE_DOWN
   *  GESTURE_LEFT
   *  GESTURE_RIGHT
   *  GESTURE_FORWARD
   *  GESTURE_BACKWARD
   *  GESTURE_CLOCKWISE
   *  GESTURE_COUNTERCLOCKWISE
   *  GESTURE_WAVE              It is not suggested to enable rotation gesture (CW/CCW) and wave gesture at the same time.
   *  GESTURE_HOVER             Disable other gestures when hover gesture enables.
   *  GESTURE_UNKNOWN
   *  GESTURE_CLOCKWISE_C           // Rotate clockwise continuously
   *  GESTURE_COUNTERCLOCKWISE_C    // Rotate counterclockwise continuously
   * @return NONE
   */
  void enGestures(uint16_t gestures);

  /**
   * @fn setUdlrWin
   * @brief Set the detection window you want
   * @param udSize Distance from top to bottom      distance range 1-30
   * @param lrSize Distance from left to right      distance range 1-30
   * @return NONE
   */
  void setUdlrWin(uint8_t udSize, uint8_t lrSize);

  /**
   * @fn setLeftRange
   * @brief Set how far your hand should move to the left so the sensor can recognize it
   * @param range
   * @n     Distance range 5-25, must be less than distance from left to right of the detection window
   * @return NONE
   */
  void setLeftRange(uint8_t range);

  /**
   * @fn setRightRange
   * @brief Set how far your hand should move to the right so the sensor can recognize it
   * @param range
   * @n     Distance range 5-25, must be less than distance from left to right of the detection window
   * @return NONE
   */
  void setRightRange(uint8_t range);

  /**
   * @fn setUpRange
   * @brief Set how far your hand should move up so the sensor can recognize it
   * @param range
   * @n     Distance range 5-25, must be less than distance from top to bottom of the detection window
   * @return NONE
   */
  void setUpRange(uint8_t range);

  /**
   * @fn setDownRange
   * @brief Set how far your hand should move down so the sensor can recognize it
   * @param range
   * @n     Distance range 5-25, must be less than distance from top to bottom of the detection window
   * @return NONE
   */
  void setDownRange(uint8_t range);

  /**
   * @fn setForwardRange
   * @brief Set how far your hand should move forward so the sensor can recognize it
   * @param range
   * @n     Distance range 1-15
   * @return NONE
   */
  void setForwardRange(uint8_t range);

  /**
   * @fn setBackwardRange
   * @brief Set how far your hand should move backward so the sensor can recognize it 
   * @param range
   * @n     Distance range 1-15
   * @return NONE
   */
  void setBackwardRange(uint8_t range);

  /**
   * @fn setWaveNumber
   * @brief Set how many times you need to wave hands so the sensor can recognize it
   * @param number
   * @n     Number range 1-15
   * @return NONE
   */
  void setWaveNumber(uint8_t number);

  /**
   * @fn setHovrWin
   * @brief Set hover detection window
   * @param udSize Distance from top to bottom      distance range 1-30
   * @param lrSize Distance from left to right      distance range 1-30
   * @return NONE
   */
  void setHovrWin(uint8_t udSize, uint8_t lrSize);

  /**
   * @fn setHovrTimer
   * @brief Set how long your hand should hover to trigger the gesture
   * @param timer
   * @n     timer 1-200  10ms-2s  默认为 60 600ms
   * @return NONE
   */
  void setHovrTimer(uint16_t timer);

  /**
   * @fn setCwsAngle
   * @brief Set how many degrees your hand should rotate clockwise to trigger the gesture
   * @param count Default 16, range 1-31
   * @n     count Rotation angle = 22.5 * count
   * @n     For example: count = 16, 22.5*count = 360  Rotate 360° to trigger the gesture
   * @return NONE
   */
  void setCwsAngle(uint8_t count);

  /**
   * @fn setCcwAngle
   * @brief Set how many degrees your hand should rotate counterclockwise to trigger the gesture
   * @param count Default 16, range 1-31
   * @n     count Rotation angle = 22.5 * count
   * @n     For example: count = 16, 22.5*count = 360  Rotate 360° to trigger the gesture
   * @return NONE
   */
  void setCcwAngle(uint8_t count);

  /**
   * @fn setCwsAngleCount
   * @brief Set how many degrees your hand should rotate clockwise continuously to trigger the gesture
   * @param count Default 4, range 1-31
   * @n     count Continuous rotation angle = 22.5 * count
   * @n     For example: count = 4, 22.5*count = 90
   * @n     Trigger the clockwise/counterclockwise rotation gesture first, 
   * @n     if keep rotating, then the continuous rotation gesture will be triggered once every 90 degrees
   * @return NONE
   */
  void setCwsAngleCount(uint8_t count);

  /**
   * @fn setCcwAngleCount
   * @brief Set how many degrees your hand should rotate counterclockwise continuously to trigger the gesture
   * @param count Default 4, range 1-31
   * @n     count Continuous rotation angle = 22.5 * count
   * @n     For example: count = 4, 22.5*count = 90
   * @n     Trigger the clockwise/counterclockwise rotation gesture first, 
   * @n     if keep rotating, then the continuous rotation gesture will be triggered once every 90 degrees
   * @return NONE
   */
  void setCcwAngleCount(uint8_t count);

  /**
   * @fn getExist
   * @brief Get whether the object is in the sensor detection range
   * @return If the object is in the sensor detection range
   * @retval 1  Yes
   * @retval 0  No
   */
  uint16_t getExist(void);

  /**
   * @fn getDataReady
   * @brief Get if a gesture is detected
   * @return If a gesture is detected
   * @retval 1  Detected
   * @retval 0  Not detected
   */
  uint16_t getDataReady(void);

  /**
   * @fn getGesturesState
   * @brief Get gesture type
   * @return Gesture type
   * @retval GESTURE_UP
   * @retval GESTURE_DOWN
   * @retval GESTURE_DOWN
   * @retval GESTURE_LEFT
   * @retval GESTURE_RIGHT
   * @retval GESTURE_FORWARD
   * @retval GESTURE_BACKWARD
   * @retval GESTURE_CLOCKWISE
   * @retval GESTURE_COUNTERCLOCKWISE
   * @retval GESTURE_WAVE
   * @retval GESTURE_HOVER
   * @retval GESTURE_UNKNOWN
   * @retval GESTURE_CLOCKWISE_C
   * @retval GESTURE_COUNTERCLOCKWISE_C
   */
  uint16_t getGesturesState(void);

protected:
  bool detectDeviceAddress(uint8_t addr);
  void setDefaultConfig(void);
  void resetSensor(void);
  uint8_t readReg(uint16_t reg, void *pBuf, uint8_t size);
  uint8_t writeReg(uint8_t reg, void *pBuf, size_t size);
  TwoWire   *_pWire = NULL;
  Stream    *_s = NULL;
  uint8_t   _addr;
};


#endif
