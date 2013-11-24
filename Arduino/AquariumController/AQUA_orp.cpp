/*
  Project: Aquarium Controller
  Library: ORP
  Version: 1.0
  Author: Rastislav Birka
*/

#include <avr/eeprom.h>
#include "Arduino.h"
#include "AQUA_orp.h"

/*
  Public Functions
*/

void AQUA_orp::init(uint8_t dqPin, uint8_t calibrate_points, uint8_t calibrate_address) {
  uint16_t value, position;
  uint8_t i;
  bool negative;

  _dqPin = dqPin;
  _pointCount = calibrate_points;
  _calibrateAddress = calibrate_address;
  _calData = new AQUA_orpCalibrationPoint[_pointCount];
  _usedData = new AQUA_orpCalibrationPoint[_pointCount];
  _const = new float[(_pointCount - 1)*2];

  position = 0;
  for(i = 0; i < _pointCount; i++) {
    eeprom_busy_wait();
    value = eeprom_read_word((const uint16_t *)(_calibrateAddress + position));
    if(value == 0xFFFF) { //address wasn't set yet
      _calData[i].state = 0;
      _calData[i].refValue = 0;
      _calData[i].actValue = 0;
      position+= 4;
    } else {
      _calData[i].state = (bool)(value & 32768);
      if(_calData[i].state == 1) {
         value-= 32768;
      }
      negative = (bool)(value & 16384);
      if(negative == 1) {
         value = 0 - (value - 16384);
      }
      _calData[i].refValue = value;
      position+= 2;
      eeprom_busy_wait();
      value = eeprom_read_word((const uint16_t *)(_calibrateAddress + position));
      negative = (bool)(value & 16384);
      if(negative == 1) {
         value = 0 - (value - 16384);
      }
      _calData[i].actValue = value;
      position+= 2;
    }
  }
  _setCalibrationValues();
  pinMode(_dqPin, INPUT);
}

//dokoncit
int AQUA_orp::getORP(bool calibrate) {
  int res = 0;

//  res = random(-400,401);
  if(calibrate == 0) {
    if(_usedPoints == 1) {
      res+= _const[0];
    } else if(_usedPoints > 1) {
      if(res >= _usedData[_usedPoints-1].actValue) {
        res = _const[(_usedPoints-2)*2]*res + _const[(_usedPoints-2)*2 + 1];
      } else {
        for(uint8_t i = 1; i < _usedPoints; i++) {
          if(res <= _usedData[i].actValue) {
            res = _const[(i-1)*2]*res + _const[(i-1)*2 + 1];
            break;
          }
        }
      }
    }
  }

  return res;
}

bool AQUA_orp::calibration(uint8_t point, AQUA_orpCalibrationPoint *values) {
  bool res = false;

  if (point < _pointCount && point >= 0 && values->refValue <= 999 && values->refValue >= -999 && values->actValue <= 999 && values->actValue >= -999) {
    if(values->state != _calData[point].state || values->refValue != _calData[point].refValue || values->actValue != _calData[point].actValue) {
      _calData[point].state = values->state;
      _calData[point].refValue = values->refValue;
      _calData[point].actValue = values->actValue;
      uint16_t plusValue = 0;
      if(_calData[point].state == 1) {
        plusValue+= 32768;
      }
      if(_calData[point].refValue < 0) {
        plusValue+= 16384;
      }
      uint16_t position = point*4;
      eeprom_busy_wait();
      eeprom_write_word((uint16_t *)(_calibrateAddress + position), abs(_calData[point].refValue) + plusValue);
      plusValue = 0;
      if(_calData[point].actValue < 0) {
        plusValue+= 16384;
      }
      position+= 2;
      eeprom_busy_wait();
      eeprom_write_word((uint16_t *)(_calibrateAddress + position), abs(_calData[point].actValue) + plusValue);
      _setCalibrationValues();
      res = true;
    }
  }
  return res;
}

AQUA_orpCalibrationPoint AQUA_orp::readCalibrationPoint(uint8_t point) {
  AQUA_orpCalibrationPoint calPoint;
  if(point < _pointCount && point >= 0) {
    calPoint.state = _calData[point].state;
    calPoint.refValue = _calData[point].refValue;
    calPoint.actValue = _calData[point].actValue;
  } else {
    calPoint.state = 0;
    calPoint.refValue = 0;
    calPoint.actValue = 0;
  }
  return calPoint;
}

/*
  Private Functions
*/

/*
c1 = (ref2 - ref1)/(act2 - act1)
c2 = ref2 - c1*act2
orp = c1*SensorValue + c2
*/
void AQUA_orp::_setCalibrationValues() {
  uint8_t i,j;
  bool isCorrect;

  _usedPoints = 0;
  for(i = 0; i < _pointCount; i++) {
    if(_calData[i].state == 1 && _calData[i].actValue > 0 && _calData[i].refValue > 0) {
      if(_usedPoints > 0) {
        isCorrect = 1;
        for(j = 0; j < _usedPoints; j++) {
          if(_calData[i].actValue == _usedData[j].actValue || _calData[i].refValue == _usedData[j].refValue) {
            isCorrect = 0;
            break;
          }
        }
        if(isCorrect == 1) {
          j = _usedPoints;
          for(j; j > 0 && _usedData[j-1].actValue > _calData[i].actValue; --j) {
            _usedData[j].state = _usedData[j-1].state;
            _usedData[j].refValue = _usedData[j-1].refValue;
            _usedData[j].actValue = _usedData[j-1].actValue;
          }
          _usedData[j].state = _calData[i].state;
          _usedData[j].refValue = _calData[i].refValue;
          _usedData[j].actValue = _calData[i].actValue;
          _usedPoints++;
        }
      } else {
        _usedData[_usedPoints].state = _calData[i].state;
        _usedData[_usedPoints].refValue = _calData[i].refValue;
        _usedData[_usedPoints].actValue = _calData[i].actValue;
        _usedPoints++;
      }
    }
  }
  if(_usedPoints == 1) {
    _const[0] = (_usedData[0].refValue - _usedData[0].actValue);
  } else if(_usedPoints > 1) {
    for(i = 0; i < _usedPoints - 1; i++) {
      _const[i*2] = (_usedData[i+1].refValue - _usedData[i].refValue)/(_usedData[i+1].actValue - _usedData[i].actValue);
      _const[i*2 + 1] = _usedData[i+1].refValue - _const[i*2]*_usedData[i+1].actValue;
    }
  }
}
