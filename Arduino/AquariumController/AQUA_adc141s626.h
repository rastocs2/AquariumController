/*
  Project: Aquarium Controller
  Library: ADC141S626
  Version: 2.1
  Author: Rastislav Birka
*/

#ifndef AQUA_adc141s626_h
#define AQUA_adc141s626_h

class AQUA_adc141s626 {
  public:
    void init(uint8_t _voutPin, uint8_t _misoPin = 50, uint8_t _mosiPin = 51, uint8_t _sclkPin = 52, uint8_t _ssPin = 53);
    int16_t getValue();

  private:
    uint8_t _voutPin, _misoPin, _mosiPin, _sclkPin, _ssPin;
};
#endif
