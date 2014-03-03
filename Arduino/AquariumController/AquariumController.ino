/*
  Project: Aquarium Controller
  Version: 1.0
  Author: Rastislav Birka
*/

#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <genieArduino.h>

#include "AquariumController.h"
#include "AQUA_relay.h"
#include "AQUA_time.h"
#include "AQUA_temp.h"
#include "AQUA_ph.h"
#include "AQUA_orp.h"
#include "AQUA_lcd.h"

AQUA_temp   objTEMP;
AQUA_ph     objPH;
AQUA_orp    objORP;
AQUA_time   objTIME;
AQUA_relay  objRELAY;
AQUA_lcd    objLCD;

static uint8_t relayCount = AQUA_RELAY_ALARMS + AQUA_RELAY_TIMERS;
bool wakeup, wakeupButton, wakeupLCD;
unsigned long inactivity = 0, calWV = 0;
String keyboardString;
bool keyboardStart = true;
AQUA_datetime settingsDatetime;
uint16_t settingsTimeout;
uint8_t settingsRelay;
uint8_t calibration = GENIE_CALIBRATION_OFF;
AQUA_relayAlarm settingsAlarm;
AQUA_relayTimerPart settingsTimer[AQUA_RELAY_TIMER_PARTS];
AQUA_tempCalibrationPoint tempCalPoint[AQUA_TEMP_CALIBRATE_POINTS];
AQUA_phCalibrationPoint phCalPoint[AQUA_PH_CALIBRATE_POINTS];
AQUA_orpCalibrationPoint orpCalPoint[AQUA_ORP_CALIBRATE_POINTS];

/*** setup Watchdog Timer ***/
void setupWDT() {
  MCUSR &= ~(1<<WDRF); //clear reset flag
  WDTCSR |= (1<<WDCE) | (1<<WDE); //in order to change WDE or prescaler, need to set WDCE
  WDTCSR = 1<<WDP0 | 1<<WDP3; //set new watchdog timeout prescaler value (8.0 seconds)
  WDTCSR |= _BV(WDIE); //enable WD interrupt (note no reset)
}

/*** Watchdog Interrupt Service (is executed when watchdog timed out) ***/
ISR(WDT_vect) {
  wakeup = 1;
}

/*** Interrupt 0 Service (pin 2) ***/
void pin2Interrupt() {
  detachInterrupt(0);
  wakeupButton = 1;
}

/*** enters arduino into sleep mode ***/
void enterSleep() {
  if(AQUA_DEBUG_MODE_ON == 1) {
    Serial.println("");Serial.println("Sleep...");
  }
  digitalWrite(AQUA_ACTIVITY_LED, LOW);
  attachInterrupt(0, pin2Interrupt, LOW);
  delay(100);

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_mode();

  //program will continue from here after WDT timeout
  sleep_disable(); //first thing to do is disable sleep

  digitalWrite(AQUA_ACTIVITY_LED, HIGH);
  if(AQUA_DEBUG_MODE_ON == 1) {
    Serial.println("");Serial.println("Wakeup...");
  }
}

/*** display form ***/
void displayForm(uint8_t form, uint8_t index = 0, bool actualValues = false) {
  char row[8];
  uint8_t actualCalibration = calibration;
  AQUA_tempCalibrationPoint tempCalPointOld;
  AQUA_phCalibrationPoint phCalPointOld;
  AQUA_orpCalibrationPoint orpCalPointOld;

  if(AQUA_DEBUG_MODE_ON == 1) {
    Serial.print("displayForm: ");Serial.println(form);
  }
  objLCD.setActualForm(form);
  genieWriteObject(GENIE_OBJ_FORM, form, 0);
  switch(form) {
    case GENIE_FORM_CALIBRATION:
      keyboardStart = true;
      keyboardString = "0";
      calibration = index - GENIE_CALIBRATION_TEMP;
      genieWriteObject(GENIE_OBJ_STRINGS, GENIE_STR_INFO_CALIBRATION, 0);
      genieWriteObject(GENIE_OBJ_STRINGS, GENIE_STR_CAL_INFO, index - GENIE_BTN_CALIBRATION_TEMP);
      sprintf(row, "%5d\0", 0);
      genieWriteStr(GENIE_STR_CAL_KEY, row);
      if(calibration == GENIE_CALIBRATION_TEMP) {
        for(uint8_t i = 0; i < AQUA_TEMP_CALIBRATE_POINTS; i++) {
          tempCalPointOld = objTEMP.readCalibrationPoint(i);
          if(actualValues == false) {
            tempCalPoint[i].state = tempCalPointOld.state;
            tempCalPoint[i].refValue = tempCalPointOld.refValue;
            tempCalPoint[i].actValue = 0;
          }
          genieWriteObject(GENIE_OBJ_4DBUTTON, i + GENIE_BTN_CAL1_STATE, tempCalPoint[i].state);
          sprintf(row, "%2d.%02d\0", (int)tempCalPointOld.refValue, (int)(tempCalPointOld.refValue*100)%100);
          genieWriteStr(i + GENIE_STR_CAl1_REF_OLD, row);
          sprintf(row, "%2d.%02d\0", (int)tempCalPointOld.actValue, (int)(tempCalPointOld.actValue*100)%100);
          genieWriteStr(i + GENIE_STR_CAl1_ACT_OLD, row);
          sprintf(row, "%2d.%02d\0", (int)tempCalPoint[i].refValue, (int)(tempCalPoint[i].refValue*100)%100);
          genieWriteStr(i + GENIE_STR_CAl1_REF_NEW, row);
          sprintf(row, "%2d.%02d\0", (int)tempCalPoint[i].actValue, (int)(tempCalPoint[i].actValue*100)%100);
          genieWriteStr(i + GENIE_STR_CAl1_ACT_NEW, row);
        }
      } else if(calibration == GENIE_CALIBRATION_PH) {
        for(uint8_t i = 0; i < AQUA_PH_CALIBRATE_POINTS; i++) {
          phCalPointOld = objPH.readCalibrationPoint(i);
          if(actualValues == false) {
            phCalPoint[i].state = phCalPointOld.state;
            phCalPoint[i].refValue = phCalPointOld.refValue;
            phCalPoint[i].actValue = 0;
          }
          genieWriteObject(GENIE_OBJ_4DBUTTON, i + GENIE_BTN_CAL1_STATE, phCalPoint[i].state);
          sprintf(row, "%2d.%02d\0", (int)phCalPointOld.refValue, (int)(phCalPointOld.refValue*100)%100);
          genieWriteStr(i + GENIE_STR_CAl1_REF_OLD, row);
          sprintf(row, "%2d.%02d\0", (int)phCalPointOld.actValue, (int)(phCalPointOld.actValue*100)%100);
          genieWriteStr(i + GENIE_STR_CAl1_ACT_OLD, row);
          sprintf(row, "%2d.%02d\0", (int)phCalPoint[i].refValue, (int)(phCalPoint[i].refValue*100)%100);
          genieWriteStr(i + GENIE_STR_CAl1_REF_NEW, row);
          sprintf(row, "%2d.%02d\0", (int)phCalPoint[i].actValue, (int)(phCalPoint[i].actValue*100)%100);
          genieWriteStr(i + GENIE_STR_CAl1_ACT_NEW, row);
        }
      } else if(calibration == GENIE_CALIBRATION_ORP) {
        for(uint8_t i = 0; i < AQUA_ORP_CALIBRATE_POINTS; i++) {
          orpCalPointOld = objORP.readCalibrationPoint(i);
          if(actualValues == false) {
            orpCalPoint[i].state = orpCalPointOld.state;
            orpCalPoint[i].refValue = orpCalPointOld.refValue;
            orpCalPoint[i].actValue = 0;
          }
          genieWriteObject(GENIE_OBJ_4DBUTTON, i + GENIE_BTN_CAL1_STATE, orpCalPoint[i].state);
          sprintf(row, "%+5d\0", (int)orpCalPointOld.refValue);
          genieWriteStr(i + GENIE_STR_CAl1_REF_OLD, row);
          sprintf(row, "%+5d\0", (int)orpCalPointOld.actValue);
          genieWriteStr(i + GENIE_STR_CAl1_ACT_OLD, row);
          sprintf(row, "%+5d\0", (int)orpCalPoint[i].refValue);
          genieWriteStr(i + GENIE_STR_CAl1_REF_NEW, row);
          sprintf(row, "%+5d\0", (int)orpCalPoint[i].actValue);
          genieWriteStr(i + GENIE_STR_CAl1_ACT_NEW, row);
        }
      }
      break;
    case GENIE_FORM_CALIBRATION_SHOW:
      calibration = GENIE_CALIBRATION_OFF;
      genieWriteObject(GENIE_OBJ_GAUGE, GENIE_GAUGE_CALIBRATION, 0);
      if(actualCalibration == GENIE_CALIBRATION_TEMP) {
        float temp = objTEMP.getTemp(1);
        for(uint8_t i = 1; i < 10; i++) {
          genieWriteObject(GENIE_OBJ_GAUGE, GENIE_GAUGE_CALIBRATION, i*10);
          temp+= objTEMP.getTemp(1);
        }
        genieWriteObject(GENIE_OBJ_GAUGE, GENIE_GAUGE_CALIBRATION, 100);
        tempCalPoint[index - GENIE_BTN_CAL1_READ_ACT].actValue = temp/10.00;
        displayForm(GENIE_FORM_CALIBRATION, GENIE_BTN_CALIBRATION_TEMP, true);
      } else if(actualCalibration == GENIE_CALIBRATION_PH) {
        float pH = objPH.getPH(1);
        for(uint8_t i = 1; i < 100; i++) {
          genieWriteObject(GENIE_OBJ_GAUGE, GENIE_GAUGE_CALIBRATION, i);
          pH+= objPH.getPH(1);
        }
        genieWriteObject(GENIE_OBJ_GAUGE, GENIE_GAUGE_CALIBRATION, 100);
        phCalPoint[index - GENIE_BTN_CAL1_READ_ACT].actValue = pH/100.00;
        displayForm(GENIE_FORM_CALIBRATION, GENIE_BTN_CALIBRATION_PH, true);
      } else if(actualCalibration == GENIE_CALIBRATION_ORP) {
        int orp = objORP.getORP(1);
        for(uint8_t i = 1; i < 100; i++) {
          genieWriteObject(GENIE_OBJ_GAUGE, GENIE_GAUGE_CALIBRATION, i);
          orp+= objORP.getORP(1);
        }
        genieWriteObject(GENIE_OBJ_GAUGE, GENIE_GAUGE_CALIBRATION, 100);
        orpCalPoint[index - GENIE_BTN_CAL1_READ_ACT].actValue = (int)(orp/100);
        displayForm(GENIE_FORM_CALIBRATION, GENIE_BTN_CALIBRATION_ORP, true);
      }
      break;
    case GENIE_FORM_SET_TIMER:
      keyboardStart = true;
      keyboardString = "0";
      settingsRelay = index - GENIE_BTN_SET_R1;
      genieWriteObject(GENIE_OBJ_STRINGS, GENIE_STR_INFO_TIMER, 0);
      genieWriteObject(GENIE_OBJ_STRINGS, GENIE_STR_TIMER_INFO, settingsRelay - AQUA_RELAY_TIMERS);
      sprintf(row, "%5d\0", 0);
      genieWriteStr(GENIE_STR_TIMER_KEY, row);
      for(uint8_t i = 0; i < AQUA_RELAY_TIMER_PARTS; i++) {
        settingsTimer[i] = objRELAY.readRelayTimerPart(settingsRelay, i);
        sprintf(row, "%2d:%02d\0", (int)(settingsTimer[i].from/60), settingsTimer[i].from%60);
        genieWriteStr(i*2 + GENIE_STR_TIMER1_START, row);
        sprintf(row, "%2d:%02d\0", (int)(settingsTimer[i].to/60), settingsTimer[i].to%60);
        genieWriteStr(i*2 + GENIE_STR_TIMER1_STOP, row);
        genieWriteObject(GENIE_OBJ_4DBUTTON, i + GENIE_BTN_TIMER1_STATE, settingsTimer[i].state);
      }
      break;
    case GENIE_FORM_SET_ALARM:
      keyboardStart = true;
      keyboardString = "0";
      settingsRelay = index - GENIE_BTN_SET_R1;
      settingsAlarm = objRELAY.readRelayAlarm(settingsRelay);
      genieWriteObject(GENIE_OBJ_STRINGS, GENIE_STR_INFO_ALARM, 0);
      genieWriteObject(GENIE_OBJ_STRINGS, GENIE_STR_ALARM_INFO, settingsRelay);
      genieWriteObject(GENIE_OBJ_4DBUTTON, GENIE_BTN_ALARM_STATE, settingsAlarm.state);
      sprintf(row, "%5d\0", 0);
      genieWriteStr(GENIE_STR_ALARM_KEY, row);
      sprintf(row, "%2d.%02d\0", (int)(settingsAlarm.start/100), settingsAlarm.start%100);
      genieWriteStr(GENIE_STR_ALARM_START, row);
      sprintf(row, "%2d.%02d\0", (int)(settingsAlarm.stop/100), settingsAlarm.stop%100);
      genieWriteStr(GENIE_STR_ALARM_STOP, row);
      break;
    case GENIE_FORM_SET_TIME:
      keyboardStart = true;
      keyboardString = "0";
      settingsDatetime = objTIME.getDateTime();
      settingsTimeout = objLCD.getTimeout();
      genieWriteObject(GENIE_OBJ_STRINGS, GENIE_STR_INFO_TIME, 0);
      sprintf(row, "%5d\0", 0);
      genieWriteStr(GENIE_STR_TIME_KEY, row);
      sprintf(row, "%4d\0", settingsDatetime.day);
      genieWriteStr(GENIE_STR_DAY, row);
      sprintf(row, "%4d\0", settingsDatetime.mon);
      genieWriteStr(GENIE_STR_MONTH, row);
      sprintf(row, "%4d\0", settingsDatetime.year);
      genieWriteStr(GENIE_STR_YEAR, row);
      sprintf(row, "%4d\0", settingsDatetime.wday);
      genieWriteStr(GENIE_STR_WDAY, row);
      sprintf(row, "%4d\0", settingsDatetime.hour);
      genieWriteStr(GENIE_STR_HOUR, row);
      sprintf(row, "%4d\0", settingsDatetime.min);
      genieWriteStr(GENIE_STR_MIN, row);
      sprintf(row, "%4d\0", settingsTimeout);
      genieWriteStr(GENIE_STR_SLEEP_TIMEOUT, row);
      break;
    case GENIE_FORM_SLEEP:
      genieWriteContrast(0);
      wakeupLCD = 0;
      break;
    case GENIE_FORM_MAIN:
    default:
      calibration = GENIE_CALIBRATION_OFF;
      break;
  }
}

/*** 4Dsystems event handler ***/
void lcdEventHandler(void) {
  char row[8];
  long iValue;
  genieFrame Event;
  genieDequeueEvent(&Event);

  if(AQUA_DEBUG_MODE_ON == 1) {
    Serial.println("----------");
    Serial.print("cmd:      ");Serial.println(Event.reportObject.cmd);
    Serial.print("object:   ");Serial.println(Event.reportObject.object);
    Serial.print("index:    ");Serial.println(Event.reportObject.index);
    Serial.print("data_msb: ");Serial.println(Event.reportObject.data_msb);
    Serial.print("data_lsb: ");Serial.println(Event.reportObject.data_lsb);
    Serial.println("----------");
  }

  if(Event.reportObject.cmd == GENIE_REPORT_EVENT || Event.reportObject.cmd == GENIE_REPORT_OBJ) {
    wakeupLCD = 1;
    wakeup = 1;
    inactivity = millis();
  }

  //If the cmd received is from a Reported Event
  if(Event.reportObject.cmd == GENIE_REPORT_EVENT) {
    if(Event.reportObject.object == GENIE_OBJ_4DBUTTON) {
      if(Event.reportObject.index < relayCount) { //manual/auto control buttons
        objRELAY.setManual(Event.reportObject.index, Event.reportObject.data_lsb);
        if(Event.reportObject.data_lsb > 0) { //manual control
          bool value = objRELAY.get(Event.reportObject.index);
          objRELAY.setManualValue(Event.reportObject.index, value);
          genieWriteObject(GENIE_OBJ_4DBUTTON, Event.reportObject.index+relayCount, (value == AQUA_RELAY_OFF ? AQUA_RELAY_ON : AQUA_RELAY_OFF));
        } else { //auto control
          objRELAY.setManualValue(Event.reportObject.index, AQUA_RELAY_OFF);
          genieWriteObject(GENIE_OBJ_4DBUTTON, Event.reportObject.index+relayCount, AQUA_RELAY_ON);
        }
      } else if(Event.reportObject.index >= relayCount && Event.reportObject.index < (relayCount*2)) { //turn on/off relay buttons
        objRELAY.setManualValue(Event.reportObject.index-relayCount, (Event.reportObject.data_lsb == AQUA_RELAY_OFF ? AQUA_RELAY_ON : AQUA_RELAY_OFF));
      } else if(Event.reportObject.index == GENIE_BTN_ALARM_STATE) { //turn on/off alarm button
        settingsAlarm.state = Event.reportObject.data_lsb;
      } else if(Event.reportObject.index >= GENIE_BTN_TIMER1_STATE && Event.reportObject.index <= GENIE_BTN_TIMER4_STATE) { //turn on/off timer buttons
        settingsTimer[Event.reportObject.index - GENIE_BTN_TIMER1_STATE].state = Event.reportObject.data_lsb;
      } else if(Event.reportObject.index >= GENIE_BTN_CAL1_STATE && Event.reportObject.index <= GENIE_BTN_CAL3_STATE) { //turn on/off calibration buttons
        if(calibration == GENIE_CALIBRATION_TEMP) {
          tempCalPoint[Event.reportObject.index - GENIE_BTN_CAL1_STATE].state = Event.reportObject.data_lsb;
        } else if(calibration == GENIE_CALIBRATION_PH) {
          phCalPoint[Event.reportObject.index - GENIE_BTN_CAL1_STATE].state = Event.reportObject.data_lsb;
        } else if(calibration == GENIE_CALIBRATION_ORP) {
          orpCalPoint[Event.reportObject.index - GENIE_BTN_CAL1_STATE].state = Event.reportObject.data_lsb;
        }
      }
    } else if(Event.reportObject.object == GENIE_OBJ_WINBUTTON) {
      if(Event.reportObject.index == GENIE_BTN_SLEEP) {
        displayForm(GENIE_FORM_SLEEP);
      } else {
        if(Event.reportObject.index == GENIE_BTN_SET_TIME) {
          displayForm(GENIE_FORM_SET_TIME);
        } else if(Event.reportObject.index >= GENIE_BTN_CALIBRATION_TEMP && Event.reportObject.index <= GENIE_BTN_CALIBRATION_ORP) { //Calibration
          /*if((Event.reportObject.index == GENIE_BTN_CALIBRATION_TEMP && objTEMP.getTemp() > 0)
              || (Event.reportObject.index == GENIE_BTN_CALIBRATION_PH && objPH.getPH() > 0)
              || (Event.reportObject.index == GENIE_BTN_CALIBRATION_ORP && objORP.getORP() != 0))*/ {
            displayForm(GENIE_FORM_CALIBRATION, Event.reportObject.index);
          }
        } else if(Event.reportObject.index >= GENIE_BTN_SET_R1 && Event.reportObject.index <= GENIE_BTN_SET_R4) { //Alarm settings
          displayForm(GENIE_FORM_SET_ALARM, Event.reportObject.index);
        } else if(Event.reportObject.index >= GENIE_BTN_SET_R5 && Event.reportObject.index <= GENIE_BTN_SET_R8) { //Timer settings
          displayForm(GENIE_FORM_SET_TIMER, Event.reportObject.index);
        } else if(Event.reportObject.index >= GENIE_BTN_BACK_TIME && Event.reportObject.index <= GENIE_BTN_BACK_CALIBRATION) { //Back buttons
          displayForm(GENIE_FORM_MAIN);
        } else if(Event.reportObject.index >= GENIE_BTN_SET_DAY && Event.reportObject.index <= GENIE_BTN_SET_TIMEOUT) { //Time settings - "Set" buttons
          keyboardStart = true;
          iValue = keyboardString.toInt();
          if(Event.reportObject.index == GENIE_BTN_SET_DAY) {
            if(iValue < 0) {
              iValue = 0;
            } else if(iValue > 31) {
              iValue = 31;
            }
            sprintf(row, "%4d\0", iValue);
            settingsDatetime.day = iValue;
            genieWriteStr(GENIE_STR_DAY, row);
          } else if(Event.reportObject.index == GENIE_BTN_SET_MONTH) {
            if(iValue < 0) {
              iValue = 0;
            } else if(iValue > 12) {
              iValue = 12;
            }
            sprintf(row, "%4d\0", iValue);
            settingsDatetime.mon = iValue;
            genieWriteStr(GENIE_STR_MONTH, row);
          } else if(Event.reportObject.index == GENIE_BTN_SET_YEAR) {
            if(iValue < 100) {
              iValue+= 2000;
            } else if(iValue < 2000) {
              iValue = 2000;
            } else if(iValue > 2099) {
              iValue = 2099;
            }
            sprintf(row, "%4d\0", iValue);
            settingsDatetime.year = iValue;
            genieWriteStr(GENIE_STR_YEAR, row);
          } else if(Event.reportObject.index == GENIE_BTN_SET_WDAY) {
            if(iValue < 0) {
              iValue = 0;
            } else if(iValue > 7) {
              iValue = 7;
            }
            sprintf(row, "%4d\0", iValue);
            settingsDatetime.wday = iValue;
            genieWriteStr(GENIE_STR_WDAY, row);
          } else if(Event.reportObject.index == GENIE_BTN_SET_HOUR) {
            if(iValue < 0) {
              iValue = 0;
            } else if(iValue > 23) {
              iValue = 23;
            }
            sprintf(row, "%4d\0", iValue);
            settingsDatetime.hour = iValue;
            genieWriteStr(GENIE_STR_HOUR, row);
          } else if(Event.reportObject.index == GENIE_BTN_SET_MIN) {
            if(iValue < 0) {
              iValue = 0;
            } else if(iValue > 59) {
              iValue = 59;
            }
            sprintf(row, "%4d\0", iValue);
            settingsDatetime.min = iValue;
            genieWriteStr(GENIE_STR_MIN, row);
          } else if(Event.reportObject.index == GENIE_BTN_SET_TIMEOUT) {
            if(iValue < 0) {
              iValue = 0;
            } else if(iValue > 3600) {
              iValue = 3600; //sleep timeout max 1 hour
            }
            sprintf(row, "%4d\0", iValue);
            settingsTimeout = iValue;
            genieWriteStr(GENIE_STR_SLEEP_TIMEOUT, row);
          }
        } else if(Event.reportObject.index >= GENIE_BTN_SAVE_DATE && Event.reportObject.index <= GENIE_BTN_SAVE_TIMEOUT) { //Time settings - "Save" buttons
          if(Event.reportObject.index == GENIE_BTN_SAVE_DATE) {
            objTIME.setDate(settingsDatetime.day, settingsDatetime.mon, settingsDatetime.year);
          } else if(Event.reportObject.index == GENIE_BTN_SAVE_TIME) {
            objTIME.setTime(settingsDatetime.hour, settingsDatetime.min, 0);
          } else if(Event.reportObject.index == GENIE_BTN_SAVE_WDAY) {
            objTIME.setWday(settingsDatetime.wday);
          } else if(Event.reportObject.index == GENIE_BTN_SAVE_TIMEOUT) {
            objLCD.setTimeout(settingsTimeout);
          }
          genieWriteObject(GENIE_OBJ_STRINGS, GENIE_STR_INFO_TIME, Event.reportObject.index - GENIE_BTN_SET_TIMEOUT);
        } else if(Event.reportObject.index >= GENIE_BTN_ALARM_SET_START && Event.reportObject.index <= GENIE_BTN_ALARM_SET_STOP) { //Alarm settings - "Set" buttons
          keyboardStart = true;
          int iPos = keyboardString.indexOf('.');
          if(iPos > -1) {
            iValue = keyboardString.substring(iPos+1, iPos+3).toInt();
            if(iValue > 0 && keyboardString.substring(iPos+1, iPos+3).length() == 1) {
              iValue*= 10;
            }
            iValue+= (keyboardString.substring(0, iPos).toInt()*100);
          } else {
            iValue = keyboardString.toInt()*100;
          }
          if(iValue < 0) {
            iValue = 0;
          } else if(iValue > 4999) {
            iValue = 4999; //49.99
          }
          sprintf(row, "%2d.%02d\0", (int)(iValue/100), iValue%100);
          if(Event.reportObject.index == GENIE_BTN_ALARM_SET_START) {
            settingsAlarm.start = iValue;
            genieWriteStr(GENIE_STR_ALARM_START, row);
          } else if(Event.reportObject.index == GENIE_BTN_ALARM_SET_STOP) {
            settingsAlarm.stop = iValue;
            genieWriteStr(GENIE_STR_ALARM_STOP, row);
          }
        } else if(Event.reportObject.index == GENIE_BTN_SAVE_ALARM) { //Alarm settings - "Save" button
          objRELAY.writeRelayAlarm(settingsRelay, &settingsAlarm);
          genieWriteObject(GENIE_OBJ_STRINGS, GENIE_STR_INFO_ALARM, 1);
        } else if(Event.reportObject.index >= GENIE_BTN_TIMER1_SET_START && Event.reportObject.index <= GENIE_BTN_TIMER4_SET_STOP) { //Timer settings - "Set" buttons
          keyboardStart = true;
          int iPos = keyboardString.indexOf(':');
          if(iPos > -1) {
            iValue = keyboardString.substring(0, iPos).toInt()*60 + keyboardString.substring(iPos+1, iPos+3).toInt();
          } else {
            iValue = keyboardString.toInt();
            if(iValue < 24) {
              iValue*= 60;
            }
          }
          if(iValue < 0) {
            iValue = 0;
          } else if(iValue > 1439) {
            iValue = 1439; //23:59
          }
          sprintf(row, "%2d:%02d\0", (int)(iValue/60), iValue%60);
          if(Event.reportObject.index == GENIE_BTN_TIMER1_SET_START) {
            settingsTimer[0].from = iValue;
            genieWriteStr(GENIE_STR_TIMER1_START, row);
          } else if(Event.reportObject.index == GENIE_BTN_TIMER1_SET_STOP) {
            settingsTimer[0].to = iValue;
            genieWriteStr(GENIE_STR_TIMER1_STOP, row);
          } else if(Event.reportObject.index == GENIE_BTN_TIMER2_SET_START) {
            settingsTimer[1].from = iValue;
            genieWriteStr(GENIE_STR_TIMER2_START, row);
          } else if(Event.reportObject.index == GENIE_BTN_TIMER2_SET_STOP) {
            settingsTimer[1].to = iValue;
            genieWriteStr(GENIE_STR_TIMER2_STOP, row);
          } else if(Event.reportObject.index == GENIE_BTN_TIMER3_SET_START) {
            settingsTimer[2].from = iValue;
            genieWriteStr(GENIE_STR_TIMER3_START, row);
          } else if(Event.reportObject.index == GENIE_BTN_TIMER3_SET_STOP) {
            settingsTimer[2].to = iValue;
            genieWriteStr(GENIE_STR_TIMER3_STOP, row);
          } else if(Event.reportObject.index == GENIE_BTN_TIMER4_SET_START) {
            settingsTimer[3].from = iValue;
            genieWriteStr(GENIE_STR_TIMER4_START, row);
          } else if(Event.reportObject.index == GENIE_BTN_TIMER4_SET_STOP) {
            settingsTimer[3].to = iValue;
            genieWriteStr(GENIE_STR_TIMER4_STOP, row);
          }
        } else if(Event.reportObject.index >= GENIE_BTN_SAVE_TIMER1 && Event.reportObject.index <= GENIE_BTN_SAVE_TIMER4) { //Timer settings - "Save" buttons
          objRELAY.writeRelayTimer(settingsRelay, Event.reportObject.index - GENIE_BTN_SAVE_TIMER1, &settingsTimer[Event.reportObject.index - GENIE_BTN_SAVE_TIMER1]);
          genieWriteObject(GENIE_OBJ_STRINGS, GENIE_STR_INFO_TIMER, Event.reportObject.index - GENIE_BTN_TIMER4_SET_STOP);
        } else if(Event.reportObject.index >= GENIE_BTN_CAL1_SET_REF && Event.reportObject.index <= GENIE_BTN_CAL3_SET_REF) { //Calibration - "Set" buttons
          keyboardStart = true;
          int iPos = keyboardString.indexOf('.');
          if(calibration == GENIE_CALIBRATION_ORP) {
            iValue = keyboardString.toInt();
          } else if(iPos > -1) {
            iValue = keyboardString.substring(iPos+1, iPos+3).toInt();
            if(iValue > 0 && keyboardString.substring(iPos+1, iPos+3).length() == 1) {
              iValue*= 10;
            }
            iValue+= (keyboardString.substring(0, iPos).toInt()*100);
          } else {
            iValue = keyboardString.toInt()*100;
          }
          if(calibration == GENIE_CALIBRATION_TEMP) {
            if(iValue < 0) {
              iValue = 0;
            } else if(iValue > 4999) {
              iValue = 4999; //temerature 49.99°C
            }
            tempCalPoint[Event.reportObject.index - GENIE_BTN_CAL1_SET_REF].refValue = (float)iValue/100.00;
            sprintf(row, "%2d.%02d\0", (int)tempCalPoint[Event.reportObject.index - GENIE_BTN_CAL1_SET_REF].refValue, (int)(tempCalPoint[Event.reportObject.index - GENIE_BTN_CAL1_SET_REF].refValue*100)%100);
          } else if(calibration == GENIE_CALIBRATION_PH) {
            if(iValue < 0) {
              iValue = 0;
            } else if(iValue > 1400) {
              iValue = 1400; //pH 14.00
            }
            phCalPoint[Event.reportObject.index - GENIE_BTN_CAL1_SET_REF].refValue = (float)iValue/100.00;
            sprintf(row, "%2d.%02d\0", (int)phCalPoint[Event.reportObject.index - GENIE_BTN_CAL1_SET_REF].refValue, (int)(phCalPoint[Event.reportObject.index - GENIE_BTN_CAL1_SET_REF].refValue*100)%100);
          } else if(calibration == GENIE_CALIBRATION_ORP) {
            if(iValue < -1999) {
              iValue = -1999;
            } else if(iValue > 1999) {
              iValue = 1999;
            }
            orpCalPoint[Event.reportObject.index - GENIE_BTN_CAL1_SET_REF].refValue = iValue;
            sprintf(row, "%+5d\0", orpCalPoint[Event.reportObject.index - GENIE_BTN_CAL1_SET_REF].refValue);
          }
          if(Event.reportObject.index == GENIE_BTN_CAL1_SET_REF) {
            genieWriteStr(GENIE_STR_CAl1_REF_NEW, row);
          } else if(Event.reportObject.index == GENIE_BTN_CAL2_SET_REF) {
            genieWriteStr(GENIE_STR_CAl2_REF_NEW, row);
          } else if(Event.reportObject.index == GENIE_BTN_CAL3_SET_REF) {
            genieWriteStr(GENIE_STR_CAl3_REF_NEW, row);
          }
        } else if(Event.reportObject.index >= GENIE_BTN_CAL1_READ_ACT && Event.reportObject.index <= GENIE_BTN_CAL3_READ_ACT) { //Calibration - "Read" buttons
          displayForm(GENIE_FORM_CALIBRATION_SHOW, Event.reportObject.index);
        } else if(Event.reportObject.index == GENIE_BTN_SAVE_CALIBRATION) { //Calibration - "Save" button
          if(calibration == GENIE_CALIBRATION_TEMP) {
            for(uint8_t i = 0; i < AQUA_TEMP_CALIBRATE_POINTS; i++) {
              objTEMP.calibration(i, &tempCalPoint[i]);
            }
          } else if(calibration == GENIE_CALIBRATION_PH) {
            for(uint8_t i = 0; i < AQUA_PH_CALIBRATE_POINTS; i++) {
              objPH.calibration(i, &phCalPoint[i]);
            }
          } else if(calibration == GENIE_CALIBRATION_ORP) {
            for(uint8_t i = 0; i < AQUA_ORP_CALIBRATE_POINTS; i++) {
              objORP.calibration(i, &orpCalPoint[i]);
            }
          }
          genieWriteObject(GENIE_OBJ_STRINGS, GENIE_STR_INFO_CALIBRATION, 1);
        }
      }
    } else if(Event.reportObject.object == GENIE_OBJ_USERBUTTON) {
      if(Event.reportObject.index == GENIE_BTN_WAKEUP) {
        displayForm(GENIE_FORM_MAIN);
        genieWriteContrast(1);
      }
    } else if(Event.reportObject.object == GENIE_OBJ_KEYBOARD) {
      uint8_t keyboardValue = genieGetEventData(&Event);
      if(keyboardValue == GENIE_KEYBOARD_CLEAR_KEY) {
          keyboardStart = true;
          sprintf(row, "%5d\0", 0);
          if(Event.reportObject.index == GENIE_KEYBOARD_TIME) {
            genieWriteStr(GENIE_STR_TIME_KEY, row);
          } else if(Event.reportObject.index == GENIE_KEYBOARD_ALARM) {
            genieWriteStr(GENIE_STR_ALARM_KEY, row);
          } else if(Event.reportObject.index == GENIE_KEYBOARD_TIMER) {
            genieWriteStr(GENIE_STR_TIMER_KEY, row);
          } else if(Event.reportObject.index == GENIE_KEYBOARD_CALIBRATION) {
            genieWriteStr(GENIE_STR_CAL_KEY, row);
          }
      } else {
        if(keyboardStart) {
          keyboardStart = false;
          keyboardString = "";
        }
        if((Event.reportObject.index > GENIE_KEYBOARD_TIME && keyboardString.length() < 5) || (Event.reportObject.index == GENIE_KEYBOARD_TIME && keyboardString.length() < 4)) {
          keyboardString += char(keyboardValue);
        }
        char tmp[6];
        keyboardString.toCharArray(tmp, 6);
        sprintf(row, "%5s\0", tmp);
        if(Event.reportObject.index == GENIE_KEYBOARD_TIME) {
          genieWriteStr(GENIE_STR_TIME_KEY, row);
        } else if(Event.reportObject.index == GENIE_KEYBOARD_ALARM) {
          genieWriteStr(GENIE_STR_ALARM_KEY, row);
        } else if(Event.reportObject.index == GENIE_KEYBOARD_TIMER) {
          genieWriteStr(GENIE_STR_TIMER_KEY, row);
        } else if(Event.reportObject.index == GENIE_KEYBOARD_CALIBRATION) {
          genieWriteStr(GENIE_STR_CAL_KEY, row);
        }
      }
    }
  }
}

/*** write actual value into calibration screen ***/
void writeActualValue() {
  char row[8];
  if(calibration == GENIE_CALIBRATION_TEMP) {
    float temp = objTEMP.getTemp(1);
    sprintf(row, "%2d.%02d\0", (int)temp, (int)(temp*100)%100);
  } else if(calibration == GENIE_CALIBRATION_PH) {
    float pH = objPH.getPH(1);
    sprintf(row, "%2d.%02d\0", (int)pH, (int)(pH*100)%100);
  } else if(calibration == GENIE_CALIBRATION_ORP) {
    sprintf(row, "%+5d\0", objORP.getORP(1));
  }
  genieWriteStr(GENIE_STR_ACTUAL_VALUE, row);
}

/*** check all what need and set all what need ;-) ***/
void checkAll() {
  AQUA_datetime datetimeStruct = objTIME.getDateTime();
  float temp = objTEMP.getTemp();
  float pH = objPH.getPH();
  int orp = objORP.getORP();

  checkRelay(datetimeStruct.hour*60 + datetimeStruct.min, temp, pH);
  if(wakeupLCD == 1 && objLCD.getActualForm() == GENIE_FORM_MAIN) {
    printStatusToLCD(&datetimeStruct, temp, pH, orp);
  }
  if(AQUA_DEBUG_MODE_ON == 1) {
    printStatusToSerial(&datetimeStruct, temp, pH, orp);
  }

  if(wakeupLCD == 1 && objLCD.getActualForm() == GENIE_FORM_MAIN && (millis() - inactivity)/1000 > (objLCD.getTimeout())) {
    displayForm(GENIE_FORM_SLEEP);
  }
}

/*** set relays to correct status ***/
void checkRelay(uint16_t tstamp, float temp, float pH) {
  bool relayStatus;
  uint8_t i;

  for(i = 0; i < relayCount; i++) {
    if(objRELAY.isManual(i)) {
      relayStatus = objRELAY.getByManual(i);
    } else if(i == AQUA_TEMP_RELAY_COOLING || i == AQUA_TEMP_RELAY_HEATING) {
      relayStatus = objRELAY.getByAlarm(i, round(temp*100));
    } else if(i == AQUA_PH_RELAY_REDUCTION || i == AQUA_PH_RELAY_INCREASE) {
      relayStatus = objRELAY.getByAlarm(i, round(pH*100));
    } else if(i >= AQUA_RELAY_ALARMS) {
      relayStatus = objRELAY.getByTimer(i, tstamp);
    } else {
      relayStatus = objRELAY.get(i);
    }
    if(relayStatus != objRELAY.get(i)) {
      objRELAY.set(i, relayStatus);
    }
  }
}

/*** print status to LCD ***/
void printStatusToLCD(AQUA_datetime *datetimeStruct, float temp, float pH, int orp) {
  char row[17];
  if(datetimeStruct->day > 0) {
    sprintf(row, "%02d.%02d.%4d %02d:%02d\0", datetimeStruct->day, datetimeStruct->mon, datetimeStruct->year, datetimeStruct->hour, datetimeStruct->min);
    genieWriteStr(GENIE_STR_TIME, row);
  } else {
    genieWriteStr(GENIE_STR_TIME, "OFF");
  }
  if(temp > 0) {
    sprintf(row, "%2d.%02d\0", (int)temp, (int)(temp*100)%100);
    genieWriteStr(GENIE_STR_TEMP, row);
  } else {
    genieWriteStr(GENIE_STR_TEMP, "OFF");
  }
  if(pH > 0) {
    sprintf(row, "%2d.%02d\0", (int)pH, (int)(pH*100)%100);
    genieWriteStr(GENIE_STR_PH, row);
  } else {
    genieWriteStr(GENIE_STR_PH, "OFF");
  }
  if(orp != 0) {
    sprintf(row, "%+5d\0", orp);
    genieWriteStr(GENIE_STR_ORP, row);
  } else {
    genieWriteStr(GENIE_STR_ORP, "OFF");
  }

  for(uint8_t i = 0; i < relayCount; i++) {
    genieWriteObject(GENIE_OBJ_LED, i, (objRELAY.get(i) == AQUA_RELAY_ON ? 1 : 0));
  }

  delay(100);
}

/*** print status to serial ***/
void printStatusToSerial(AQUA_datetime *datetimeStruct, float temp, float pH, int orp) {
  Serial.println("");Serial.print("Date: ");
  Serial.print(datetimeStruct->day);Serial.print(".");
  Serial.print(datetimeStruct->mon);Serial.print(".");
  Serial.print(datetimeStruct->year);Serial.print(" - ");Serial.println(datetimeStruct->wday);
  Serial.print("Time: ");
  Serial.print(datetimeStruct->hour);Serial.print(":");
  Serial.print(datetimeStruct->min);Serial.print(":");
  Serial.println(datetimeStruct->sec);

  Serial.print("Temperature: "); Serial.println(temp);
  Serial.print("pH: "); Serial.println(pH);
  Serial.print("ORP: "); Serial.println(orp);

  for(uint8_t i = 0; i < relayCount; i++) {
    Serial.print("Relay");Serial.print(i+1);Serial.print(": ");Serial.println(objRELAY.get(i) == AQUA_RELAY_ON ? "ON" : "OFF");
  }

  delay(100);
}

/*** setup function ***/
void setup() {
  analogReference(EXTERNAL); //use AREF for reference voltage (connect 3.3V pin to AREF pin)

  if(AQUA_DEBUG_MODE_ON == 1) {
    Serial.begin(115200);
    Serial.println("Initialization...");
  }

  if(AQUA_DEBUG_MODE_ON == 1) {
    Serial.println("Time object initialization...");
  }
  objTIME.init(AQUA_TIME_DATA_PIN, AQUA_TIME_CLOCK_PIN, DS_TYPE_3231, true, 1); //dataPin(SDA), clockPin(SCLK), DS3231, use DST, time zone (+1 = SEC)

  if(AQUA_DEBUG_MODE_ON == 1) {
    Serial.println("Relay object initialization...");
  }
  objRELAY.init(AQUA_RELAY_FIRST_PIN, AQUA_RELAY_ALARMS, AQUA_RELAY_TIMERS, AQUA_RELAY_TIMER_PARTS, AQUA_RELAY_ALARMS_ADDR, AQUA_RELAY_TIMERS_ADDR); //first pin number, number of alarms, number of timers, number of timer parts, alarms address, timers address

  if(AQUA_DEBUG_MODE_ON == 1) {
    Serial.println("Temperature object initialization...");
  }
  objTEMP.init(AQUA_TEMP_DQ_PIN, AQUA_TEMP_CALIBRATE_POINTS, AQUA_TEMP_CALIBRATE_ADDR); //pin(DQ) - a 4.7K resistor is necessary, number of calibrating points, calibrate address

  if(AQUA_DEBUG_MODE_ON == 1) {
    Serial.println("pH object initialization...");
  }
  objPH.init(AQUA_PH_VOUT_PIN, AQUA_PH_VOCM_PIN, AQUA_PH_CALIBRATE_POINTS, AQUA_PH_CALIBRATE_ADDR, AQUA_ANALOG_VREF, AQUA_ADC_BIT_RESOLUTION); //VOUT pin, VOCM pin, number of calibrating points, calibrate address, vRef, ADC bit resolution

  if(AQUA_DEBUG_MODE_ON == 1) {
    Serial.println("ORP object initialization...");
  }
  objORP.init(AQUA_ORP_VOUT_PIN, AQUA_ORP_VOCM_PIN, AQUA_ORP_CALIBRATE_POINTS, AQUA_ORP_CALIBRATE_ADDR, AQUA_ANALOG_VREF, AQUA_ADC_BIT_RESOLUTION); //VOUT pin, VOCM pin, number of calibrating points, calibrate address, vRef, ADC bit resolution

  if(AQUA_DEBUG_MODE_ON == 1) {
    Serial.print("Setup LCD:");Serial.print(AQUA_LCD_SERIAL);Serial.print(":");Serial.println(AQUA_LCD_SERIAL_SPEED);
  }
  switch(AQUA_LCD_SERIAL) {
    case 3:
      genieBegin(GENIE_SERIAL_3, AQUA_LCD_SERIAL_SPEED);
      break;
    case 2:
      genieBegin(GENIE_SERIAL_2, AQUA_LCD_SERIAL_SPEED);
      break;
    case 1:
    default:
      genieBegin(GENIE_SERIAL_1, AQUA_LCD_SERIAL_SPEED);
      break;
  }
  genieAttachEventHandler(lcdEventHandler);
  pinMode(AQUA_LCD_RESET_PIN, OUTPUT); //Set D4 on Arduino to Output
  digitalWrite(AQUA_LCD_RESET_PIN, LOW); //Reset the Display via D4
  delay(100);
  digitalWrite(AQUA_LCD_RESET_PIN, HIGH); //unReset the Display via D4
  delay(4000); //let the display start up
  displayForm(GENIE_FORM_MAIN);
  genieWriteContrast(1);
  objLCD.init(AQUA_LCD_TIMEOUT_ADDR);

  if(AQUA_DEBUG_MODE_ON == 1) {
    Serial.println("Setup Watchdog Timer...");
  }
  setupWDT();
  pinMode(AQUA_WAKEUP_BUTTON, INPUT);
  digitalWrite(AQUA_WAKEUP_BUTTON, HIGH);
  pinMode(AQUA_ACTIVITY_LED, OUTPUT);
  digitalWrite(AQUA_ACTIVITY_LED, HIGH);

  wakeup = 1;
  wakeupLCD = 1;
  if(AQUA_DEBUG_MODE_ON == 1) {
    Serial.println("Ready for use!");
  }
  delay(100);
}

/*** loop function ***/
void loop() {
  genieDoEvents();
  if(wakeupLCD == 1 && calibration > GENIE_CALIBRATION_OFF) {
    if(calWV < millis() - 500) {
      writeActualValue();
      calWV = millis();
    }
  }
  if(wakeupButton == 1) {
    wakeupButton = 0;
    wakeup = 1;
    inactivity = millis();
    if(AQUA_DEBUG_MODE_ON == 1) {
      Serial.println("");Serial.println("pin2Interrupt...");
    }
    if(wakeupLCD == 0) {
      displayForm(GENIE_FORM_MAIN);
      genieWriteContrast(1);
      wakeupLCD = 1;
    }
  }
  if(wakeup == 1) {
    wakeup = 0;
    checkAll();
    if(wakeupLCD == 0) {
      enterSleep();
    }
  }
}
