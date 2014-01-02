/*
  Project: Aquarium Controller
  Library: Time (DS3231)
  Version: 1.0
  Author: Rastislav Birka
*/

#ifndef AQUA_time_h
#define AQUA_time_h

#define DS_TYPE_1307       0
#define DS_TYPE_3231       1

#define TIME_ADDR_READ     0xD1 //slave address for read:  11010001
#define TIME_ADDR_WRITE    0xD0 //slave address for write: 11010000
#define TIME_ADDR_SEC      0x00
#define TIME_ADDR_MIN      0x01
#define TIME_ADDR_HOUR     0x02
#define TIME_ADDR_WDAY     0x03
#define TIME_ADDR_DAY      0x04
#define TIME_ADDR_MON      0x05
#define TIME_ADDR_YEAR     0x06
#define TIME_ADDR_A1_SEC   0x07
#define TIME_ADDR_A1_MIN   0x08
#define TIME_ADDR_A1_HOUR  0x09
#define TIME_ADDR_A1_DYDT  0x0A
#define TIME_ADDR_A2_MIN   0x0B
#define TIME_ADDR_A2_HOUR  0x0C
#define TIME_ADDR_A2_DYDT  0x0D
#define TIME_ADDR_STATUS   0x0F
#define TIME_ADDR_AGING    0x10
#define TIME_ADDR_TEMP_MSB 0x11
#define TIME_ADDR_TEMP_LSB 0x12

#define TIME_ADDR_CONTROL_1307 0x07
#define TIME_ADDR_CONTROL_3231 0x0E

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
    void init(uint8_t dataPin, uint8_t clockPin, uint8_t dsType = DS_TYPE_1307, bool useDST = false, uint8_t timeZone = 0);
    void setOutput(bool enable);
    void enableSQW(bool enable);
    void setSQWRate(int rate);
    void start();
    void stop();

    void setDate(uint8_t day, uint8_t mon, uint16_t year);
    void setTime(uint8_t hour, uint8_t min, uint8_t sec);
    void setWday(uint8_t wday);
    void setDST(bool useDST);
    void setTimeZone(int timeZone);
    AQUA_datetime getDateTime();

  private:
    uint8_t _dataPin, _clockPin, _dsType;
    uint8_t _regDateTime[8];
    bool _useDST; //whether will used DTS or no
    uint8_t _timeZone; //it is used only for calculating DTS and only for time zone >= 0

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
