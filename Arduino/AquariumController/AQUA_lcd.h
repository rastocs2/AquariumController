/*
  Project: Aquarium Controller
  Library: LCD - 4D Systems - Visi Genie
  Version: 1.0
  Author: Rastislav Birka
*/

#ifndef AQUA_lcd_h
#define AQUA_lcd_h

#define GENIE_FORM_MAIN             0
#define GENIE_FORM_SLEEP            1
#define GENIE_FORM_SET_TIME         2
#define GENIE_FORM_SET_ALARM        3
#define GENIE_FORM_SET_TIMER        4
#define GENIE_FORM_CALIBRATION      5
#define GENIE_FORM_CALIBRATION_SHOW 6

#define GENIE_BTN_WAKEUP            0
#define GENIE_BTN_SLEEP             0
#define GENIE_BTN_SET_TIME          1
#define GENIE_BTN_CALIBRATION_TEMP  2
#define GENIE_BTN_CALIBRATION_PH    3
#define GENIE_BTN_CALIBRATION_ORP   4
#define GENIE_BTN_SET_R1            5
#define GENIE_BTN_SET_R2            6
#define GENIE_BTN_SET_R3            7
#define GENIE_BTN_SET_R4            8
#define GENIE_BTN_SET_R5            9
#define GENIE_BTN_SET_R6           10
#define GENIE_BTN_SET_R7           11
#define GENIE_BTN_SET_R8           12

#define GENIE_BTN_CONTROL_R1        0
#define GENIE_BTN_CONTROL_R2        1
#define GENIE_BTN_CONTROL_R3        2
#define GENIE_BTN_CONTROL_R4        3
#define GENIE_BTN_CONTROL_R5        4
#define GENIE_BTN_CONTROL_R6        5
#define GENIE_BTN_CONTROL_R7        6
#define GENIE_BTN_CONTROL_R8        7
#define GENIE_BTN_ONOFF_R1          8
#define GENIE_BTN_ONOFF_R2          9
#define GENIE_BTN_ONOFF_R3         10
#define GENIE_BTN_ONOFF_R4         11
#define GENIE_BTN_ONOFF_R5         12
#define GENIE_BTN_ONOFF_R6         13
#define GENIE_BTN_ONOFF_R7         14
#define GENIE_BTN_ONOFF_R8         15

#define GENIE_BTN_BACK_TIME        13
#define GENIE_BTN_BACK_ALARM       14
#define GENIE_BTN_BACK_TIMER       15
#define GENIE_BTN_BACK_CALIBRATION 16

#define GENIE_BTN_SET_DAY          17
#define GENIE_BTN_SET_MONTH        18
#define GENIE_BTN_SET_YEAR         19
#define GENIE_BTN_SET_WDAY         20
#define GENIE_BTN_SET_HOUR         21
#define GENIE_BTN_SET_MIN          22
#define GENIE_BTN_SET_TIMEOUT      23
#define GENIE_BTN_SAVE_DATE        24
#define GENIE_BTN_SAVE_TIME        25
#define GENIE_BTN_SAVE_WDAY        26
#define GENIE_BTN_SAVE_TIMEOUT     27

#define GENIE_BTN_ALARM_STATE      16
#define GENIE_BTN_ALARM_SET_START  28
#define GENIE_BTN_ALARM_SET_STOP   29
#define GENIE_BTN_SAVE_ALARM       30

#define GENIE_BTN_TIMER1_STATE     17
#define GENIE_BTN_TIMER2_STATE     18
#define GENIE_BTN_TIMER3_STATE     19
#define GENIE_BTN_TIMER4_STATE     20
#define GENIE_BTN_TIMER1_SET_START 31
#define GENIE_BTN_TIMER1_SET_STOP  32
#define GENIE_BTN_TIMER2_SET_START 33
#define GENIE_BTN_TIMER2_SET_STOP  34
#define GENIE_BTN_TIMER3_SET_START 35
#define GENIE_BTN_TIMER3_SET_STOP  36
#define GENIE_BTN_TIMER4_SET_START 37
#define GENIE_BTN_TIMER4_SET_STOP  38
#define GENIE_BTN_SAVE_TIMER1      39
#define GENIE_BTN_SAVE_TIMER2      40
#define GENIE_BTN_SAVE_TIMER3      41
#define GENIE_BTN_SAVE_TIMER4      42

#define GENIE_BTN_CAL1_STATE       21
#define GENIE_BTN_CAL2_STATE       22
#define GENIE_BTN_CAL3_STATE       23
#define GENIE_BTN_CAL1_SET_REF     43
#define GENIE_BTN_CAL2_SET_REF     44
#define GENIE_BTN_CAL3_SET_REF     45
#define GENIE_BTN_CAL1_READ_ACT    46
#define GENIE_BTN_CAL2_READ_ACT    47
#define GENIE_BTN_CAL3_READ_ACT    48
#define GENIE_BTN_SAVE_CALIBRATION 49

#define GENIE_KEYBOARD_TIME         0
#define GENIE_KEYBOARD_ALARM        1
#define GENIE_KEYBOARD_TIMER        2
#define GENIE_KEYBOARD_CALIBRATION  3
#define GENIE_KEYBOARD_CLEAR_KEY   99

#define GENIE_GAUGE_CALIBRATION     0

#define GENIE_STR_TIME              0
#define GENIE_STR_TEMP              1
#define GENIE_STR_PH                2
#define GENIE_STR_ORP               3

#define GENIE_STR_TIME_KEY          4
#define GENIE_STR_DAY               5
#define GENIE_STR_MONTH             6
#define GENIE_STR_YEAR              7
#define GENIE_STR_WDAY              8
#define GENIE_STR_HOUR              9
#define GENIE_STR_MIN              10
#define GENIE_STR_SLEEP_TIMEOUT    11

#define GENIE_STR_ALARM_INFO       12
#define GENIE_STR_ALARM_KEY        13
#define GENIE_STR_ALARM_START      14
#define GENIE_STR_ALARM_STOP       15

#define GENIE_STR_TIMER_INFO       16
#define GENIE_STR_TIMER_KEY        17
#define GENIE_STR_TIMER1_START     18
#define GENIE_STR_TIMER1_STOP      19
#define GENIE_STR_TIMER2_START     20
#define GENIE_STR_TIMER2_STOP      21
#define GENIE_STR_TIMER3_START     22
#define GENIE_STR_TIMER3_STOP      23
#define GENIE_STR_TIMER4_START     24
#define GENIE_STR_TIMER4_STOP      25

#define GENIE_STR_CAL_INFO         26
#define GENIE_STR_CAL_KEY          27
#define GENIE_STR_CAl1_REF_OLD     28
#define GENIE_STR_CAl2_REF_OLD     29
#define GENIE_STR_CAl3_REF_OLD     30
#define GENIE_STR_CAl1_REF_NEW     31
#define GENIE_STR_CAl2_REF_NEW     32
#define GENIE_STR_CAl3_REF_NEW     33
#define GENIE_STR_CAl1_ACT_OLD     34
#define GENIE_STR_CAl2_ACT_OLD     35
#define GENIE_STR_CAl3_ACT_OLD     36
#define GENIE_STR_CAl1_ACT_NEW     37
#define GENIE_STR_CAl2_ACT_NEW     38
#define GENIE_STR_CAl3_ACT_NEW     39

#define GENIE_STR_INFO_TIME        40
#define GENIE_STR_INFO_ALARM       41
#define GENIE_STR_INFO_TIMER       42
#define GENIE_STR_INFO_CALIBRATION 43
#define GENIE_STR_ACTUAL_VALUE     44

#define GENIE_CALIBRATION_OFF       0
#define GENIE_CALIBRATION_TEMP      1
#define GENIE_CALIBRATION_PH        2
#define GENIE_CALIBRATION_ORP       3

class AQUA_lcd {
  public:
    void init(uint8_t timeout_address);
    uint16_t getTimeout(void);
    void setTimeout(uint16_t timeout);
    uint8_t getActualForm(void);
    void setActualForm(uint8_t form);

  private:
    uint16_t _timeout;
    uint8_t _timeoutAddress;
    uint8_t _actualForm;
};
#endif
