/*
  Project: Aquarium Controller
  Library: ADC141S626
  Version: 2.1
  Author: Rastislav Birka
*/

#include <Arduino.h>
#include "AQUA_adc141s626.h"

/*
  Public Functions
*/

void AQUA_adc141s626::init(uint8_t voutPin, uint8_t misoPin, uint8_t mosiPin, uint8_t sclkPin, uint8_t ssPin) {
  _voutPin = voutPin;
  _misoPin = misoPin;
  _mosiPin = mosiPin;
  _sclkPin = sclkPin;
  _ssPin = ssPin;
}

int16_t AQUA_adc141s626::getValue() {
  int16_t value = 0;
  return value;
}

/*
  Private Functions
*/

