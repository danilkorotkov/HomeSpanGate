// Possible values for characteristic CURRENT_DOOR_STATE:
#define CURRENT_DOOR_STATE_OPEN    0
#define CURRENT_DOOR_STATE_CLOSED  1
#define CURRENT_DOOR_STATE_OPENING 2
#define CURRENT_DOOR_STATE_CLOSING 3
#define CURRENT_DOOR_STATE_STOPPED 4
#define CURRENT_DOOR_STATE_UNKNOWN 255

#define TARGET_DOOR_STATE_OPEN    0
#define TARGET_DOOR_STATE_CLOSED  1
#define TARGET_DOOR_STATE_UNKNOWN 255
#define SENSOR_CLOSED   0
#define SENSOR_RELEASED 1

////////////////////////////
volatile int ButtonArray[3];
volatile bool isTimered = false;
hw_timer_t * ButtonTimer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
  
struct Sensor {
    const uint8_t PIN;
    bool changed;
};

void IRAM_ATTR isr(void* arg) {
    Sensor* s = static_cast<Sensor*>(arg);
    s->changed = true;
};

void IRAM_ATTR onTimer(){
  portENTER_CRITICAL_ISR(&timerMux);
  isTimered = true;
  portEXIT_CRITICAL_ISR(&timerMux);
};

void initTimer(){
  ButtonTimer = timerBegin(0, 80, true);
  timerAttachInterrupt(ButtonTimer, &onTimer, true);
  timerAlarmWrite(ButtonTimer, 1000000, false);
  timerAlarmEnable(ButtonTimer);  
};

struct SL_GATE : Service::GarageDoorOpener {         // First we create a derived class from the HomeSpan 

  int OpenPin     = 18;                                       // this variable stores the pin number defined 
  int ClosePin    = 19;
  int StopPin     = 21;
  struct Sensor OpSensorPin = {22, false};
  struct Sensor ClSensorPin = {23, false};
  struct Sensor ObSensorPin = {17, false};
  
  uint32_t CycleTimeout = 60000; //60s
  volatile uint32_t CycleTimeBegin;
  
  SpanCharacteristic *CurrentDoorState;              // here we create a generic pointer to a SpanCharacteristic 
  SpanCharacteristic *TargetDoorState;
  SpanCharacteristic *ObstructionDetected;
  SpanCharacteristic *Name;
  
  SL_GATE() : Service::GarageDoorOpener(){

    LOG1("Constructing Gate…\n");
    CurrentDoorState=new Characteristic::CurrentDoorState();// this is where we create the On Characterstic we had previously defined in setup().  Save this in the pointer created above, for use below
    TargetDoorState=new Characteristic::TargetDoorState();
    ObstructionDetected=new Characteristic::ObstructionDetected();
    Name=new Characteristic::Name("Gate");
                         
    pinMode(this->OpenPin,OUTPUT); 
    digitalWrite(this->OpenPin,LOW);
    ButtonArray[0] = this->OpenPin;
                                              
    pinMode(this->ClosePin,OUTPUT);
    digitalWrite(this->ClosePin,LOW);
    ButtonArray[1] = this->ClosePin;  
                      
    pinMode(this->StopPin,OUTPUT);
    digitalWrite(this->StopPin,LOW);
    ButtonArray[2] = this->StopPin;
    
    pinMode(this->ClSensorPin.PIN, INPUT_PULLUP);
    pinMode(this->OpSensorPin.PIN, INPUT_PULLUP);
    pinMode(this->ObSensorPin.PIN, INPUT_PULLUP);
    attachInterruptArg(this->ClSensorPin.PIN, isr, &(this->ClSensorPin), CHANGE);
    attachInterruptArg(this->OpSensorPin.PIN, isr, &(this->OpSensorPin), CHANGE);
    attachInterruptArg(this->ObSensorPin.PIN, isr, &(this->ObSensorPin), CHANGE);         
    //poll current state
    PollCurrentState();
    LOG1("Constructing Gate successful!\n");
    //LOG1(WiFi.localIP());  
  } // end constructor

  void PollCurrentState(){
    if (digitalRead(ClSensorPin.PIN) == SENSOR_CLOSED)        {CurrentDoorState->setVal(CURRENT_DOOR_STATE_CLOSED); TargetDoorState->setVal(TARGET_DOOR_STATE_CLOSED);}
    else if (digitalRead(OpSensorPin.PIN) == SENSOR_CLOSED)   {CurrentDoorState->setVal(CURRENT_DOOR_STATE_OPEN);   TargetDoorState->setVal(TARGET_DOOR_STATE_OPEN);}
    else                                                      {CurrentDoorState->setVal(CURRENT_DOOR_STATE_OPEN);   TargetDoorState->setVal(TARGET_DOOR_STATE_OPEN);}
    if (digitalRead(ObSensorPin.PIN) == SENSOR_CLOSED)        {ObstructionDetected->setVal(true);}
  }

  // Finally, we over-ride the default update() method Note update() returns type boolean
  boolean update(){            

    if(TargetDoorState->getNewVal()==TARGET_DOOR_STATE_OPEN &&
       CurrentDoorState->getVal() != CURRENT_DOOR_STATE_OPENING){ 
                                                              // if the target-state value is set to 0, 
                                                              //HomeKit is requesting the door to be in open position
        LOG1("-----------Opening Gate----------\n");
        CurrentDoorState->setVal(CURRENT_DOOR_STATE_OPENING);   // set the current-state value to 2, which means "opening"
        //ObstructionDetected->setVal(false);                     // clear any prior obstruction detection
      
        digitalWrite(ClosePin,LOW);
        digitalWrite(OpenPin,HIGH);    

        initTimer();
    
    } else if(TargetDoorState->getNewVal()==TARGET_DOOR_STATE_CLOSED &&
              CurrentDoorState->getVal() != CURRENT_DOOR_STATE_CLOSING){
      
        LOG1("----------Closing Gate----------\n");                                 // else the target-state value is set to 1, and HomeKit is requesting the door to be in the closed position
        CurrentDoorState->setVal(CURRENT_DOOR_STATE_CLOSING);   // set the current-state value to 3, which means "closing"         
        //ObstructionDetected->setVal(false);                     // clear any prior obstruction detection
      
        digitalWrite(OpenPin,LOW);
        digitalWrite(ClosePin,HIGH);

        initTimer();                
    } 
    LOG1("----------UpdateOver----------\n");
    CycleTimeBegin = millis();
    return(true);                                   // return true to indicate the update was successful (otherwise create code to return false if some reason you could not turn on the LED)
  
  } // update

    void loop(){                                     // loop() method
      if (isTimered){
        portENTER_CRITICAL(&timerMux);
        timerEnd(ButtonTimer);
        ButtonTimer = NULL;
        isTimered = false;
        portEXIT_CRITICAL(&timerMux);        
        LOG1("----------Timer deinited----------\n");
        for (int i=0; i<3; i++) {
          digitalWrite(ButtonArray[i],LOW);
        }        
      }
      
      if (ClSensorPin.changed) {
        ClSensorPin.changed = false;
        CycleTimeBegin = millis();
        LOG1("----------ClSensorPin.changed----------\n");
        if (digitalRead(ClSensorPin.PIN) == SENSOR_CLOSED)   {CurrentDoorState->setVal(CURRENT_DOOR_STATE_CLOSED);TargetDoorState->setVal(TARGET_DOOR_STATE_CLOSED);}
        if (digitalRead(ClSensorPin.PIN) == SENSOR_RELEASED) {CurrentDoorState->setVal(CURRENT_DOOR_STATE_OPENING);TargetDoorState->setVal(TARGET_DOOR_STATE_OPEN);}
      }
      if (OpSensorPin.changed) {
        OpSensorPin.changed = false;
        CycleTimeBegin = millis();
        LOG1("----------OpSensorPin.changed----------\n");
        if (digitalRead(OpSensorPin.PIN) == SENSOR_CLOSED)   {CurrentDoorState->setVal(CURRENT_DOOR_STATE_OPEN);TargetDoorState->setVal(TARGET_DOOR_STATE_OPEN);}
        if (digitalRead(OpSensorPin.PIN) == SENSOR_RELEASED) {CurrentDoorState->setVal(CURRENT_DOOR_STATE_CLOSING);TargetDoorState->setVal(TARGET_DOOR_STATE_CLOSED);}     
      }

      if (ObSensorPin.changed) {
        ObSensorPin.changed = false;
        if (digitalRead(ObSensorPin.PIN) == SENSOR_CLOSED)        {ObstructionDetected->setVal(true);}
        else                                                      {ObstructionDetected->setVal(false);}
      }
      
      if ( (millis() - CycleTimeBegin) > CycleTimeout ) { 
        LOG1("----------CycleTimeBegin.updated----------\n");
        CycleTimeBegin = millis();
        PollCurrentState();
      }
  } // loop 
};
