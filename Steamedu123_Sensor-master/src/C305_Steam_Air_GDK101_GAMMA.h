#ifndef SteamGDK101_H
#define SteamGDK101_H

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "gdk101_i2c.h"

class SteamGDK101
{

public:
  SteamGDK101(void){};
  ~SteamGDK101(void){};

  void begin();
  void read();
  void display();

  // 새로 추가된 getter 메서드
  float get10minAvg() {
    return _gdk101.mea_10min_avg;
  }

  float get1minAvg() {
    return _gdk101.mea_1min_avg;
  }

  uint8_t getMeasuringTimeMin() {
    return _gdk101.mea_time_min;
  }

  uint8_t getMeasuringTimeSec() {
    return _gdk101.mea_time_sec;
  }

private:
  GDK101_I2C _gdk101;

  float avg10min;
  float avg1min;
  float fw_vers;
  int _status;

  bool vibration;
  byte mea_min;
  byte mea_sec;

  //output
  char out_buff[48];

  void displayReset(bool s);
  void displayStatus(int s);
  void displayVibration(bool s);

  float convert_uSvTomSv(float sv);
  void _displayAirCondition_RN(float uSv);
};

#endif
