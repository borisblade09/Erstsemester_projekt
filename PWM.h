#pragma once
#include <ESP32PWM.h>

ESP32PWM pwm1;
ESP32PWM pwm2;
ESP32PWM pwm3;
ESP32PWM pwm4;
int freq = 1000;

void PWM_init() {
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  pwm1.attachPin(Motor_links_A_Pin, freq, 8);
  pwm2.attachPin(Motor_links_B_Pin, freq, 8);
  pwm3.attachPin(Motor_rechts_A_Pin, freq, 8);
  pwm4.attachPin(Motor_rechts_B_Pin, freq, 8);
}

void PWM_analogWrite(int Motor, int speed) {
  switch (Motor) {
    case Motor_links_A_Pin:
      pwm1.write(speed);
      break;
    case Motor_links_B_Pin:
      pwm2.write(speed);
      break;
    case Motor_rechts_A_Pin:
      pwm3.write(speed);
      break;
    case Motor_rechts_B_Pin:
      pwm4.write(speed);
      break;
  }
}
