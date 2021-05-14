#ifndef BUTTON_H
#define BUTTON_H

#include "HomeSpan.h" 
#define SINGLE_PRESS 0

void IRAM_ATTR onBtnTimer();

struct SwLock;

struct SwLock: Service::StatelessProgrammableSwitch{
  SpanCharacteristic *ProgrammableSwitchEvent;

  int SwPin = 21;

  SwLock();
  boolean update();
  void loop();
  void initSwTimer();
};
#endif /BUTTON_H/
