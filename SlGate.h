#ifndef SLGATE_H
#define SLGATE_H
#include "HomeSpan.h" 

// Possible values for characteristic garage
#define CURRENT_DOOR_STATE_OPEN    0
#define CURRENT_DOOR_STATE_CLOSED  1
#define CURRENT_DOOR_STATE_OPENING 2
#define CURRENT_DOOR_STATE_CLOSING 3
#define CURRENT_DOOR_STATE_STOPPED 4
#define CURRENT_DOOR_STATE_UNKNOWN 255

#define TARGET_DOOR_STATE_OPEN    0
#define TARGET_DOOR_STATE_CLOSED  1
#define TARGET_DOOR_STATE_UNKNOWN 255

struct SL_GATE;
struct Sensor;

void IRAM_ATTR isr(void*);
void IRAM_ATTR onTimer();

struct Sensor {
    uint8_t PIN;
    bool changed;
    int stableState;
};

struct SL_GATE : Service::GarageDoorOpener {         // First we create a derived class from the HomeSpan 

  struct Sensor OpSensorPin = {22, false, 1};
  struct Sensor ClSensorPin = {23, false, 1};
  struct Sensor ObSensorPin = {17, false, 1};
  
  uint32_t CycleTimeout = 60000; //60s
  uint32_t CycleTimeBegin;
  uint32_t PortPollTimeout = 700;
  uint32_t PortPollBegin, ObPortPollBegin = 0;
  
  SpanCharacteristic *CurrentDoorState;              
  SpanCharacteristic *TargetDoorState;
  SpanCharacteristic *ObstructionDetected;
  SpanCharacteristic *Name; 
  SpanCharacteristic *WiFiLevel;
   
  SL_GATE();
  void PollCurrentState();
  void initTimer();
  boolean update();                                   
  void loop();
  void FullyOpened();
  void FullyClosed();
  void FullyOpenExtern();
  void FullyCloseExtern();
  void Open();
  void Close();
};


////////////////

#endif //SLGATE_H
