/*
  Project: Aquarium Controller
  Library: Constants
  Version: 2.1
  Author: Rastislav Birka
*/

#define AQUA_DEBUG_MODE_ON          0 //0-live application, 1-debug mode with print to serial
#define AQUA_ACTIVITY_LED          12 //pin where is led for indicate activity
#define AQUA_WAKEUP_BUTTON          2 //pin where is button for wakeup
#define AQUA_ANALOG_VREF         3300 //reference voltage for analog (mV)
#define AQUA_ADC_BIT_RESOLUTION    10 //ADC bit resolution (10 for arduino internal ADC)

#define AQUA_RELAY_FIRST_PIN       30 //for 8 relay module will use next 7 pins (30,...,37)
#define AQUA_RELAY_ALARMS           4 //number of alarms (temperature high, temperature low, pH high, pH low)
#define AQUA_RELAY_TIMERS           4 //number of timers (day light, night light, ...)
#define AQUA_RELAY_TIMER_PARTS      4 //number of timer parts (day light, night light, ...)
#define AQUA_RELAY_ALARMS_ADDR   0x00 //4 bytes for each alarm = 2 bytes for start value and 2 bytes for stop value (4 alarms = 16 bytes)
#define AQUA_RELAY_TIMERS_ADDR   0x10 //4 bytes for each timer part = 2 bytes for start timestamp and 2 bytes for end timestamp (4 timer parts for each timer = 16 timer parts = 64 bytes)

#define AQUA_TIME_DATA_PIN         22 //SDA pin
#define AQUA_TIME_CLOCK_PIN        23 //SCLK pin
#define AQUA_TIME_SQ_PIN           24 //SQ pin
#define AQUA_TIME_DS_PIN           25 //DS pin

#define AQUA_TEMP_DQ_PIN           26 //DQ pin
#define AQUA_TEMP_CALIBRATE_ADDR 0x50 //4 bytes for each calibration point = 2 bytes for reference value and 2 bytes for measured value (3 point calibration = 12 bytes)
#define AQUA_TEMP_CALIBRATE_POINTS  3 //number of calibrating points
#define AQUA_TEMP_RELAY_COOLING     0 //cooling
#define AQUA_TEMP_RELAY_HEATING     1 //heating

#define AQUA_PH_DQ_PIN             27 //DQ pin
#define AQUA_PH_CALIBRATE_ADDR   0x60 //4 bytes for each calibration point = 2 bytes for reference value and 2 bytes for measured value (3 point calibration = 12 bytes)
#define AQUA_PH_CALIBRATE_POINTS    3 //number of calibrating points
#define AQUA_PH_RELAY_REDUCTION     2 //pH reduction
#define AQUA_PH_RELAY_INCREASE      3 //pH increase
#define AQUA_PH_VOUT_PIN            0 //VOUT pin - Analog Output
#define AQUA_PH_VOCM_PIN            1 //VOCM pin - Output common mode voltage

#define AQUA_ORP_DQ_PIN            28 //DQ pin
#define AQUA_ORP_CALIBRATE_ADDR  0x70 //4 bytes for each calibration point = 2 bytes for reference value and 2 bytes for measured value (3 point calibration = 12 bytes)
#define AQUA_ORP_CALIBRATE_POINTS   3 //number of calibrating points
#define AQUA_ORP_VOUT_PIN           4 //VOUT pin - Analog Output
#define AQUA_ORP_VOCM_PIN           5 //VOCM pin - Output common mode voltage

#define AQUA_LCD_DEFAULT_TIMEOUT   30 //timeout for display sleep mode in seconds
#define AQUA_LCD_TIMEOUT_ADDR    0x80 //2 bytes for lcd timeout in seconds
#define AQUA_LCD_SERIAL             2 //which serial port is used for LCD //1 = pin 19(RX) and pin 18(TX), 2 = pin 17(RX) and pin 16(TX), 3 = pin 15(RX) and pin 14(TX)
#define AQUA_LCD_SERIAL_SPEED  115200 //speed for serial communication
#define AQUA_LCD_RESET_PIN          4 //pin for reset

#define AQUA_SPI_MISO_PIN          50 //SPI MISO pin
#define AQUA_SPI_MOSI_PIN          51 //SPI MOSI pin
#define AQUA_SPI_SCLK_PIN          52 //SPI SCLK pin
#define AQUA_SPI_SS_PIN            53 //SPI SS pin
#define AQUA_SPI_PH_VOUT_PIN       49 //pH VOUT pin (ADC141S626)
#define AQUA_SPI_ORP_VOUT_PIN      48 //ORP VOUT pin (ADC141S626)

#define AQUA_ADS1115_SDA_PIN       46 //SDA pin
#define AQUA_ADS1115_SCL_PIN       47 //SCL pin
