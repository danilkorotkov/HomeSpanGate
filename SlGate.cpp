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
//#define SENSOR_CLOSED   0
//#define SENSOR_RELEASED 1

#define P_SENSOR_CLOSED   0
#define P_SENSOR_RELEASED 1

#include "SlGate.h"

////////////////////////////
bool isTimered = false;
hw_timer_t * ButtonTimer = NULL;
portMUX_TYPE DoortimerMux = portMUX_INITIALIZER_UNLOCKED;

int OpenPin     = 18;                                       
int ClosePin    = 19;
//int StopPin     = 21;
int JmpPin      = 13; // high на в промежуточном положении все High и  low - в крайних 

int SENSOR_CLOSED   = 0;
int SENSOR_RELEASED = 1;

void IRAM_ATTR isr(void* arg) {
    Sensor* s = static_cast<Sensor*>(arg);
    s->changed = true;
};

void IRAM_ATTR onTimer(){
  portENTER_CRITICAL_ISR(&DoortimerMux);
  isTimered = true;
  digitalWrite(OpenPin,LOW);
  digitalWrite(ClosePin,LOW);
  portEXIT_CRITICAL_ISR(&DoortimerMux);
};
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void SL_GATE::initTimer(){
  ButtonTimer = timerBegin(0, 80, true);
  timerAttachInterrupt(ButtonTimer, &onTimer, true);
  timerAlarmWrite(ButtonTimer, 1000000, false);
  timerAlarmEnable(ButtonTimer);  
};

SL_GATE::SL_GATE() : Service::GarageDoorOpener(){

  LOG1("Constructing Gate…\n");
  CurrentDoorState    = new Characteristic::CurrentDoorState(CURRENT_DOOR_STATE_OPEN);
  TargetDoorState     = new Characteristic::TargetDoorState(TARGET_DOOR_STATE_OPEN);
  ObstructionDetected = new Characteristic::ObstructionDetected();
  Name=new Characteristic::Name("Gate"); 
                         
  pinMode(OpenPin,OUTPUT); 
  digitalWrite(OpenPin,LOW);
                                              
  pinMode(ClosePin,OUTPUT);
  digitalWrite(ClosePin,LOW);
                      
  pinMode(JmpPin, INPUT_PULLUP);
  SENSOR_CLOSED    ^= digitalRead(JmpPin);
  SENSOR_RELEASED  ^= digitalRead(JmpPin);// инвертирование концевиков
  if (digitalRead(JmpPin)) {LOG1("reverse sensors \n");}
    
  pinMode(ClSensorPin.PIN, INPUT_PULLUP);
  pinMode(OpSensorPin.PIN, INPUT_PULLUP);
  pinMode(ObSensorPin.PIN, INPUT_PULLUP);
  attachInterruptArg(ClSensorPin.PIN, isr, &ClSensorPin, CHANGE);
  attachInterruptArg(OpSensorPin.PIN, isr, &OpSensorPin, CHANGE);
  attachInterruptArg(ObSensorPin.PIN, isr, &ObSensorPin, CHANGE);
  

  ClSensorPin.stableState = digitalRead(ClSensorPin.PIN);
  OpSensorPin.stableState = digitalRead(OpSensorPin.PIN);
  
  PollCurrentState();

  LOG1("Constructing Gate successful!\n");
  //LOG1(WiFi.localIP());  
} // end constructor

void SL_GATE::PollCurrentState(){
    LOG1("polling...\n");
  
    if (digitalRead(ClSensorPin.PIN)      == SENSOR_CLOSED)   {if (CurrentDoorState-> getVal() !=CURRENT_DOOR_STATE_CLOSED)        
                                                              {LOG1("polling CLOSED\n");
                                                              ClSensorPin.stableState = SENSOR_CLOSED;
                                                              FullyClosed();
                                                              }}
    
    else if (digitalRead(OpSensorPin.PIN) == SENSOR_CLOSED)   {if (CurrentDoorState->  getVal() != CURRENT_DOOR_STATE_OPEN)   
                                                              {LOG1("polling OPEN\n");
                                                              OpSensorPin.stableState = SENSOR_CLOSED;
                                                              FullyOpened();
                                                              }}
    
    else if                                                   (CurrentDoorState-> getVal() != CURRENT_DOOR_STATE_OPEN)   
                                                              {LOG1("polling all released\n");
                                                              CurrentDoorState-> setVal(CURRENT_DOOR_STATE_OPEN);   
                                                              TargetDoorState->   setVal(TARGET_DOOR_STATE_OPEN);
                                                              OpSensorPin.stableState = SENSOR_RELEASED; 
                                                              ClSensorPin.stableState = SENSOR_RELEASED;
                                                              }
    
    if (digitalRead(ObSensorPin.PIN)    == P_SENSOR_CLOSED && !ObstructionDetected->getVal())        
                                                              {ObstructionDetected->setVal(true);
                                                              ObSensorPin.stableState = P_SENSOR_CLOSED;
                                                              }
    
  LOG1("polling over\n");
}

boolean SL_GATE::update(){            

    if(TargetDoorState->getNewVal()==TARGET_DOOR_STATE_OPEN &&
       CurrentDoorState->getVal() != CURRENT_DOOR_STATE_OPENING){ 
                                                                                                                     
      LOG1("-----------Opening Gate----------\n");
      FullyOpenExtern();
      Open();
    
    } else if(TargetDoorState->getNewVal()==TARGET_DOOR_STATE_CLOSED &&
              CurrentDoorState->getVal() != CURRENT_DOOR_STATE_CLOSING){

        // анализ препятствия
        if (!ObstructionDetected->getVal()){
          LOG1("----------Closing Gate----------\n");                                
          FullyCloseExtern();
          Close();
          
        } else if (CurrentDoorState->getVal() == CURRENT_DOOR_STATE_OPENING || CurrentDoorState->getVal() == CURRENT_DOOR_STATE_OPEN) 
                  {TargetDoorState->setVal(TARGET_DOOR_STATE_OPEN);}

    } 
    
    LOG1("----------UpdateOver----------\n");
    CycleTimeBegin = millis();
    return(true);                                  
  
} // update

void SL_GATE::loop(){                                     
      // если истек таймер удержания кнопки, убиваем таймер, отжимаем кнопку
      if (isTimered){
        portENTER_CRITICAL(&DoortimerMux);
        timerEnd(ButtonTimer);
        ButtonTimer = NULL;
        isTimered = false;
        portEXIT_CRITICAL(&DoortimerMux);        
        LOG1("----------Timer deinited----------\n");     
      }
      
      // если сработал концевик, фиксируем время срабатывания, и игнорируем его изменения-дребезг после обработки события
      if (ClSensorPin.changed && (millis() - PortPollBegin)>PortPollTimeout) {
        ClSensorPin.changed = false;
        PortPollBegin = millis();
        CycleTimeBegin = millis();
        // если новоее состояние отличается от предыдущего стабильного
        if (digitalRead(ClSensorPin.PIN) == SENSOR_CLOSED && ClSensorPin.stableState == SENSOR_RELEASED)   {
                                                              LOG1("----------ClSensorPin.SENSOR_CLOSED----------\n");
                                                              // обновляем стабильное
                                                              ClSensorPin.stableState = SENSOR_CLOSED;       
                                                              // устанавливаем состояние Закрыто
                                                              FullyClosed();
                                                              }
        
        if (digitalRead(ClSensorPin.PIN) == SENSOR_RELEASED && ClSensorPin.stableState == SENSOR_CLOSED) {
                                                              LOG1("----------ClSensorPin.SENSOR_RELEASED----------\n");  
                                                              
                                                              // если состояние итак открывается, то ничего менять не будем
                                                              // по сути игнорим открывание через HAP и реагируем только на брелок
                                                              if ( CurrentDoorState->getVal() != CURRENT_DOOR_STATE_OPENING ){ 
                                                                FullyOpenExtern();
                                                                TargetDoorState->setVal(TARGET_DOOR_STATE_OPEN);
                                                              }
                                                              ClSensorPin.stableState = SENSOR_RELEASED;
                                                              }
      
      } else if ( ((millis() - PortPollBegin)>PortPollTimeout) && (ClSensorPin.stableState == SENSOR_CLOSED) && (CurrentDoorState->getVal() != CURRENT_DOOR_STATE_CLOSED) ){
                                                              LOG1("closed by timeout\n");
                                                              FullyClosed();}
      
      if (OpSensorPin.changed && (millis() - PortPollBegin)>PortPollTimeout) {
        OpSensorPin.changed = false;
        PortPollBegin = millis();
        CycleTimeBegin = millis();
        if (digitalRead(OpSensorPin.PIN) == SENSOR_CLOSED && OpSensorPin.stableState == SENSOR_RELEASED)   {
                                                              LOG1("----------OpSensorPin.SENSOR_CLOSED----------\n");
                                                              OpSensorPin.stableState = SENSOR_CLOSED;
                                                              FullyOpened();}
        
        if (digitalRead(OpSensorPin.PIN) == SENSOR_RELEASED && OpSensorPin.stableState == SENSOR_CLOSED) {
                                                              LOG1("----------OpSensorPin.SENSOR_RELEASED----------\n");
                                                              
                                                              // если состояние итак закрывается, то ничего менять не будем
                                                              // по сути игнорим закрывание через HAP и реагируем на брелок
                                                              if (CurrentDoorState->getVal() != CURRENT_DOOR_STATE_CLOSING){ 
                                                                FullyCloseExtern();
                                                                TargetDoorState->setVal(TARGET_DOOR_STATE_CLOSED);
                                                              }
                                                              OpSensorPin.stableState = SENSOR_RELEASED;
                                                              }     
      
      } else if ( ((millis() - PortPollBegin)>PortPollTimeout) && OpSensorPin.stableState == SENSOR_CLOSED && CurrentDoorState->getVal() != CURRENT_DOOR_STATE_OPEN ){
                                                              LOG1("opened by timeout\n");
                                                              FullyOpened();}

      if (ObSensorPin.changed && (millis() - ObPortPollBegin)>PortPollTimeout) {
        ObSensorPin.changed = false;
        ObPortPollBegin = millis();
        CycleTimeBegin = millis();
        if (digitalRead(ObSensorPin.PIN) == P_SENSOR_CLOSED && ObSensorPin.stableState == P_SENSOR_RELEASED)    
                                                              {LOG1("----------Optocoupler.SENSOR_CLOSED----------\n");
                                                               ObstructionDetected->setVal(true);
                                                               ObSensorPin.stableState = P_SENSOR_CLOSED;
                                                               }
        
        if (digitalRead(ObSensorPin.PIN) == P_SENSOR_RELEASED && ObSensorPin.stableState == P_SENSOR_CLOSED)                                                        
                                                              {LOG1("----------Optocoupler.SENSOR_RELEASED----------\n");
                                                               ObstructionDetected->setVal(false);
                                                               ObSensorPin.stableState = P_SENSOR_RELEASED;
                                                               }
      
      } else if ( ((millis() - ObPortPollBegin)>PortPollTimeout) && ObSensorPin.stableState == P_SENSOR_CLOSED && !ObstructionDetected->getVal() )
                                                              {ObstructionDetected->setVal(true);
                                                              }
        
        else if ( ((millis() - ObPortPollBegin)>PortPollTimeout) && ObSensorPin.stableState == P_SENSOR_RELEASED && ObstructionDetected->getVal() )
                                                              {ObstructionDetected->setVal(false);
                                                              }
                                                              
      
      if ( (millis() - CycleTimeBegin) > CycleTimeout ) { 
        LOG1("----------CycleTimeBegin.updated----------\n");
        CycleTimeBegin = millis();
        PollCurrentState();
      }
}// loop 

void SL_GATE::FullyOpened(){
  CurrentDoorState-> setVal(CURRENT_DOOR_STATE_OPEN);   
  TargetDoorState->   setVal(TARGET_DOOR_STATE_OPEN);
}

void SL_GATE::FullyOpenExtern(){
}

void SL_GATE::FullyCloseExtern(){
  CurrentDoorState->setVal(CURRENT_DOOR_STATE_CLOSING);
}

void SL_GATE::FullyClosed(){
  CurrentDoorState-> setVal(CURRENT_DOOR_STATE_CLOSED); 
  TargetDoorState->  setVal(TARGET_DOOR_STATE_CLOSED);  
}

void SL_GATE::Open(){
  LOG1("call open routine\n");
  digitalWrite(ClosePin,LOW);
  digitalWrite(OpenPin,HIGH);
  
  initTimer();  
}

void SL_GATE::Close(){
  LOG1("call close routine\n");
  digitalWrite(OpenPin,LOW);
  digitalWrite(ClosePin,HIGH);
  
  initTimer();  
}
//////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
