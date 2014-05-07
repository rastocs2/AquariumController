/*
  Project: Aquarium Controller
  Library: ADS1115
  Version: 2.1
  Author: Rastislav Birka
*/

#ifndef AQUA_ads1115_h
#define AQUA_ads1115_h

class AQUA_ads1115 {
  public:
    void init(uint8_t sdaPin, uint8_t sclPin);
    int16_t getValue();

  private:
    uint8_t _sdaPin, _sclPin;
};
#endif
