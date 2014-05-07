/*
  Project: Aquarium Controller
  Library: PH
  Version: 2.1
  Author: Rastislav Birka
*/

#ifndef AQUA_ph_h
#define AQUA_ph_h

#include "AQUA_adc141s626.h"
#include "AQUA_ads1115.h"

typedef struct {
  bool state;
  float refValue;
  float actValue;
} AQUA_phCalibrationPoint;

class AQUA_ph {
  public:
    void init(uint8_t voutPin, uint8_t vocmPin, uint8_t calibrate_points, uint8_t calibrate_address, int vRef = 5000, uint8_t adc_bit = 10, float alpha = 59.16);
    void useInternalADC();
    void useADC141S626(uint8_t _voutPin, uint8_t _misoPin = 50, uint8_t _mosiPin = 51, uint8_t _sclkPin = 52, uint8_t _ssPin = 53);
    void useADS1115(uint8_t sdaPin, uint8_t sclPin);
    float getPH(bool calibrate = 0);
    bool calibration(uint8_t point, AQUA_phCalibrationPoint *values);
    AQUA_phCalibrationPoint readCalibrationPoint(uint8_t point);

  private:
    uint8_t _adc;
    uint8_t _voutPin, _vocmPin;
    uint8_t _pointCount;
    uint8_t _calibrateAddress;
    AQUA_phCalibrationPoint* _calData;
    AQUA_phCalibrationPoint* _usedData;
    uint8_t _usedPoints;
    float* _const;
    float _alpha, _constPerUnit;
    AQUA_adc141s626 *objADC141S626;
    AQUA_ads1115 *objADS1115;

    void _setCalibrationValues();
    float _readInternalADC();
};
#endif
