/*
   -- ESP2022_Joystick --

   This source code of graphical user interface 
   has been generated automatically by RemoteXY editor.
   To compile this code using RemoteXY library 3.1.8 or later version 
   download by link http://remotexy.com/en/library/
   To connect using RemoteXY mobile app by link http://remotexy.com/en/download/                   
     - for ANDROID 4.11.1 or later version;
     - for iOS 1.9.1 or later version;

   This source code is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.    
*/

//////////////////////////////////////////////
//        RemoteXY include library          //
//////////////////////////////////////////////

// RemoteXY select connection mode and include library 
#define REMOTEXY_MODE__ESP32CORE_BLE
#include <BLEDevice.h>

#include <RemoteXY.h>

// RemoteXY connection settings 
#define REMOTEXY_BLUETOOTH_NAME "ESP2022_joystick"
#define REMOTEXY_ACCESS_PASSWORD "esp2022"


// RemoteXY configurate  
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] =   // 116 bytes, was 96
{ 
  255,5,0,11,0,109,0,16,29,1,2,1,3,71,29,11,2,26,31,31,
  83,101,110,115,111,114,0,80,104,111,110,101,0,67,4,3,62,55,5,2,
  26,11,5,32,4,4,55,55,2,26,31,10,48,38,74,19,19,4,36,8,
  32,71,79,32,0,24,83,84,79,80,0,2,1,3,89,29,7,2,26,31,
  31,77,111,116,46,79,78,0,77,111,116,46,79,70,70,0,

  // NEW 2 QT bars (horizontal, next to each, bottom)
  65,2,3,15,80,6,  // Level-Bar 1 (front), X=15
  65,2,3,85,80,6   // Level-Bar 2 (back), X=85 (positions are layout indices)
};
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  uint8_t switch_1; // =1 if switch ON and =0 if OFF 
  int8_t joystick_1_x; // from -100 to 100  
  int8_t joystick_1_y; // from -100 to 100  
  uint8_t pushSwitch_1; // =1 if state is ON, else =0 
  uint8_t switch_2; // =1 if switch ON and =0 if OFF 

    // output variables
  char text_1[11];  // string UTF8 end zero 

    // NEW: level bars (0..100)
  uint8_t level_front;   // 0..100
  uint8_t level_back;    // 0..100

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0 

} RemoteXY;
#pragma pack(pop)

/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////

#define PIN_PUSHSWITCH_1 10
#define PIN_SWITCH_2 9


void setup() 
{
  RemoteXY_Init (); 
  
  pinMode (PIN_PUSHSWITCH_1, OUTPUT);
  pinMode (PIN_SWITCH_2, OUTPUT);
  
  // TODO you setup code
  
  // NEW initialize default values for new QT outputs
  RemoteXY.level_front = 0;
  RemoteXY.level_back  = 0;
  
}

void loop() 
{ 
  RemoteXY_Handler ();
  
  digitalWrite(PIN_PUSHSWITCH_1, (RemoteXY.pushSwitch_1==0)?LOW:HIGH);
  digitalWrite(PIN_SWITCH_2, (RemoteXY.switch_2==0)?LOW:HIGH);
  
  // NOTE:
  // Die Sensor-Balken in
  // (Projekt_file_gruppe_07_D.ino):
  //   RemoteXY.level_front = map(sensorFrontValue, 0, 4095, 0, 100);
  //   RemoteXY.level_back  = map(sensorBackValue, 0, 4095, 0, 100);
  //
  // Hier im UI-File nichts weiter verändern (kein Delay), RemoteXY_Handler()
  // übernimmt die Aktualisierung der App.
  
  // do not call delay() inside loop for RemoteXY
}
