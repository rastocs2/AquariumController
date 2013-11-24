/*
  Project: Aquarium Controller
  Library: Time
  Version: 1.0
  Author: Rastislav Birka
*/

#ifndef AQUA_time_h
#define AQUA_time_h

#define TIME_ADDR_READ    0xD1 //DS1307 slave address for read:  11010001
#define TIME_ADDR_WRITE   0xD0 //DS1307 slave address for write: 11010000
#define TIME_ADDR_SEC     0    //00-59
#define TIME_ADDR_MIN     1    //00-59
#define TIME_ADDR_HOUR    2    //00-23
#define TIME_ADDR_WDAY    3    //01-07
#define TIME_ADDR_DAY     4    //01-31
#define TIME_ADDR_MON     5    //01-12
#define TIME_ADDR_YEAR    6    //00-99
#define TIME_ADDR_CONTROL 7

typedef struct {
  uint8_t sec;
  uint8_t min;
  uint8_t hour;
  uint8_t wday;
  uint8_t day;
  uint8_t mon;
  uint16_t year;
} AQUA_datetime;

class AQUA_time {
  public:
    void init(uint8_t dataPin, uint8_t clockPin);
    void setOutput(bool enable);
    void enableSQW(bool enable);
    void setSQWRate(int rate);
    void start();
    void stop();

    void setDate(uint8_t day, uint8_t mon, uint16_t year);
    void setTime(uint8_t hour, uint8_t min, uint8_t sec);
    void setWday(uint8_t wday);
    AQUA_datetime getDateTime();

  private:
    uint8_t _dataPin, _clockPin;
    uint8_t _regDateTime[8];

    void _sendStart(uint8_t addr);
    void _sendStop();
    void _sendAck();
    void _sendNack();
    void _waitForAck();
    uint8_t _readByte();
    void _writeByte(uint8_t value);
    uint8_t _readRegister(uint8_t reg);
    void _writeRegister(uint8_t reg, uint8_t value);
    void _readDateTime();
    uint8_t _decode(uint8_t value);
    uint8_t _encode(uint8_t value);
};
#endif
