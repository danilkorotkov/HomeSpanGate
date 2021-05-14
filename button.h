#ifndef BUTTON_H
#define BUTTON_H

#include "HomeSpan.h" 
#define PRESS 1

void IRAM_ATTR onBtnTimer();

struct SwLock;

struct SwLock: Service::Switch{
  SpanCharacteristic *On;

  int SwPin = 21;

  SwLock();
  boolean update();
  void loop();
  void initSwTimer();
};
#endif /BUTTON_H/
