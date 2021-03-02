struct Battery : Service::BatteryService{
  SpanCharacteristic *BatteryLevel;
  SpanCharacteristic *ChargingState;
  SpanCharacteristic *StatusLowBattery;
  volatile uint32_t CycleTimeBegin = millis();
  uint32_t PollTimeout = 2000;
  volatile uint8_t iterator = 100;
    
  Battery() : Service::BatteryService(){
    BatteryLevel = new Characteristic::BatteryLevel(100);
    ChargingState = new Characteristic::ChargingState(0);
    StatusLowBattery = new Characteristic::StatusLowBattery(0);
    
  }
  void loop(){
    if ( (millis() - CycleTimeBegin) > PollTimeout ) {
      BatteryLevel->setVal(iterator);
      CycleTimeBegin = millis();
      iterator--;
      if (iterator == 0) {iterator = 100;}
    }    
  }
};
