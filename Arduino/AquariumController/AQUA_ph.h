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
    void init(uint8_t dqPin, uint8_t calibrate_points, uint8_t calibrate_address);
    float getPH(bool calibrate = 0);
    bool calibration(uint8_t point, AQUA_phCalibrationPoint *values);
    AQUA_phCalibrationPoint readCalibrationPoint(uint8_t point);

  private:
    uint8_t _dqPin;
    uint8_t _pointCount;
    uint8_t _calibrateAddress;
    AQUA_phCalibrationPoint* _calData;
    AQUA_phCalibrationPoint* _usedData;
    uint8_t _usedPoints;
    float* _const;

    void _setCalibrationValues();
};
#endif
