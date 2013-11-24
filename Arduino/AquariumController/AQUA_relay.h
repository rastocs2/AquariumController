/*
  Project: Aquarium Controller
  Library: Relay
  Version: 1.0
  Author: Rastislav Birka
*/

#ifndef AQUA_relay_h
#define AQUA_relay_h

#define AQUA_RELAY_ON  0 //relay is turn on
#define AQUA_RELAY_OFF 1 //relay is turn off

typedef struct {
  bool manual;
  uint8_t value;
} AQUA_relayManual;

typedef struct {
  bool state;
  int start;
  int stop;
} AQUA_relayAlarm;

typedef struct {
  bool state;
  uint16_t from;
  uint16_t to;
} AQUA_relayTimerPart;

typedef struct {
  AQUA_relayTimerPart* part;
} AQUA_relayTimer;

class AQUA_relay {
  public:
    void init(uint8_t firstPin, uint8_t alarms, uint8_t timers, uint8_t timer_parts, uint8_t alarms_address, uint8_t timers_address);
    bool get(uint8_t relay);
    bool set(uint8_t relay, bool value);
    bool getByAlarm(uint8_t relay, int value);
    bool getByTimer(uint8_t relay, uint16_t tstamp);
    bool isManual(uint8_t relay);
    bool getByManual(uint8_t relay);
    bool setManual(uint8_t relay, bool manual);
    bool setManualValue(uint8_t relay, bool state);
    AQUA_relayAlarm readRelayAlarm(uint8_t relay);
    AQUA_relayTimerPart readRelayTimerPart(uint8_t relay, uint8_t part);
    bool writeRelayAlarm(uint8_t relay, AQUA_relayAlarm *values);
    bool writeRelayTimer(uint8_t relay, uint8_t part, AQUA_relayTimerPart *values);

  private:
    uint8_t _firstPin;
    uint8_t _alarmCount;
    uint8_t _timerCount;
    uint8_t _timerParts;
    uint8_t _alarmAddress;
    uint8_t _timerAddress;
    uint8_t* _values;
    AQUA_relayAlarm* _alarms;
    AQUA_relayTimer* _timers;
    AQUA_relayManual* _manual;
};
#endif
