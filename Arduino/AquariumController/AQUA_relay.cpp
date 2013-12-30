/*
  Project: Aquarium Controller
  Library: Relay
  Version: 1.0
  Author: Rastislav Birka
*/

#include <avr/eeprom.h>
#include <Arduino.h>
#include "AQUA_relay.h"

/*
  Public Functions
*/

void AQUA_relay::init(uint8_t firstPin, uint8_t alarms, uint8_t timers, uint8_t timer_parts, uint8_t alarms_address, uint8_t timers_address) {
  uint16_t value, position;
  uint8_t i,j;
  bool negative;

  _firstPin = firstPin;
  _alarmCount = alarms;
  _timerCount = timers;
  _timerParts = timer_parts;
  _alarmAddress = alarms_address;
  _timerAddress = timers_address;
  _values = new uint8_t[_alarmCount + _timerCount];
  _alarms = new AQUA_relayAlarm[_alarmCount];
  _timers = new AQUA_relayTimer[_timerCount];
  _manual = new AQUA_relayManual[_alarmCount + _timerCount];

  position = 0;
  for(i = 0; i < _alarmCount; i++) {
    eeprom_busy_wait();
    value = eeprom_read_word((const uint16_t *)(_alarmAddress + position));
    if(value == 0xFFFF) { //address wasn't set yet
      _alarms[i].state = 0;
      _alarms[i].start = 0;
      _alarms[i].stop = 0;
      position+= 4;
    } else {
      _alarms[i].state = (bool)(value & 32768);
      if(_alarms[i].state == 1) {
         value-= 32768;
      }
      negative = (bool)(value & 16384);
      if(negative == 1) {
         value = 0 - (value - 16384);
      }
      _alarms[i].start = value;
      position+= 2;
      eeprom_busy_wait();
      value = eeprom_read_word((const uint16_t *)(_alarmAddress + position));
      negative = (bool)(value & 16384);
      if(negative == 1) {
         value = 0 - (value - 16384);
      }
      _alarms[i].stop = value;
      position+= 2;
    }
  }

  position = 0;
  for(i = 0; i < _timerCount; i++) {
    _timers[i].part = new AQUA_relayTimerPart[_timerParts];
    for(j = 0; j < _timerParts; j++) {
      eeprom_busy_wait();
      value = eeprom_read_word((const uint16_t *)(_timerAddress + position));
      if(value == 0xFFFF) { //address wasn't set yet
        _timers[i].part[j].state = 0;
        _timers[i].part[j].from = 0;
        _timers[i].part[j].to = 0;
        position+= 4;
      } else {
        _timers[i].part[j].state = (bool)(value & 32768);
        if(_timers[i].part[j].state == 1) {
           value-= 32768;
        }
        _timers[i].part[j].from = value;
        position+= 2;
        eeprom_busy_wait();
        _timers[i].part[j].to = eeprom_read_word((const uint16_t *)(_timerAddress + position));
        position+= 2;
      }
    }
  }

  for(i = 0; i < (_alarmCount + _timerCount); i++) {
    _values[i] = AQUA_RELAY_ON;
    set(i, AQUA_RELAY_OFF);
    pinMode(_firstPin+i, OUTPUT);
    _manual[i].manual = 0;
    _manual[i].value = AQUA_RELAY_OFF;
  }
}

bool AQUA_relay::get(uint8_t relay) {
  bool res = AQUA_RELAY_OFF;

  if(relay < (_alarmCount + _timerCount) && relay >= 0) {
    res = _values[relay];
  }
  return res;
}

bool AQUA_relay::set(uint8_t relay, bool value) {
  bool res = false;

  if(relay < (_alarmCount + _timerCount) && relay >= 0) {
    if(_values[relay] != value) {
      _values[relay] = value;
      digitalWrite(_firstPin+relay, _values[relay]);
      delay(50);
      res = true;
    }
  }
  return res;
}

bool AQUA_relay::getByAlarm(uint8_t relay, int value) {
  bool res = AQUA_RELAY_OFF;

  if(relay < _alarmCount && relay >= 0) {
    if(_alarms[relay].state == 1) {
      if(_alarms[relay].start > _alarms[relay].stop) {
        if(_values[relay] == AQUA_RELAY_OFF && value >= _alarms[relay].start) {
          res = AQUA_RELAY_ON;
        } else if(_values[relay] == AQUA_RELAY_ON && value > _alarms[relay].stop) {
          res = AQUA_RELAY_ON;
        }
      } else if(_alarms[relay].start < _alarms[relay].stop) {
        if(_values[relay] == AQUA_RELAY_OFF && value <= _alarms[relay].start) {
          res = AQUA_RELAY_ON;
        } else if(_values[relay] == AQUA_RELAY_ON && value < _alarms[relay].stop) {
          res = AQUA_RELAY_ON;
        }
      } else {
        if(value != _alarms[relay].start) {
          res = AQUA_RELAY_ON;
        }
      }
    }
  }
  return res;
}

bool AQUA_relay::getByTimer(uint8_t relay, uint16_t tstamp) {
  bool res = AQUA_RELAY_OFF;

  if(relay < (_alarmCount + _timerCount) && relay >= _alarmCount) {
    relay-= _alarmCount;
    for(uint8_t j = 0; j < _timerParts; j++) {
      if(_timers[relay].part[j].state == 1) {
        if(_timers[relay].part[j].from == _timers[relay].part[j].to) {
          res = AQUA_RELAY_ON;
        } else if(_timers[relay].part[j].from < _timers[relay].part[j].to) {
            if(tstamp >= _timers[relay].part[j].from && tstamp < _timers[relay].part[j].to) {
              res = AQUA_RELAY_ON;
            }
        } else {
          if(tstamp >= _timers[relay].part[j].from || tstamp < _timers[relay].part[j].to) {
            res = AQUA_RELAY_ON;
          }
        }
      }
    }
  }
  return res;
}

bool AQUA_relay::isManual(uint8_t relay) {
  bool res = false;

  if(relay < (_alarmCount + _timerCount) && relay >= 0) {
    res = _manual[relay].manual;
  }
  return res;
}

bool AQUA_relay::getByManual(uint8_t relay) {
  bool res = AQUA_RELAY_OFF;

  if(relay < (_alarmCount + _timerCount) && relay >= 0) {
    if(_manual[relay].manual == 1) {
      res = _manual[relay].value;
    } else {
      res = _values[relay];
    }
  }
  return res;
}

bool AQUA_relay::setManual(uint8_t relay, bool manual) {
  bool res = false;

  if(relay < (_alarmCount + _timerCount) && relay >= 0) {
    _manual[relay].manual = manual;
    res = true;
  }
  return res;
}

bool AQUA_relay::setManualValue(uint8_t relay, bool value) {
  bool res = false;

  if(relay < (_alarmCount + _timerCount) && relay >= 0) {
    _manual[relay].value = value;
    res = true;
  }
  return res;
}

AQUA_relayAlarm AQUA_relay::readRelayAlarm(uint8_t relay) {
  AQUA_relayAlarm alarm;
  if(relay < _alarmCount && relay >= 0) {
    alarm.state = _alarms[relay].state;
    alarm.start = _alarms[relay].start;
    alarm.stop = _alarms[relay].stop;
  } else {
    alarm.state = 0;
    alarm.start = 0;
    alarm.stop = 0;
  }
  return alarm;
}

AQUA_relayTimerPart AQUA_relay::readRelayTimerPart(uint8_t relay, uint8_t part) {
  AQUA_relayTimerPart timerPart;
  if(relay < (_alarmCount + _timerCount) && relay >= _alarmCount && part < _timerParts && part >= 0) {
    relay-= _alarmCount;
    timerPart.state = _timers[relay].part[part].state;
    timerPart.from = _timers[relay].part[part].from;
    timerPart.to = _timers[relay].part[part].to;
  } else {
    timerPart.state = 0;
    timerPart.from = 0;
    timerPart.to = 0;
  }
  return timerPart;
}

bool AQUA_relay::writeRelayAlarm(uint8_t relay, AQUA_relayAlarm *values) {
  bool res = false;

  if(relay < _alarmCount && relay >= 0 && values->start < 16384 && values->start > -16384 && values->stop < 16384 && values->stop > -16384) {
    if(values->state != _alarms[relay].state || values->start != _alarms[relay].start || values->stop != _alarms[relay].stop) {
      _alarms[relay].state = values->state;
      _alarms[relay].start = values->start;
      _alarms[relay].stop = values->stop;
      uint16_t plusValue = 0;
      if(_alarms[relay].state == 1) {
        plusValue+= 32768;
      }
      if(_alarms[relay].start < 0) {
        plusValue+= 16384;
      }
      uint16_t position = relay*4;
      eeprom_busy_wait();
      eeprom_write_word((uint16_t *)(_alarmAddress + position), abs(_alarms[relay].start) + plusValue);
      plusValue = 0;
      if(_alarms[relay].stop < 0) {
        plusValue+= 16384;
      }
      position+= 2;
      eeprom_busy_wait();
      eeprom_write_word((uint16_t *)(_alarmAddress + position), abs(_alarms[relay].stop) + plusValue);
      res = true;
    }
  }
  return res;
}

bool AQUA_relay::writeRelayTimer(uint8_t relay, uint8_t part, AQUA_relayTimerPart *values) {
  bool res = false;

  if(relay < (_alarmCount + _timerCount) && relay >= _alarmCount && part < _timerParts && part >= 0 && values->from < 1440 && values->from >= 0 && values->to < 1440 && values->to >= 0) {
    relay-= _alarmCount;
    if(values->state != _timers[relay].part[part].state || values->from != _timers[relay].part[part].from || values->to != _timers[relay].part[part].to) {
      _timers[relay].part[part].state = values->state;
      _timers[relay].part[part].from = values->from;
      _timers[relay].part[part].to = values->to;
      uint16_t plusValue = 0;
      if(_timers[relay].part[part].state == 1) {
        plusValue+= 32768;
      }
      uint16_t position = (relay*_timerParts*4) + (part*4);
      eeprom_busy_wait();
      eeprom_write_word((uint16_t *)(_timerAddress + position), _timers[relay].part[part].from + plusValue);
      position+= 2;
      eeprom_busy_wait();
      eeprom_write_word((uint16_t *)(_timerAddress + position), _timers[relay].part[part].to);
      res = true;
    }
  }
  return res;
}
