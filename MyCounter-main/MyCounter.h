#ifndef _MYCOUNTER_H_
#define _MYCOUNTER_H_

#include <Arduino.h>

class MyCounter {
private :
  int ledPin;
  int btnPin;
  int timeSecond;
  bool btnPinActive;
  bool ledPinActive;
  bool blink;
  
  uint32_t _next;

public:
  MyCounter(const int btnPin,
            const int ledPin,
            const int timeSecond = 10,
            const bool btnPinActive = LOW,
            const bool ledPinActive = LOW);

  ~MyCounter();
  
  void setBlink(bool blink);
  void setTimeSecond(const int timeSecond);
  void loop();

};

#endif
