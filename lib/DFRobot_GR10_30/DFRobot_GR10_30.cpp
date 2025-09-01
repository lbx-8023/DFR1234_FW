/*!
 * @file DFRobot_GR10_30.cpp
 * @brief This is the method implementation file of GR10_30.
 * @copyright   Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author [zhixinliu](zhixinliu@dfrobot.com)
 * @version  V0.1
 * @date  2022-07-24
 * @url https://github.com/DFRobor/DFRobot_GR10_30
 */

#include "DFRobot_GR10_30.h"


DFRobot_GR10_30::DFRobot_GR10_30(uint8_t addr, TwoWire *pWire)
{
  _pWire = pWire;
  _addr = addr;
};

DFRobot_GR10_30::DFRobot_GR10_30(uint8_t addr, Stream *s):DFRobot_RTU(s)
{
  _s = s;
  _addr = addr;
}

int8_t DFRobot_GR10_30::begin(void)
{
  delay(500);
  setTimeoutTimeMs(200);
  if(_addr > 0xF7){
    DBG("Invaild Device addr.");
  }
  if(_addr != 0){
    if(!detectDeviceAddress(_addr)){
      DBG("Device addr Error.");
      return -1;
    }
  }else{
    return -1;
  }
  return 0;
}

bool DFRobot_GR10_30::detectDeviceAddress(uint8_t addr)
{
  if(_pWire){
    // _pWire->begin();
    _pWire->beginTransmission(addr);
    if(_pWire->endTransmission() != 0) {
      DBG("i2c connect error");
      delay(50);
      return false;
    }
    return true;
  }else{
    uint16_t ret = readInputRegister(addr, GR10_30_INPUTREG_ADDR);
    DBG(ret);
    if((ret & 0xFF) == addr){
      return true;
    }
  }
  return false;
}

void DFRobot_GR10_30::resetSensor(void)
{
  uint8_t _sendData[2] = {0};
  _sendData[0] = 0x55;
  _sendData[1] = 0x00;
  writeReg(R_RESET, _sendData, 2);
  delay(100);
}

void DFRobot_GR10_30::enGestures(uint16_t gestures)
{
  uint8_t _sendData[2] = {0};
  _sendData[0] = (gestures>>8)&0xC7;
  _sendData[1] = gestures;
  writeReg(R_INTERRUPT_MODE, _sendData, 2);
  delay(50);
}

void DFRobot_GR10_30::setUdlrWin(uint8_t udSize, uint8_t lrSize)
{
  uint8_t _sendData[2] = {0};
  _sendData[0] = lrSize&0x1f;
  _sendData[1] = udSize&0x1f;
  writeReg(R_LRUP_WIN, _sendData, 2);
  delay(50);
}

void DFRobot_GR10_30::setLeftRange(uint8_t range)
{
  uint8_t _sendData[2] = {0};
  _sendData[0] = 0;
  _sendData[1] = range&0x1f;
  writeReg(R_L_RANGE, _sendData, 2);
  delay(50);
}

void DFRobot_GR10_30::setRightRange(uint8_t range)
{
  uint8_t _sendData[2] = {0};
  _sendData[0] = 0;
  _sendData[1] = range&0x1f;
  writeReg(R_R_RANGE, _sendData, 2);
  delay(50);
}

void DFRobot_GR10_30::setUpRange(uint8_t range)
{
  uint8_t _sendData[2] = {0};
  _sendData[0] = 0;
  _sendData[1] = range&0x1f;
  writeReg(R_U_RANGE, _sendData, 2);
  delay(50);
}

void DFRobot_GR10_30::setDownRange(uint8_t range)
{
  uint8_t _sendData[2] = {0};
  _sendData[0] = 0;
  _sendData[1] = range&0x1f;
  writeReg(R_D_RANGE, _sendData, 2);
  delay(50);
}

void DFRobot_GR10_30::setForwardRange(uint8_t range)
{
  uint8_t _sendData[2] = {0};
  _sendData[0] = 0;
  _sendData[1] = range&0x1f;
  writeReg(R_FORWARD_RANGE, _sendData, 2);
  delay(50);
}

void DFRobot_GR10_30::setBackwardRange(uint8_t range)
{
  uint8_t _sendData[2] = {0};
  _sendData[0] = 0;
  _sendData[1] = range&0x1f;
  writeReg(R_BACKUP_RANGE, _sendData, 2);
  delay(50);
}

void DFRobot_GR10_30::setWaveNumber(uint8_t number)
{
  uint8_t _sendData[2] = {0};
  _sendData[0] = 0;
  _sendData[1] = number&0x0f;
  writeReg(R_WAVE_COUNT, _sendData, 2);
  delay(50);
}

void DFRobot_GR10_30::setHovrWin(uint8_t udSize, uint8_t lrSize)
{
  uint8_t _sendData[2] = {0};
  _sendData[0] = lrSize&0x1f;
  _sendData[1] = udSize&0x1f;
  writeReg(R_HOVR_WIN, _sendData, 2);
  delay(50);
}

void DFRobot_GR10_30::setHovrTimer(uint16_t timer)
{
  uint8_t _sendData[2] = {0};
  timer &= 0x03ff;        // 10位有效
  _sendData[0] = (timer>>8)&0x03;
  _sendData[1] = timer;
  writeReg(R_HOVR_TIMER, _sendData, 2);
  delay(50);
}

void DFRobot_GR10_30::setCwsAngle(uint8_t count)
{
  uint8_t _sendData[2] = {0};
  count &= 0x1F;
  _sendData[0] = 0;
  _sendData[1] = count;
  writeReg(R_CWS_ANGLE, _sendData, 2);
  delay(50);
}

void DFRobot_GR10_30::setCcwAngle(uint8_t count)
{
  uint8_t _sendData[2] = {0};
  count &= 0x1F;
  _sendData[0] = 0;
  _sendData[1] = count;
  writeReg(R_CCW_ANGLE, _sendData, 2);
  delay(50);
}

void DFRobot_GR10_30::setCwsAngleCount(uint8_t count)
{
  uint8_t _sendData[2] = {0};
  count &= 0x1F;
  _sendData[0] = 0;
  _sendData[1] = count;
  writeReg(R_CWS_ANGLE_COUNT, _sendData, 2);
  delay(50);
}

void DFRobot_GR10_30::setCcwAngleCount(uint8_t count)
{
  uint8_t _sendData[2] = {0};
  count &= 0x1F;
  _sendData[0] = 0;
  _sendData[1] = count;
  writeReg(R_CCW_ANGLE_COUNT, _sendData, 2);
  delay(50);
}

uint16_t DFRobot_GR10_30::getExist(void)
{
  uint8_t _sendData[10] = {0};
  if(0xff == readReg(R_WAVE_COUNT, _sendData, 2)){
    return 0;
  }
  return ((uint16_t)_sendData[0]<<8)+ _sendData[1];
}

uint16_t DFRobot_GR10_30::getDataReady(void)
{
  uint8_t _sendData[10] = {0};
  if(0xff == readReg(R_DATA_READY, _sendData, 2)){
    return 0;
  }
  //DBG(((uint16_t)_sendData[0]<<8)+ _sendData[1]);
  return ((uint16_t)_sendData[0]<<8)+ _sendData[1];
}

uint16_t DFRobot_GR10_30::getGesturesState(void)
{
  uint8_t _sendData[10] = {0};
  if(0xff == readReg(R_INTERRUPT_STATE, _sendData, 2)){
    return 0;
  }
  //DBG(((uint16_t)_sendData[0]<<8)+ _sendData[1]);
  return ((uint16_t)_sendData[0]<<8)+ _sendData[1];
}

void DFRobot_GR10_30::setDefaultConfig(void)
{
  setUdlrWin(30, 30);
  setLeftRange(10);
  setRightRange(10);
  setUpRange(10);
  setDownRange(10);
  setForwardRange(10);
  setBackwardRange(10);
  setWaveNumber(2);
  setHovrWin(20,20);
  setHovrTimer(0x3C);
  setCwsAngle(16);
  setCcwAngle(16);
  setCwsAngleCount(8);
  setCcwAngleCount(8);
}

uint8_t DFRobot_GR10_30::readReg(uint16_t reg, void *pBuf, uint8_t size)
{
  uint8_t* _pBuf = (uint8_t*)pBuf;
  if(pBuf == NULL){
    DBG("data error");
    return 0;
  }

  if(_pWire){
    _pWire->beginTransmission(_addr);
    _pWire->write(reg);
    //_pWire->endTransmission();
    if(_pWire->endTransmission() != 0){
      return 0xFF;
    }
    _pWire->requestFrom(_addr, size);
    for(uint8_t i = 0; i < size; i++)
      _pBuf[i] = _pWire->read();
    return size;
  }else{
    return readInputRegister(_addr, reg, _pBuf, size);
  }
}
uint8_t DFRobot_GR10_30::writeReg(uint8_t reg, void *pBuf, size_t size)
{
  uint8_t *_pBuf = (uint8_t*)pBuf;
  uint8_t ret = 0;
  if(_pWire){
    _pWire->beginTransmission(_addr);
    _pWire->write(reg);
    for(size_t i = 0; i < size; i++){
      _pWire->write(_pBuf[i]);
    }
    _pWire->endTransmission();
  }else{
    ret = writeHoldingRegister(_addr, reg, _pBuf, size);
  }
  return ret;
}
