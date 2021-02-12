struct DoorLock : Service::LockMechanism{
  SpanCharacteristic *LockCurrentState;
  SpanCharacteristic *LockTargetState;
  SpanCharacteristic *PositionState;
  volatile uint32_t CycleTimeBegin = millis();
  uint32_t PollTimeout = 2000;
  volatile uint8_t iterator = 100;
    
  DoorLock() : Service::LockMechanism(){
    LockCurrentState = new Characteristic::LockCurrentState(1);
    LockTargetState = new Characteristic::LockTargetState(1);

  }
  void loop(){
    if ( (millis() - CycleTimeBegin) > PollTimeout ) {
      ;
    }
  }    
};
