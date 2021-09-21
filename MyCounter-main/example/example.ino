#include "MyCounter.h"

#define PIN_BUTTON 0
#define PIN_LED 16

MyCounter counter(PIN_BUTTON, PIN_LED);

void setup() {
  Serial.begin(115200);

  counter.setTimeSecond(5);
}

void loop() {
  counter.loop();

}
