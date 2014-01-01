/*
  Project: Aquarium Controller
  Library: LCD - 4D Systems - Visi Genie
  Version: 1.0
  Author: Rastislav Birka
*/

#include <avr/eeprom.h>
#include <Arduino.h>
#include "AQUA_lcd.h"

void AQUA_lcd::init(uint8_t timeout_address) {
  _timeoutAddress = timeout_address;
  eeprom_busy_wait();
  _timeout = eeprom_read_word((const uint16_t *)_timeoutAddress);
  if(_timeout == 0xFFFF) {
    _timeout = 30; //default timeout is 30 seconds
  }
  _actualForm = GENIE_FORM_MAIN;
}

uint16_t AQUA_lcd::getTimeout(void) {
  return _timeout;
}

void AQUA_lcd::setTimeout(uint16_t timeout) {
  if(timeout != _timeout) {
    _timeout = timeout;
    eeprom_busy_wait();
    eeprom_write_word((uint16_t *)_timeoutAddress, _timeout);
  }
}

uint8_t AQUA_lcd::getActualForm(void) {
  return _actualForm;
}

void AQUA_lcd::setActualForm(uint8_t form) {
  if(form >= GENIE_FORM_MAIN && form <= GENIE_FORM_CALIBRATION) {
    _actualForm = form;
  }
}
