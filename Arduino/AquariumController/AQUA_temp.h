/*
  Project: Aquarium Controller
  Library: Temperature (a 4.7K resistor is necessary)
  Version: 1.0
  Author: Rastislav Birka
*/

#ifndef AQUA_temp_h
#define AQUA_temp_h

#define DS18B20_RPWRSUPPLY 0xB4
#define DS18B20_SEARCHROM 0xF0
#define DS18B20_READROM 0x33
#define DS18B20_MATCHROM 0x55
#define DS18B20_SKIPROM 0xCC
#define DS18B20_ALARMSEARCH 0xEC
#define DS18B20_CONVERTTEMP 0x44
#define DS18B20_RSCRATCHPAD 0xBE
#define DS18B20_WSCRATCHPAD 0x4E
#define DS18B20_CPYSCRATCHPAD 0x48
#define DS18B20_RECEEPROM 0xB8

typedef struct {
  bool state;
  float refValue;
  float actValue;
} AQUA_tempCalibrationPoint;

class AQUA_temp {
  public:
    void init(uint8_t dqPin, uint8_t calibrate_points, uint8_t calibrate_address);
    float getTemp(bool calibrate = 0);
    bool calibration(uint8_t point, AQUA_tempCalibrationPoint *values);
    AQUA_tempCalibrationPoint readCalibrationPoint(uint8_t point);

  private:
    uint8_t _dqPin;
    uint8_t _pointCount;
    uint8_t _calibrateAddress;
    AQUA_tempCalibrationPoint* _calData;
    AQUA_tempCalibrationPoint* _usedData;
    uint8_t _usedPoints;
    float* _const;

    void _setCalibrationValues();
    bool _reset(void);
    uint8_t _read(void);
    uint8_t _read_bit(void);
    void _write(uint8_t command);
    void _write_bit(uint8_t value);
    void _wait_to_convert(void);
};
#endif
