/*
  Project: Aquarium Controller
  Library: PH
  Version: 1.0
  Author: Rastislav Birka
*/

#ifndef AQUA_ph_h
#define AQUA_ph_h

typedef struct {
  bool state;
  float refValue;
  float actValue;
} AQUA_phCalibrationPoint;

class AQUA_ph {
  public:
    void init(uint8_t voutPin, uint8_t vocmPin, uint8_t calibrate_points, uint8_t calibrate_address, int vRef = 5000, uint8_t adc_bit = 10, float alpha = 59.16);
    float getPH(bool calibrate = 0);
    bool calibration(uint8_t point, AQUA_phCalibrationPoint *values);
    AQUA_phCalibrationPoint readCalibrationPoint(uint8_t point);

  private:
    uint8_t _voutPin;
    uint8_t _vocmPin;
    uint8_t _pointCount;
    uint8_t _calibrateAddress;
    AQUA_phCalibrationPoint* _calData;
    AQUA_phCalibrationPoint* _usedData;
    uint8_t _usedPoints;
    float* _const;
    float _alpha, _constPerUnit;

    void _setCalibrationValues();
};
#endif
