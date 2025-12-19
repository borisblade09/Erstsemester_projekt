#define REMOTEXY_MODE__ESP32CORE_BLE
#include <BLEDevice.h>
#include <RemoteXY.h>
#include "Pins1.h"
#include "PWM1.h"

#define REMOTEXY_BLUETOOTH_NAME "FussballRoboter_7D"
#define REMOTEXY_ACCESS_PASSWORD "1234"

const int SENSOR_THRESHOLD = 2000; 


#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] =
  { 255,3,0,2,0,117,0,19,0,0,0,70,117,115,115,98,97,108,108,95,
  82,111,98,111,116,101,114,95,55,68,0,29,2,106,200,200,84,1,1,5,
  0,5,208,21,143,143,5,10,74,74,32,150,8,31,1,63,102,57,57,158,
  46,32,31,10,6,1,72,105,116,33,0,129,21,9,71,29,70,2,79,12,
  64,13,70,117,195,159,98,97,108,108,114,111,98,111,116,101,114,0,70,65,
  17,43,43,116,20,11,11,16,24,64,0,70,43,93,43,43,116,33,11,11,
  16,8,64,0 };
  
struct {
  int8_t joystick_01_x; 
  int8_t joystick_01_y; 
  uint8_t button_01;   
  uint8_t led_01; 
  uint8_t led_02; 
  uint8_t connect_flag;
} RemoteXY;
#pragma pack(pop)

void stopAll() {
  setMotorSpeed(Motor_links_A_Pin, Motor_links_B_Pin, 0);
  setMotorSpeed(Motor_rechts_A_Pin, Motor_rechts_B_Pin, 0);
}

void setMotorSpeed(int pinA, int pinB, int speed) {
  if (speed == 0) {
    PWM_analogWrite(pinA, 0);
    PWM_analogWrite(pinB, 0);
    digitalWrite(pinA, LOW);
    digitalWrite(pinB, LOW);
  } else if (speed > 0) {
    PWM_analogWrite(pinA, speed);
    PWM_analogWrite(pinB, 0);
  } else {
    PWM_analogWrite(pinA, 0);
    PWM_analogWrite(pinB, -speed);
  }
}

void setup() {
  
  pinMode(6, OUTPUT); digitalWrite(6, LOW);
  pinMode(7, OUTPUT); digitalWrite(7, LOW);
  pinMode(10, OUTPUT); digitalWrite(10, LOW);
  pinMode(5, OUTPUT); digitalWrite(5, LOW);
  pinMode(1, OUTPUT); digitalWrite(1, LOW);


  delay(3000); 

  //Serial.begin(115200);
  RemoteXY_Init();
  PWM_init(); 
  
  pinMode(Sensor_Front_Pin, INPUT);
  pinMode(Sensor_Back_Pin, INPUT);
  
  pinMode(Solenoid_Pin, OUTPUT);
  digitalWrite(Solenoid_Pin, LOW);

  stopAll();
}

void loop() {
  RemoteXY_Handler();

  if (RemoteXY.connect_flag == 0) {
    stopAll();
    return;
  }

  
  if (RemoteXY.button_01 != 0) {
    digitalWrite(Solenoid_Pin, HIGH); 
    delay(100);                       
    digitalWrite(Solenoid_Pin, LOW);
  }


  int frontValue = analogRead(Sensor_Front_Pin);
  int backValue = analogRead(Sensor_Back_Pin);


  bool frontLine = (frontValue > SENSOR_THRESHOLD);
  bool backLine = (backValue > SENSOR_THRESHOLD);
  
  RemoteXY.led_01 = frontLine ? 1 : 0;
  RemoteXY.led_02 = backLine ? 1 : 0;

  if (frontLine) {
    setMotorSpeed(Motor_links_A_Pin, Motor_links_B_Pin, 150);
    setMotorSpeed(Motor_rechts_A_Pin, Motor_rechts_B_Pin, 150);
    delay(600); 
    stopAll();
    return; 
  }

  if (backLine) {
    setMotorSpeed(Motor_links_A_Pin, Motor_links_B_Pin, -150);
    setMotorSpeed(Motor_rechts_A_Pin, Motor_rechts_B_Pin, -150);
    delay(600); 
    stopAll();
    return;
  }

  int x = RemoteXY.joystick_01_x;
  int y = RemoteXY.joystick_01_y;

  int left = (y + x) * 2.5;
  int right = (y - x) * 2.5;
  
  left = constrain(left, -255, 255);
  right = constrain(right, -255, 255);

  if (abs(left) < 20) left = 0;
  if (abs(right) < 20) right = 0;

  setMotorSpeed(Motor_links_A_Pin, Motor_links_B_Pin, left);
  setMotorSpeed(Motor_rechts_A_Pin, Motor_rechts_B_Pin, right);
  
  delay(10);
}