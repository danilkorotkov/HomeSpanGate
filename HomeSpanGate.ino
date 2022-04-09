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
#include "SlGate.h" 
#include "Lock.h"
TaskHandle_t h_HK_poll;

void HK_poll(void * pvParameters){

  for (;;){
    homeSpan.poll();
  }   //loop
}     //task

void setup() {
  Serial.begin(115200);

  //used pins JmpPin13 LockPin16 ObS17 OpenPin18 ClosePin19 StopPin21 OpS22 ClS23 ControlPin0 StatusPin2  
  homeSpan.setApSSID("Sl-Gate-AP");
  homeSpan.setApPassword("");
  homeSpan.setControlPin(0);
  homeSpan.setStatusPin(2);
  homeSpan.setLogLevel(1);

  homeSpan.setSketchVersion("2.1.1");
  homeSpan.enableOTA();
  
  homeSpan.begin(Category::GarageDoorOpeners,"Sl Gate");
  
  new SpanAccessory(); 
  
    new Service::AccessoryInformation(); 
      new Characteristic::Name("Gate"); 
      new Characteristic::Manufacturer("Danil"); 
      new Characteristic::SerialNumber("0000001"); 
      new Characteristic::Model("2 key model"); 
      new Characteristic::FirmwareRevision("2.0.4"); 
      new Characteristic::Identify();            
      
    new Service::HAPProtocolInformation();      
      new Characteristic::Version("1.1.0"); 
  
    new SL_GATE();

    new SpanAccessory(); 
  
    new Service::AccessoryInformation(); 
      new Characteristic::Name("Калитка"); 
      new Characteristic::Manufacturer("Danil"); 
      new Characteristic::SerialNumber("0000002"); 
      new Characteristic::Model("3 key model"); 
      new Characteristic::FirmwareRevision("0.0.5"); 
      new Characteristic::Identify();            
      
    new Service::HAPProtocolInformation();      
      new Characteristic::Version("1.1.0"); 

    new DoorLock();

    xTaskCreatePinnedToCore(
                    HK_poll,    /* Task function. */
                    "HK_poll",  /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &h_HK_poll, /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */  
  
    delay(1000);
}

void loop() {
  //homeSpan.poll();
}
