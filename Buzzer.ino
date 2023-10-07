#include <ESP32Servo.h>

#define Buzzer 25

void BuzzerSetup(void) {
  pinMode(Buzzer, OUTPUT);
}

void BuzzerTone(int freq, int duration) {
  tone(Buzzer, freq, duration);
}
