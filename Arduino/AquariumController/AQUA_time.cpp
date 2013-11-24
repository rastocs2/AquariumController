/*
  Project: Aquarium Controller
  Library: Time
  Version: 1.0
  Author: Rastislav Birka
*/

#include "Arduino.h"
#include "AQUA_time.h"

/*
  Public Functions
*/

void AQUA_time::init(uint8_t dataPin, uint8_t clockPin) {
  _dataPin = dataPin;
  _clockPin = clockPin;
  pinMode(_clockPin, OUTPUT);
  start();
}

void AQUA_time::setOutput(bool enable) {
  uint8_t value = _readRegister(TIME_ADDR_CONTROL);
  if((value & 128) != enable) {
    value &= ~(1 << 7);
    value |= (enable << 7);
    _writeRegister(TIME_ADDR_CONTROL, value);
  }
}

void AQUA_time::enableSQW(bool enable) {
  uint8_t value = _readRegister(TIME_ADDR_CONTROL);
  if((value & 16) != enable) {
    value &= ~(1 << 4);
    value |= (enable << 4);
    _writeRegister(TIME_ADDR_CONTROL, value);
  }
}

void AQUA_time::setSQWRate(int rate) {
  uint8_t value = _readRegister(TIME_ADDR_CONTROL);
  value &= ~(3);
  value |= (rate);
  _writeRegister(TIME_ADDR_CONTROL, value);
}

void AQUA_time::start() {
  uint8_t value = _readRegister(TIME_ADDR_SEC);
  if((value & 128) != 0) {
    value &= ~(1 << 7);
    value |= (0 << 7);
    _writeRegister(TIME_ADDR_SEC, value);
  }
}

void AQUA_time::stop() {
  uint8_t value = _readRegister(TIME_ADDR_SEC);
  if((value & 128) != 1) {
    value &= ~(1 << 7);
    value |= (1 << 7);
    _writeRegister(TIME_ADDR_SEC, value);
  }
}

void AQUA_time::setDate(uint8_t day, uint8_t mon, uint16_t year) {
  if (day > 0 && day < 32 && mon > 0 && mon < 13 && year > 1999 && year < 3000) {
    year -= 2000;
    _writeRegister(TIME_ADDR_YEAR, _encode(year));
    _writeRegister(TIME_ADDR_MON, _encode(mon));
    _writeRegister(TIME_ADDR_DAY, _encode(day));
  }
}

void AQUA_time::setTime(uint8_t hour, uint8_t min, uint8_t sec) {
  if (hour >= 0 && hour < 24 && min >= 0 && min < 60 && sec >= 0 && sec < 60) {
    _writeRegister(TIME_ADDR_HOUR, _encode(hour));
    _writeRegister(TIME_ADDR_MIN, _encode(min));
    _writeRegister(TIME_ADDR_SEC, _encode(sec));
  }
}

void AQUA_time::setWday(uint8_t wday) {
  if (wday > 0 && wday < 8) {
    _writeRegister(TIME_ADDR_WDAY, wday);
  }
}

AQUA_datetime AQUA_time::getDateTime() {
  AQUA_datetime datetimeStruct;

  _readDateTime();
  datetimeStruct.sec  = _decode(_regDateTime[0]);
  datetimeStruct.min  = _decode(_regDateTime[1]);
  datetimeStruct.hour = _decode(_regDateTime[2]);
  datetimeStruct.wday = _regDateTime[3];
  datetimeStruct.day  = _decode(_regDateTime[4]);
  datetimeStruct.mon  = _decode(_regDateTime[5]);
  datetimeStruct.year = _decode(_regDateTime[6]) + 2000;

  return datetimeStruct;
}

/*
  Private Functions
*/

void AQUA_time::_sendStart(uint8_t addr) {
  pinMode(_dataPin, OUTPUT);
  digitalWrite(_dataPin, HIGH);
  digitalWrite(_clockPin, HIGH);
  digitalWrite(_dataPin, LOW);
  digitalWrite(_clockPin, LOW);
  shiftOut(_dataPin, _clockPin, MSBFIRST, addr);
}

void AQUA_time::_sendStop() {
  pinMode(_dataPin, OUTPUT);
  digitalWrite(_dataPin, LOW);
  digitalWrite(_clockPin, HIGH);
  digitalWrite(_dataPin, HIGH);
  pinMode(_dataPin, INPUT);
}

void AQUA_time::_sendAck() {
  pinMode(_dataPin, OUTPUT);
  digitalWrite(_clockPin, LOW);
  digitalWrite(_dataPin, LOW);
  digitalWrite(_clockPin, HIGH);
  digitalWrite(_clockPin, LOW);
  pinMode(_dataPin, INPUT);
}

void AQUA_time::_sendNack() {
  pinMode(_dataPin, OUTPUT);
  digitalWrite(_clockPin, LOW);
  digitalWrite(_dataPin, HIGH);
  digitalWrite(_clockPin, HIGH);
  digitalWrite(_clockPin, LOW);
  pinMode(_dataPin, INPUT);
}

void AQUA_time::_waitForAck() {
  pinMode(_dataPin, INPUT);
  digitalWrite(_clockPin, HIGH);
  while (_dataPin == LOW) {
  }
  digitalWrite(_clockPin, LOW);
}

uint8_t AQUA_time::_readByte() {
  uint8_t value = 0;
  uint8_t currentBit;

  pinMode(_dataPin, INPUT);
  for (int i = 0; i < 8; ++i) {
    digitalWrite(_clockPin, HIGH);
    currentBit = digitalRead(_dataPin);
    value |= (currentBit << 7-i);
    delayMicroseconds(10);
    digitalWrite(_clockPin, LOW);
  }
  return value;
}

void AQUA_time::_writeByte(uint8_t value) {
  pinMode(_dataPin, OUTPUT);
  shiftOut(_dataPin, _clockPin, MSBFIRST, value);
}

uint8_t AQUA_time::_readRegister(uint8_t reg) {
  uint8_t value;

  _sendStart(TIME_ADDR_WRITE);
  _waitForAck();
  _writeByte(reg);
  _waitForAck();
  _sendStop();
  _sendStart(TIME_ADDR_READ);
  _waitForAck();
  value = _readByte();
  _sendNack();
  _sendStop();

  return value;
}

void AQUA_time::_writeRegister(uint8_t reg, uint8_t value) {
  _sendStart(TIME_ADDR_WRITE);
  _waitForAck();
  _writeByte(reg);
  _waitForAck();
  _writeByte(value);
  _waitForAck();
  _sendStop();
}

void AQUA_time::_readDateTime() {
  _sendStart(TIME_ADDR_WRITE);
  _waitForAck();
  _writeByte(0);
  _waitForAck();
  _sendStop();
  _sendStart(TIME_ADDR_READ);
  _waitForAck();

  for (int i = 0; i < 8; i++) {
    _regDateTime[i] = _readByte();
    if (i < 7) {
      _sendAck();
    } else {
      _sendNack();
    }
  }
  _sendStop();
}

uint8_t	AQUA_time::_decode(uint8_t value) {
  uint8_t decoded = (value & 15) + 10 * ((value & (15 << 4)) >> 4);
  return decoded;
}

uint8_t AQUA_time::_encode(uint8_t value) {
  uint8_t encoded = ((value / 10) << 4) + (value % 10);
  return encoded;
}
