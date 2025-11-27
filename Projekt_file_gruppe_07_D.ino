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
#define REMOTEXY_BLUETOOTH_NAME "test_drive"
#define REMOTEXY_ACCESS_PASSWORD "test"

// RemoteXY configurate
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] = { 
  255,5,0,11,0,89,0,16,29,1,2,1,3,71,29,11,2,26,31,31,
  83,101,110,115,111,114,0,80,104,111,110,101,0,67,4,3,62,55,5,2,
  26,11,5,32,4,4,55,55,2,26,31,10,48,38,74,19,19,4,36,8,
  32,71,79,32,0,24,83,84,79,80,0,2,1,3,89,29,7,2,26,31,
  31,77,111,116,46,79,78,0,77,111,116,46,79,70,70,0, 

  // Display for Sensors
  65,2,3,15,80,6, // bar front
  65,2,3,85,80,6  // bar back
};

// Structure RemoteXY
struct {
  uint8_t switch_1;
  int8_t joystick_1_x; // от -100 до 100
  int8_t joystick_1_y; // от -100 до 100
  uint8_t pushSwitch_1; // Кнопка удара
  uint8_t switch_2;
  char text_1[128]; // from 11 -> logging msg textbox
  uint8_t level_front;  // QTR front
  uint8_t level_back;  // QTR back
  uint8_t connect_flag;
} RemoteXY;
#pragma pack(pop)

// Motor Speed
void setMotorSpeed(int pinA, int pinB, int speed) {
  if (speed > 0) {
    PWM_analogWrite(pinA, speed);
    PWM_analogWrite(pinB, 0);
  } else if (speed < 0) {
    PWM_analogWrite(pinA, 0);
    PWM_analogWrite(pinB, -speed); 
  } else {
    PWM_analogWrite(pinA, 0);
    PWM_analogWrite(pinB, 0);
  }
}

// ==============================
// --- Motor helper functions ---
// ==============================

// Stop
void stopMotoren() {
  PWM_analogWrite(Motor_links_A_Pin, 0);
  PWM_analogWrite(Motor_links_B_Pin, 0);
  PWM_analogWrite(Motor_rechts_A_Pin, 0);
  PWM_analogWrite(Motor_rechts_B_Pin, 0);
}

// Drive backwards and stop
void kurzRueckwaerts(int speed = 150, int dauer = 400) {
  PWM_analogWrite(Motor_links_A_Pin, 0);
  PWM_analogWrite(Motor_links_B_Pin, speed);
  PWM_analogWrite(Motor_rechts_A_Pin, 0);
  PWM_analogWrite(Motor_rechts_B_Pin, speed);
  delay(dauer);
  stopMotoren();
}

// Drive forward and stop
void kurzVorwaerts(int speed = 150, int dauer = 400) {
  PWM_analogWrite(Motor_links_A_Pin, speed);
  PWM_analogWrite(Motor_links_B_Pin, 0);
  PWM_analogWrite(Motor_rechts_A_Pin, speed);
  PWM_analogWrite(Motor_rechts_B_Pin, 0);
  delay(dauer);
  stopMotoren();
}

// ============================
//   Timed Log-System
// ============================
#define LOG_MAX_LINES 10
#define LOG_DURATION_MS 2000 // Linestop-Msg is displayed 2 seconds

struct LogEntry {
  String text;
  unsigned long timestamp; // time for msg creation
  bool permanent;          // true = stays permanent (i.e. calibrating)
};

LogEntry logLines[LOG_MAX_LINES];

void addLog(const char* msg, bool permanent=false) {
  Serial.println(msg);

  // delete older msgs, push down
  for(int i=LOG_MAX_LINES-1;i>0;i--) logLines[i] = logLines[i-1];
  logLines[0].text = String(msg);
  logLines[0].timestamp = millis();
  logLines[0].permanent = permanent;

  updateRemoteXYText();
}

void updateRemoteXYText() {
  String text = "";
  for(int i = LOG_MAX_LINES-1; i>=0; i--) {
    if(logLines[i].text.length() > 0) {
      // delete tmp msgs
      if(!logLines[i].permanent && millis() - logLines[i].timestamp > LOG_DURATION_MS) continue;
      text += logLines[i].text + "\n";
    }
  }
  text.toCharArray(RemoteXY.text_1, sizeof(RemoteXY.text_1));
}

// ============================
//   QTR AUTO CALIBRATE (10s)
// ============================
int linieThreshold = 500;

void kalibriereSensoren() {
  int minVorne = 4095, maxVorne = 0;
  int minHinten = 4095, maxHinten = 0;

  addLog("Starte Sensor-Kalibrierung...", true);
  addLog("Bitte beide Sensoren 10 Sekunden über WEISS und SCHWARZ bewegen.", true);

  unsigned long start = millis();
  while (millis() - start < 10000) {  // 10 seconds calibrating
    int vorne = analogRead(sensorVornePin);
    int hinten = analogRead(sensorHintenPin);

    if (vorne < minVorne) minVorne = vorne;
    if (vorne > maxVorne) maxVorne = vorne;
    if (hinten < minHinten) minHinten = hinten;
    if (hinten > maxHinten) maxHinten = hinten;

    RemoteXY.level_front = map(vorne, 0, 4095, 0, 100);
    RemoteXY.level_back  = map(hinten, 0, 4095, 0, 100);

    RemoteXY_Handler(); // Update UI while calibrating
    delay(20);
  }

  // Black, White, Border
  int schwarz = (minVorne + minHinten) / 2;
  int weiss   = (maxVorne + maxHinten) / 2;

  linieThreshold = (schwarz + weiss) / 2;

  char buf[64];
  snprintf(buf, sizeof(buf), "Kalibriert: Schwarz=%d Weiss=%d SW=%d", schwarz, weiss, linieThreshold);
  addLog(buf, true);
}

void setup() {
  Serial.begin(115200);
  RemoteXY_Init();
  PWM_init(); // Инициализация ШИМ из PWM.h
  
  // Инициализация пинов
  // pinMode(Line_Sensor_Pin, INPUT);

  // Pin init for Solenoid
  pinMode(Solenoid_Pin, OUTPUT);
  digitalWrite(Solenoid_Pin, LOW);
  
  // Pin init for QT
  pinMode(sensorVornePin, INPUT);
  pinMode(sensorHintenPin, INPUT);

  // Остановка моторов
  setMotorSpeed(Motor_links_A_Pin, Motor_links_B_Pin, 0);
  setMotorSpeed(Motor_rechts_A_Pin, Motor_rechts_B_Pin, 0);

  Serial.println("Bluetooth RC Car ready");
  addLog("Bluetooth RC Car ready", true);  // new msg in log in app
}

void loop() {
  RemoteXY_Handler();

  // Shoot loop if switched button
  if (RemoteXY.pushSwitch_1 != 0) {
    addLog("Schuss!");
    digitalWrite(Solenoid_Pin, HIGH); 
    delay(20);                       
    digitalWrite(Solenoid_Pin, LOW);
    delay(1000);
  }

  // QT reading init
  int sensorVorne = analogRead(sensorVornePin);
  int sensorHinten = analogRead(sensorHintenPin);

  // UI element QT sensor bar
  RemoteXY.level_front = map(sensorVorne, 0, 4095, 0, 100);
  RemoteXY.level_back  = map(sensorHinten, 0, 4095, 0, 100);

  // --- Line detection front ---
  if (sensorVorne < linieThreshold) {
    addLog("Linie vorne erkannt!");
    kurzRueckwaerts(180, 400);
    stopMotoren();
    delay(500);
    return;
  }

  // --- Line detection back ---
  if (sensorHinten < linieThreshold) {
    addLog("Linie hinten erkannt!");
    kurzVorwaerts(180, 400);
    stopMotoren();
    delay(500);
    return;
  }

  // if (digitalRead(Line_Sensor_Pin) == HIGH) {
  //   setMotorSpeed(Motor_links_A_Pin, Motor_links_B_Pin, -150);
  //   setMotorSpeed(Motor_rechts_A_Pin, Motor_rechts_B_Pin, -150);
  //   delay(600); 
  //   setMotorSpeed(Motor_links_A_Pin, Motor_links_B_Pin, 0);
  //   setMotorSpeed(Motor_rechts_A_Pin, Motor_rechts_B_Pin, 0);
  //   return; // Прерываем loop, чтобы джойстик не мешал
  // }

 
  int x = RemoteXY.joystick_1_x; // Поворот -100...100
  int y = RemoteXY.joystick_1_y; // Газ -100...100

 
  int leftMotorSpeed = (y + x) * 2.5;
  int rightMotorSpeed = (y - x) * 2.5;

  
  leftMotorSpeed = constrain(leftMotorSpeed, -255, 255);
  rightMotorSpeed = constrain(rightMotorSpeed, -255, 255);


  if (abs(leftMotorSpeed) < 20) leftMotorSpeed = 0;
  if (abs(rightMotorSpeed) < 20) rightMotorSpeed = 0;

  setMotorSpeed(Motor_links_A_Pin, Motor_links_B_Pin, leftMotorSpeed);
  setMotorSpeed(Motor_rechts_A_Pin, Motor_rechts_B_Pin, rightMotorSpeed);
  
  delay(10);
}

