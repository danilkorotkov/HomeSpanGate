struct GateDoor : Service::Door{
  SpanCharacteristic *CurrentPosition;
  SpanCharacteristic *TargetPosition;
  SpanCharacteristic *PositionState;
  volatile uint32_t CycleTimeBegin = millis();
  uint32_t PollTimeout = 2000;
  volatile uint8_t iterator = 100;
    
  GateDoor() : Service::Door(){
    CurrentPosition = new Characteristic::CurrentPosition(50);
    TargetPosition = new Characteristic::TargetPosition(50);
    PositionState = new Characteristic::PositionState(2);
  }
  void loop(){
    if ( (millis() - CycleTimeBegin) > PollTimeout ) {
      ;
    }
  }    
};
