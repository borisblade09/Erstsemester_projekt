// #define Motor_links_A_Pin 4
// #define Motor_links_B_Pin 5
// #define Motor_rechts_A_Pin 6
// #define Motor_rechts_B_Pin 7

#define REMOTEXY_MODE__ESP32CORE_BLE
#include <BLEDevice.h>
#include <RemoteXY.h>
#include <ESP32Servo.h>
#include "pins.h"
#include "PWM.h"

// RemoteXY connection settings
#define REMOTEXY_BLUETOOTH_NAME "ESP2022_joystick"
#define REMOTEXY_ACCESS_PASSWORD "esp2022"

// RemoteXY configurate
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] = { 
  255,5,0,11,0,89,0,16,29,1,2,1,3,71,29,11,2,26,31,31,
  83,101,110,115,111,114,0,80,104,111,110,101,0,67,4,3,62,55,5,2,
  26,11,5,32,4,4,55,55,2,26,31,10,48,38,74,19,19,4,36,8,
  32,71,79,32,0,24,83,84,79,80,0,2,1,3,89,29,7,2,26,31,
  31,77,111,116,46,79,78,0,77,111,116,46,79,70,70,0 
};

// Structure RemoteXY
struct {
  uint8_t switch_1;
  int8_t joystick_1_x;
  int8_t joystick_1_y;
  uint8_t pushSwitch_1;
  uint8_t switch_2;
  char text_1[11];
  uint8_t connect_flag;
} RemoteXY;
#pragma pack(pop)


Servo myservo;

// --- Settings ---
void setup() {
  Serial.begin(115200);
  RemoteXY_Init();
  PWM_init();

  myservo.setPeriodHertz(50);
  myservo.attach(servoPin);

  PWM_analogWrite(Motor_links_A_Pin, 0);
  PWM_analogWrite(Motor_links_B_Pin, 0);
  PWM_analogWrite(Motor_rechts_A_Pin, 0);
  PWM_analogWrite(Motor_rechts_B_Pin, 0);

  Serial.println("Bluetooth RC Car ready 🚗");
}

// --- Main loop ---
void loop() {
  RemoteXY_Handler();

  int x = RemoteXY.joystick_1_x; // -100…100
  int y = RemoteXY.joystick_1_y; // -100…100

  // Servo
  int angle = map(x, -100, 100, 0, 180);
  myservo.write(angle);

  // Speed 
  int speed = map(abs(y), 0, 100, 0, 255);

  if (y > 20) { // Ahead
    PWM_analogWrite(Motor_links_A_Pin, speed);
    PWM_analogWrite(Motor_links_B_Pin, 0);
    PWM_analogWrite(Motor_rechts_A_Pin, speed);
    PWM_analogWrite(Motor_rechts_B_Pin, 0);
  }
  else if (y < -20) { // Backwards
    PWM_analogWrite(Motor_links_A_Pin, 0);
    PWM_analogWrite(Motor_links_B_Pin, speed);
    PWM_analogWrite(Motor_rechts_A_Pin, 0);
    PWM_analogWrite(Motor_rechts_B_Pin, speed);
  }
  else { // Stop
    PWM_analogWrite(Motor_links_A_Pin, 0);
    PWM_analogWrite(Motor_links_B_Pin, 0);
    PWM_analogWrite(Motor_rechts_A_Pin, 0);
    PWM_analogWrite(Motor_rechts_B_Pin, 0);
  }

  //delay(20); // Stabilization

  Serial.print("x="); Serial.print(RemoteXY.joystick_1_x);
  Serial.print("  y="); Serial.print(RemoteXY.joystick_1_y);
  Serial.print("  switch1="); Serial.print(RemoteXY.switch_1);
  Serial.print("  connect="); Serial.println(RemoteXY.connect_flag);
  delay(200);
}
