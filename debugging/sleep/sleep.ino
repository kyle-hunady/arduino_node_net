#include "LowPower.h"

void setup()
{
}

void loop() {
    digitalWrite(LED_BUILTIN,HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN,LOW);
    goToSleep(16);
}

void goToSleep(int duration) {
  int cycles = duration / 8;
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);  
}
