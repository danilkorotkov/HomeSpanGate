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
  homeSpan.setApPassword("");
  homeSpan.setControlPin(0);
  homeSpan.setStatusPin(2);
  homeSpan.setLogLevel(1);
  homeSpan.setHostNameSuffix("v0.2");
  
  new SpanAccessory(); 
  
    new Service::AccessoryInformation(); 
      new Characteristic::Name("Gate"); 
      new Characteristic::Manufacturer("Danil"); 
      new Characteristic::SerialNumber("0000001"); 
      new Characteristic::Model("3 key model"); 
      new Characteristic::FirmwareRevision("0.2"); 
      new Characteristic::Identify();            
      
    new Service::HAPProtocolInformation();      
      new Characteristic::Version("1.1.0"); 
  
    new SL_GATE();
  
}

void loop() {
  homeSpan.poll();
}
