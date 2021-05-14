#include "button.h" 

hw_timer_t * SwTimer = NULL;
portMUX_TYPE SwTimerMux = portMUX_INITIALIZER_UNLOCKED;
bool isSwTimered = false;

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
 
SwLock :: SwLock() : Service::StatelessProgrammableSwitch(){
    ProgrammableSwitchEvent  = new Characteristic::ProgrammableSwitchEvent();

    pinMode(SwPin,OUTPUT); 
    digitalWrite(SwPin,LOW);

}

boolean SwLock :: update(){            
  if(ProgrammableSwitchEvent->getNewVal() == SINGLE_PRESS){ 
    LOG1("-----------SINGLE_PRESS----------\n");   
    
    digitalWrite(SwPin,HIGH);
    initSwTimer();
  }
  return(true);                                   // return true to indicate the update was successful (otherwise create code to return false if some reason you could not turn on the LED)
  
} // update
  
void SwLock ::loop(){
  if (isSwTimered){
    portENTER_CRITICAL(&SwTimerMux);
    timerEnd(SwTimer);
    SwTimer = NULL;
    isSwTimered = false;
    portEXIT_CRITICAL(&SwTimerMux);        
    LOG1("----------Stop deinited----------\n");
    digitalWrite(SwPin,LOW);
  }
}
