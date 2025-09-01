/*!
 *@file DFRobot_GestureFaceDetection.h
 *@brief Define the basic structure of class DFRobot_GestureFaceDetection, the implementation of basic methods.
 *@details this module is used to identify the information in the QR code
 *@copyright   Copyright (c) 2025 DFRobot Co.Ltd (http://www.dfrobot.com)
 *@License     The MIT License (MIT)
 *@author [thdyyl](yuanlong.yu@dfrobot.com)
 *@version  V1.0
 *@date  2025-04-07
 *@https://github.com/DFRobot/DFRobot_GestureFaceDetection
*/


 
#ifndef _DFROBOT_GESTUREFACEDETECTION_H
#define _DFROBOT_GESTUREFACEDETECTION_H

#include "Arduino.h"
#include "DFRobot_RTU.h"
#include <Wire.h>
#include <stdint.h>

// Uncomment the following line to enable debugging messages
// #define ENABLE_DBG

#ifdef ENABLE_DBG
#define LDBG(...)  {Serial.print("["); Serial.print(__FUNCTION__); Serial.print("(): "); Serial.print(__LINE__); Serial.print(" ] "); Serial.println(__VA_ARGS__);}
#else
#define LDBG(...)
#endif

// GestureFaceDetection Configuration Register Addresses
#define REG_GFD_ADDR                    0x00    ///< Device address register
#define REG_GFD_BAUDRATE                0x01    ///< Baud rate configuration register
#define REG_GFD_VERIFY_AND_STOP         0x02    ///< Parity and stop bits configuration register
#define REG_GFD_FACE_THRESHOLD          0x03    ///< Face detection threshold, X coordinate
#define REG_GFD_FACE_SCORE_THRESHOLD    0x04    ///< Face score threshold
#define REG_GFD_GESTURE_SCORE_THRESHOLD 0x05    ///< Gesture score threshold

#define GFD_PID     0x0272  ///< Product ID
// Error codes for UART configuration
#define ERR_INVALID_BAUD            0x0001  ///< Invalid baud rate
#define ERR_INVALID_PARITY          0x0002  ///< Invalid parity setting
#define ERR_INVALID_STOPBIT         0x0004  ///< Invalid stop bit
#define ERR_CONFIG_BUAD             0x0010  ///< Baud rate configuration failed.
#define ERR_CONFIG_PARITY_STOPBIT   0x0020  ///< Failed to configure checksum and stop bit.
#define SUCCESS                     0x0000  ///< Operation succeeded

// GestureFaceDetection Data Register Addresses
#define REG_GFD_PID                 0x00    ///< Product ID register
#define REG_GFD_VID                 0x01    ///< Vendor ID register
#define REG_GFD_HW_VERSION          0x02    ///< Hardware version register
#define REG_GFD_SW_VERSION          0x03    ///< Software version register
#define REG_GFD_FACE_NUMBER         0x04    ///< Number of detected faces
#define REG_GFD_FACE_LOCATION_X     0x05    ///< Face X coordinate
#define REG_GFD_FACE_LOCATION_Y     0x06    ///< Face Y coordinate
#define REG_GFD_FACE_SCORE          0x07    ///< Face score
#define REG_GFD_GESTURE_TYPE        0x08    ///< Gesture type
#define REG_GFD_GESTURE_SCORE       0x09    ///< Gesture score

#define INPUT_REG_OFFSET            0x06    ///< Input register offset

/**
 * @brief Enumeration for baud rate configuration.
 */
typedef enum {
    eBaud_1200 = 1,     ///< Baud rate 1200
    eBaud_2400,         ///< Baud rate 2400
    eBaud_4800,         ///< Baud rate 4800
    eBaud_9600,         ///< Baud rate 9600
    eBaud_14400,        ///< Baud rate 14400
    eBaud_19200,        ///< Baud rate 19200
    eBaud_38400,        ///< Baud rate 38400
    eBaud_57600,        ///< Baud rate 57600
    eBaud_115200,       ///< Baud rate 115200
    eBaud_230400,       ///< Baud rate 230400
    eBaud_460800,       ///< Baud rate 460800
    eBaud_921600,       ///< Baud rate 921600
    eBaud_MAX
} eBaudConfig_t;

/**
 * @brief Enumeration for UART parity configuration.
 */
typedef enum {
    UART_CFG_PARITY_NONE = 0,  ///< No parity
    UART_CFG_PARITY_ODD,       ///< Odd parity
    UART_CFG_PARITY_EVEN,      ///< Even parity
    UART_CFG_PARITY_MARK,      ///< Mark parity
    UART_CFG_PARITY_SPACE,     ///< Space parity
    UART_CFG_PARITY_MAX,
} eParityConfig_t;

/**
 * @brief Enumeration for UART stop bits configuration.
 */
typedef enum {
    UART_CFG_STOP_BITS_0_5 = 0, ///< 0.5 stop bits
    UART_CFG_STOP_BITS_1,       ///< 1 stop bit
    UART_CFG_STOP_BITS_1_5,     ///< 1.5 stop bits
    UART_CFG_STOP_BITS_2,       ///< 2 stop bits
    UART_CFG_STOP_MAX,
} eStopbits_t;

/**
 * @brief DFRobot_GestureFaceDetection class provides an interface for interacting with DFRobot GestureFaceDetection devices.
 */
class DFRobot_GestureFaceDetection {
public:
    /**
     * @brief Constructor for DFRobot_GestureFaceDetection.
     */
    DFRobot_GestureFaceDetection();

    /**
     * @brief Init function
     * @return True if initialization is successful, otherwise false.
     */
    bool begin();

    /**
     * @brief Get the PID of the device.
     * @return PID of the device.
     */
    uint16_t getPid();

    /**
     * @brief Get the VID of the device.
     * @return VID of the device.
     */
    uint16_t getVid();

    /**
     * @brief Set the device address.
     * @param addr Device address. 0x01 - 0xF6
     * @return True if the address is set successfully, otherwise false.
     */
    bool setDeviceAddr(uint16_t addr);

    /**
     * @brief Configure UART settings.
     * 
     * This method is used to set the UART communication parameters for the device, including baud rate, parity, and stop bits. 
     * Users can choose the appropriate parameters based on their needs to ensure stable and effective communication with the device.
     * !!!However, the current CSK6 chip's serial port only supports changing the baud rate, and the stop and check bits should be set to default.
     *
     * @param baud Baud rate configuration, of type `eBaudConfig_t`, with possible values including:
     *              - `eBaud_1200`  - 1200 baud
     *              - `eBaud_2400`  - 2400 baud
     *              - `eBaud_4800`  - 4800 baud
     *              - `eBaud_9600`  - 9600 baud  (Default)
     *              - `eBaud_14400` - 14400 baud
     *              - `eBaud_19200` - 19200 baud
     *              - `eBaud_38400` - 38400 baud
     *              - `eBaud_57600` - 57600 baud
     *              - `eBaud_115200`- 115200 baud
     *              - `eBaud_230400`- 230400 baud
     *              - `eBaud_460800`- 460800 baud
     *              - `eBaud_921600`- 921600 baud
     *
     * @param parity Parity configuration, of type `eParityConfig_t`, with possible values including:
     *              - `UART_CFG_PARITY_NONE`  - No parity (Default)
     *              - `UART_CFG_PARITY_ODD`   - Odd parity
     *              - `UART_CFG_PARITY_EVEN`  - Even parity
     *              - `UART_CFG_PARITY_MARK`  - Mark parity
     *              - `UART_CFG_PARITY_SPACE` - Space parity
     *
     * @param stopBit Stop bits configuration, of type `eStopbits_t`, with possible values including:
     *                - `UART_CFG_STOP_BITS_0_5` - 0.5 stop bits
     *                - `UART_CFG_STOP_BITS_1`   - 1 stop bit  (Default)
     *                - `UART_CFG_STOP_BITS_1_5` - 1.5 stop bits
     *                - `UART_CFG_STOP_BITS_2`   - 2 stop bits
     *
     * @return Status of the configuration, returning the status code if the configuration is successful; otherwise, it returns an error code.
     * @retval SUCCESS                     0x0000  ///< Operation succeeded
     * @retval ERR_INVALID_BAUD            0x0001  ///< Invalid baud rate (bit 0)
     * @retval ERR_INVALID_PARITY          0x0002  ///< Invalid parity setting (bit 1)
     * @retval ERR_INVALID_STOPBIT         0x0004  ///< Invalid stop bit (bit 2)
     * @retval ERR_CONFIG_BAUD             0x0010  ///< Hardware baud rate configuration failed (bit 4)
     * @retval ERR_CONFIG_PARITY_STOPBIT   0x0020  ///< Failed to configure parity/stop bits (bit 5)
     * 
     * @note Errors can be combined via bitwise OR (e.g., 0x0003 = invalid baud + parity).
     *       Check individual bits with `if (error & ERR_INVALID_BAUD)`.
     */
    uint16_t configUart(eBaudConfig_t baud, eParityConfig_t parity, eStopbits_t stopBit);

    /**
     * @brief Set face detection threshold.
     * 
     * Sets the threshold for face detection (0-100). Default is 60%.
     *
     * @param score Threshold value.
     * @return True if successful, otherwise false.
     */
    bool setFaceDetectThres(uint16_t score);
    
    /**
     * @brief Get face detection threshold.
     *
     * Gets the threshold for face detection (0-100). Default is 60%.
     * 
     * @return uint16_t The threshold value.
     * @note The threshold value is a percentage (0-100).
     */
    uint16_t getFaceDetectThres();

    /**
     * @brief Set detection threshold for X coordinate.
     * 
     * Sets the threshold for detecting the X coordinate (0-100). Default is 60%.
     *
     * @param x Threshold value.
     * @return True if successful, otherwise false.
     */
    bool setDetectThres(uint16_t x);

    /**
     * @brief Get the Detect Thres object
     * 
     * Gets the threshold for detecting the X coordinate (0-100). Default is 60%.
     * 
     * @return uint16_t 
     */
    uint16_t getDetectThres();
    
    /**
     * @brief Set gesture detection threshold.
     * 
     * Sets the threshold for gesture detection (0-100). Default is 60%.
     *
     * @param score Threshold value.
     * @return True if successful, otherwise false.
     */
    bool setGestureDetectThres(uint16_t score);

    /**
     * @brief Get the Gesture Detect Thres object
     * 
     * Gets the threshold for gesture detection (0-100). Default is 60%.
     * 
     * @return uint16_t 
     */
    uint16_t getGestureDetectThres();
    /**
     * @brief Get the number of faces detected by the device.
     * @return Number of faces detected.
     */
    uint16_t getFaceNumber();

    /**
     * @brief Get the X coordinate of the detected face.
     * @return X coordinate of the face.
     */
    uint16_t getFaceLocationX();

    /**
     * @brief Get the Y coordinate of the detected face.
     * @return Y coordinate of the face.
     */
    uint16_t getFaceLocationY();

    /**
     * @brief Get the score of the detected face.
     * @return Score of the face.
     * @note The score is a value between 0 and 100, indicating the confidence of the face detection.
     *       Higher values indicate a higher confidence in the detection.
     */
     uint16_t getFaceScore();

    /**
     * @brief Get the type of detected gesture.
     * 
     * This method retrieves the currently detected gesture type. The gesture recognition feature can be used in various applications, such as human-machine interaction or control systems. 
     * The returned gesture type corresponds to the following values:
     * - 1: LIKE (👍) - blue
     * - 2: OK (👌) - green
     * - 3: STOP (🤚) - red
     * - 4: YES (✌️) - yellow
     * - 5: SIX (🤙) - purple
     * 
     * If no gesture is detected, the return value may be a specific invalid value (e.g., 0).
     *
     * @return The detected gesture type, returning the type identifier for the gesture.
     */
    uint16_t getGestureType();

    /**
     * @brief Get the score of the detected gesture.
     * @return Score of the gesture.
     * @note The score is a value between 0 and 100, indicating the confidence of the gesture detection.
     */
    uint16_t getGestureScore();


private:
    /**
     * @brief Read input register.
     * @param reg Register address.
     * @return Value of the input register.
     */
    virtual uint16_t reaInputdReg(uint16_t reg) = 0;

    /**
     * @brief Read holding register.
     * @param reg Register address.
     * @return Value of the holding register.
     */
    virtual uint16_t readHoldingReg(uint16_t reg) = 0;

    /**
     * @brief Write to the holding register.
     * @param reg Register address.
     * @param data Data to write.
     * @return True if the write is successful, otherwise false.
     */
    virtual bool writeIHoldingReg(uint16_t reg, uint16_t data) = 0;

protected:
    uint8_t _addr; ///< Device address
};

/**
 * @brief DFRobot_GestureFaceDetection_UART class provides UART specific implementation for DFRobot GestureFaceDetection devices.
 */
class DFRobot_GestureFaceDetection_UART : public DFRobot_GestureFaceDetection, public DFRobot_RTU {
public:
    /**
     * @brief Constructor for DFRobot_GestureFaceDetection_UART.
     * @param s_ Pointer to the Stream object.
     * @param addr Device address.
     */
    DFRobot_GestureFaceDetection_UART(Stream *s_, uint8_t addr);
    bool begin();
    uint16_t reaInputdReg(uint16_t reg);
    uint16_t readHoldingReg(uint16_t reg);
    bool writeIHoldingReg(uint16_t reg, uint16_t data);
};

/**
 * @brief DFRobot_GestureFaceDetection_I2C class provides I2C specific implementation for DFRobot GestureFaceDetection devices.
 */
class DFRobot_GestureFaceDetection_I2C : public DFRobot_GestureFaceDetection {
public: 
    /**
     * @brief Constructor for DFRobot_GestureFaceDetection_I2C.
     * @param addr Device address.
     */
    DFRobot_GestureFaceDetection_I2C(uint8_t addr);

    /**
     * @brief Initialize the I2C communication.
     * @param pWire Pointer to the TwoWire object, default is &Wire.
     * @return True if initialization is successful, otherwise false.
     */
    bool begin(TwoWire *pWire = &Wire);

    /**
     * @fn calculate_crc
     * @brief Calculate CRC-8 checksum using polynomial 0x07
     * @param data Pointer to input data buffer
     * @param length Length of data in bytes
     * @return 8-bit CRC checksum
     * @note Implements CRC-8 algorithm with:
     *       - Polynomial: x^8 + x^2 + x^1 + 1 (0x07 in hex)
     *       - Initial value: 0xFF
     *       - No output XOR
     */
    uint8_t calculate_crc(const uint8_t *data, size_t length);
    bool writeIHoldingReg(uint16_t reg, uint16_t data);
    uint16_t reaInputdReg(uint16_t reg);
    uint16_t readHoldingReg(uint16_t reg);
    bool writeReg(uint16_t reg, uint16_t data);
    uint16_t readReg(uint16_t reg);

private:
    TwoWire *_pWire; ///< I2C communication object
};

#endif
