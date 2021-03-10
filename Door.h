struct GateDoor : Service::Door{
  SpanCharacteristic *CurrentPosition;
  SpanCharacteristic *TargetPosition;
  SpanCharacteristic *PositionState;
  SpanService *gate;
 
  /*  0 ”Going to the minimum value specified in metadata”
      1 ”Going to the maximum value specified in metadata”
      2 ”Stopped”
      3-255 ”Reserved”*/
  
  uint32_t CycleTimeBegin = millis();
  uint32_t PollTimeout = 2000;
    
  GateDoor(SL_GATE* gate) : Service::Door(){
    CurrentPosition = new Characteristic::CurrentPosition(50);
    TargetPosition  = new Characteristic::TargetPosition(50);
    PositionState   = new Characteristic::PositionState(2);
    this->gate=gate;
    
  }
  void loop(){
    if ( (millis() - CycleTimeBegin) > PollTimeout ) {
      gate->CurrentDoorState-> setVal( !gate->CurrentDoorState->getVal() );
    }
  }    
};
