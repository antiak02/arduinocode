#include <ESP32Servo.h>

Servo ServoArray[8];

int ServoPin[] = {2, 4, 17, 16, 5, 18, 19, 23};

void ServoSetup(int ServoNum) {
  ServoArray[ServoNum].attach(ServoPin[ServoNum]);
}

void ServoInhibit(int ServoNum) {
  ServoArray[ServoNum].detach();
}

void ServoUpdate(int ServoNum, int Velocity) {
  ServoArray[ServoNum].write(Velocity+95);
}
