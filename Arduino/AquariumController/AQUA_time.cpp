/*
  Project: Aquarium Controller
  Library: Time
  Version: 1.0
  Author: Rastislav Birka
*/

#include <avr/eeprom.h>
#include <Arduino.h>
#include "AQUA_time.h"

/*
  Public Functions
*/

void AQUA_time::init(uint8_t dataPin, uint8_t clockPin, uint8_t dsType, bool useDST, uint8_t timeZone) {
  _dataPin = dataPin;
  _clockPin = clockPin;
  _dsType = dsType;
  _useDST = useDST;
  _timeZone = timeZone;
  pinMode(_clockPin, OUTPUT);
  start();
}

void AQUA_time::setOutput(bool enable) {
  uint8_t value, addr, bit;
  switch(_dsType) {
    case DS_TYPE_3231:
      addr = TIME_ADDR_CONTROL_3231;
      bit = 2;
      break;
    case DS_TYPE_1307:
    default:
      addr = TIME_ADDR_CONTROL_1307;
      bit = 7;
  }
  value = _readRegister(addr);
  if((value & (1 << bit)) != enable) {
    value &= ~(1 << bit);
    value |= (enable << bit);
    _writeRegister(addr, value);
  }
}

void AQUA_time::enableSQW(bool enable) {
  uint8_t value, addr, bit;
  switch(_dsType) {
    case DS_TYPE_3231:
      addr = TIME_ADDR_CONTROL_3231;
      bit = 6;
      break;
    case DS_TYPE_1307:
    default:
      addr = TIME_ADDR_CONTROL_1307;
      bit = 4;
  }
  value = _readRegister(addr);
  if((value & (1 << bit)) != enable) {
    value &= ~(1 << bit);
    value |= (enable << bit);
    _writeRegister(addr, value);
  }
}

void AQUA_time::setSQWRate(int rate) {
  uint8_t value, addr, rs;
  if(rate > 3) {
    rate = 3;
  }
  switch(_dsType) {
    case DS_TYPE_3231:
      addr = TIME_ADDR_CONTROL_3231;
      rs = 24; //00011000
      rate <<= 3;
      break;
    case DS_TYPE_1307:
    default:
      addr = TIME_ADDR_CONTROL_1307;
      rs = 3;
  }
  value = _readRegister(addr);
  value &= ~(rs);
  value |= (rate);
  _writeRegister(addr, value);
}

void AQUA_time::start() {
  uint8_t value, addr;
  switch(_dsType) {
    case DS_TYPE_3231:
      addr = TIME_ADDR_CONTROL_3231;
      break;
    case DS_TYPE_1307:
    default:
      addr = TIME_ADDR_SEC;
  }
  value = _readRegister(addr);
  if((value & 128) != 0) {
    value &= ~(1 << 7);
    value |= (0 << 7);
    _writeRegister(addr, value);
  }
}

void AQUA_time::stop() {
  uint8_t value, addr;
  switch(_dsType) {
    case DS_TYPE_3231:
      addr = TIME_ADDR_CONTROL_3231;
      break;
    case DS_TYPE_1307:
    default:
      addr = TIME_ADDR_SEC;
  }
  value = _readRegister(addr);
  if((value & 128) != 1) {
    value &= ~(1 << 7);
    value |= (1 << 7);
    _writeRegister(addr, value);
  }
}

void AQUA_time::setDate(uint8_t day, uint8_t mon, uint16_t year) {
  if(day > 0 && day < 32 && mon > 0 && mon < 13 && year > 1999 && year < 2100) {
    _writeRegister(TIME_ADDR_YEAR, _encode(year-2000));
    _writeRegister(TIME_ADDR_MON, _encode(mon));
    _writeRegister(TIME_ADDR_DAY, _encode(day));
    _writeRegister(TIME_ADDR_WDAY, _calculateWday(day, mon, year));
  }
}

void AQUA_time::setTime(uint8_t hour, uint8_t min, uint8_t sec) {
  if(hour >= 0 && hour < 24 && min >= 0 && min < 60 && sec >= 0 && sec < 60) {
    if (_isDST && hour > 0) {
      hour--;
    }
    _writeRegister(TIME_ADDR_HOUR, _encode(hour));
    _writeRegister(TIME_ADDR_MIN, _encode(min));
    _writeRegister(TIME_ADDR_SEC, _encode(sec));
  }
}

void AQUA_time::setDST(bool useDST) {
  _useDST = useDST;
}

void AQUA_time::setTimeZone(int timeZone) {
  _timeZone = timeZone;
}

AQUA_datetime AQUA_time::getDateTime() {
  AQUA_datetime datetimeStruct;
  static uint8_t daysInMonths[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

  _readDateTime();
  datetimeStruct.sec  = _decode(_regDateTime[0]);
  datetimeStruct.min  = _decode(_regDateTime[1]);
  datetimeStruct.hour = _decode(_regDateTime[2]);
  datetimeStruct.wday = _regDateTime[3];
  datetimeStruct.day  = _decode(_regDateTime[4]);
  datetimeStruct.mon  = _decode(_regDateTime[5]);
  datetimeStruct.year = _decode(_regDateTime[6]) + 2000;

  _isDST = false;
  if(_useDST) {
    //DST start in last sunday in march about 01:00 GMT and end in last sunday in october about 01:00 GMT
    //sunday = 7th day in week according to ISO 8601
    if(datetimeStruct.mon > 3 && datetimeStruct.mon < 10) {
      _isDST = true;
    } else if(datetimeStruct.mon == 3 && datetimeStruct.day > 24) {
      if(datetimeStruct.wday == 7) {
        if((datetimeStruct.hour - _timeZone) > 0) {
          _isDST = true;
        }
      } else if((31 - datetimeStruct.day + datetimeStruct.wday) <= 7) {
        _isDST = true;
      }
    } else if(datetimeStruct.mon == 10 && datetimeStruct.day < 25) {
      _isDST = true;
    } else if(datetimeStruct.mon == 10) {
      if(datetimeStruct.wday == 7) {
        if((datetimeStruct.hour - _timeZone) < 1) {
          _isDST = true;
        }
      } else if((31 - datetimeStruct.day + datetimeStruct.wday) >= 7) {
        _isDST = true;
      }
    }
    if(_isDST) {
      datetimeStruct.hour++;
    }
    if(datetimeStruct.mon >= 3 && datetimeStruct.mon <= 10 && datetimeStruct.hour >= 24) {
      datetimeStruct.hour-= 24;
      datetimeStruct.day++;
      if(datetimeStruct.day > daysInMonths[datetimeStruct.mon-1]) {
        datetimeStruct.day = 1;
        datetimeStruct.mon++;
      }
      datetimeStruct.wday++;
      if(datetimeStruct.wday > 7) {
        datetimeStruct.wday = 1;
      }
    }
  }

  return datetimeStruct;
}

/*
  Private Functions
*/

uint8_t AQUA_time::_calculateWday(uint8_t day, uint8_t mon, uint16_t year) {
  static uint8_t monValues[12] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  uint8_t wDay = 0;

  if (mon < 3) {
    year--;
  }
  wDay = (year + year/4 - year/100 + year/400 + monValues[mon-1] + day) % 7;
  if (wDay == 0) { //Sunday = 0, Monday = 1, ...
    wDay = 7; //sunday = 7th day in week according to ISO 8601
  }
  return wDay;
}

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
  while(_dataPin == LOW) {
  }
  digitalWrite(_clockPin, LOW);
}

uint8_t AQUA_time::_readByte() {
  uint8_t value = 0;
  uint8_t currentBit;

  pinMode(_dataPin, INPUT);
  for(int i = 0; i < 8; ++i) {
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

  for(int i = 0; i < 8; i++) {
    _regDateTime[i] = _readByte();
    if(i < 7) {
      _sendAck();
    } else {
      _sendNack();
    }
  }
  _sendStop();
}

uint8_t AQUA_time::_decode(uint8_t value) {
  uint8_t decoded = (value & 15) + 10 * ((value & (15 << 4)) >> 4);
  return decoded;
}

uint8_t AQUA_time::_encode(uint8_t value) {
  uint8_t encoded = ((value / 10) << 4) + (value % 10);
  return encoded;
}
