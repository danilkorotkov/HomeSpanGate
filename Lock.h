#define CURRENT_LOCK_STATE_OPEN    0
#define CURRENT_LOCK_STATE_CLOSED  1

#define TARGET_LOCK_STATE_OPEN    0
#define TARGET_LOCK_STATE_CLOSED  1

volatile bool isDoorTimered = false;
hw_timer_t * DoorTimer = NULL;
portMUX_TYPE LocktimerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onDoorTimer(){
  portENTER_CRITICAL_ISR(&LocktimerMux);
  isDoorTimered = true;
  portEXIT_CRITICAL_ISR(&LocktimerMux);
};

void initDoorTimer(){
  DoorTimer = timerBegin(1, 80, true);
  timerAttachInterrupt(DoorTimer, &onDoorTimer, true);
  timerAlarmWrite(DoorTimer, 1000000, false);
  timerAlarmEnable(DoorTimer);  
};


struct DoorLock : Service::LockMechanism{
  SpanCharacteristic *LockCurrentState;
  SpanCharacteristic *LockTargetState;

  int LockPin = 16;
 
  DoorLock() : Service::LockMechanism(){
    LockCurrentState  = new Characteristic::LockCurrentState(CURRENT_LOCK_STATE_CLOSED);
    LockTargetState   = new Characteristic::LockTargetState (TARGET_LOCK_STATE_CLOSED);
    pinMode(this->LockPin,OUTPUT); 
    digitalWrite(this->LockPin,LOW);

  }

 // Finally, we over-ride the default update() method Note update() returns type boolean
  boolean update(){            

    if(LockTargetState->getNewVal()==TARGET_LOCK_STATE_OPEN &&
       LockCurrentState->getVal() != CURRENT_LOCK_STATE_OPEN){ 
                                                              
                                                              
      LOG1("-----------Opening Door----------\n");
      LockCurrentState->setVal(CURRENT_LOCK_STATE_OPEN);   
          
      digitalWrite(LockPin,HIGH);
   
      initDoorTimer();
    }
    return(true);                                   // return true to indicate the update was successful (otherwise create code to return false if some reason you could not turn on the LED)
  
  } // update
  
  void loop(){
    if (isDoorTimered){
        portENTER_CRITICAL(&LocktimerMux);
        timerEnd(DoorTimer);
        DoorTimer = NULL;
        isDoorTimered = false;
        portEXIT_CRITICAL(&LocktimerMux);        
        LOG1("----------Timer door deinited----------\n");
        digitalWrite(LockPin,LOW);
        LockCurrentState->setVal(CURRENT_LOCK_STATE_CLOSED);
        LockTargetState->setVal(TARGET_LOCK_STATE_CLOSED);
        
    }
  }
};
