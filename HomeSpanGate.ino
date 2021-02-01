////////////////////////////////////////////////////////////
//                                                        //
//    HomeSpan: A HomeKit implementation for the ESP32    //
//    ------------------------------------------------    //
//                                                        //
//        Sliding gate                                    //
//           TODO Lock Service                            //
//                                                        //
////////////////////////////////////////////////////////////
#include "HomeSpan.h" 
#include "GateClass.h" 

void setup() {
  Serial.begin(115200);

  homeSpan.begin(Category::GarageDoorOpeners,"Sl Gate");
  homeSpan.setApSSID("Sl-Gate-AP");
  
  new SpanAccessory(); 
  
    new Service::AccessoryInformation(); 
      new Characteristic::Name("Gate"); 
      new Characteristic::Manufacturer("Danil"); 
      new Characteristic::SerialNumber("0000001"); 
      new Characteristic::Model("3 key model"); 
      new Characteristic::FirmwareRevision("0.1"); 
      new Characteristic::Identify();            
      
    new Service::HAPProtocolInformation();      
      new Characteristic::Version("1.1.0"); 
  //SL_GATE(int OpenPin, int ClosePin, int StopPin);
  new SL_GATE();
  
}

void loop() {
  homeSpan.poll();
}
