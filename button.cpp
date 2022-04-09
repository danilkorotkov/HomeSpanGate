#include "button.h" 

extern bool StopStatus;

hw_timer_t * SwTimer = NULL;
portMUX_TYPE SwTimerMux = portMUX_INITIALIZER_UNLOCKED;
bool isSwTimered = false;
uint32_t stopTimeout = 0;

void IRAM_ATTR onSwTimer(){
  portENTER_CRITICAL_ISR(&SwTimerMux);
  isSwTimered = true;
  portEXIT_CRITICAL_ISR(&SwTimerMux);
};

void SwLock :: initSwTimer(){
  SwTimer = timerBegin(2, 80, true);
  timerAttachInterrupt(SwTimer, &onSwTimer, true);
  timerAlarmWrite(SwTimer, 1000000, false);
  timerAlarmEnable(SwTimer);  
};
 
SwLock :: SwLock() : Switch(){
    On  = new Characteristic::On();

    pinMode(SwPin,OUTPUT); 
    digitalWrite(SwPin,LOW);

}

boolean SwLock :: update(){            
  if(On->getNewVal() == PRESS){ 
    LOG1("-----------SINGLE_PRESS----------\n");   
    On->setVal(1);
    digitalWrite(SwPin,HIGH);
    StopStatus = true;
    //initSwTimer();
    stopTimeout = millis();
  }
  return(true);                                   // return true to indicate the update was successful (otherwise create code to return false if some reason you could not turn on the LED)
  
} // update
  
void SwLock ::loop(){
/*  if (isSwTimered){
    portENTER_CRITICAL(&SwTimerMux);
    timerEnd(SwTimer);
    SwTimer = NULL;
    isSwTimered = false;
    portEXIT_CRITICAL(&SwTimerMux);        
    LOG1("----------Stop deinited----------\n");
    digitalWrite(SwPin,LOW);
    On->setVal(0);
  }*/
  if ( (millis() - stopTimeout) > 1000 && On->getVal() == 1) {
           
    LOG1("----------Stop deinited----------\n");
    digitalWrite(SwPin,LOW);
    On->setVal(0);
  }

}
