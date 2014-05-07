/*
  Project: Aquarium Controller
  Library: ADS1115
  Version: 2.1
  Author: Rastislav Birka
*/

#include <Arduino.h>
#include "AQUA_ads1115.h"

/*
  Public Functions
*/

void AQUA_ads1115::init(uint8_t sdaPin, uint8_t sclPin) {
  _sdaPin = sdaPin;
  _sclPin = sclPin;
}

int16_t AQUA_ads1115::getValue() {
  int16_t value = 0;
  return value;
}

/*
  Private Functions
*/

