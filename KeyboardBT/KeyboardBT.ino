#include <BleKeyboard.h>
#include "esp32-hal-cpu.h"

const int rows = 3;
const int cols = 3;
int rowsPins[] = {18, 16, 17};
int colsPins[] = {21, 22, 23};

char keyMap[][cols] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
  };

int keyState[rows][cols] = {LOW};

BleKeyboard bleKeyboard("BTETest", "roolin", 69);

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  for(int i = 0; i < rows; i++) {
    pinMode(rowsPins[i], INPUT_PULLDOWN);
  }

  for(int i = 0; i < cols; i++) {
    pinMode(colsPins[i], OUTPUT);
  }
  
  bleKeyboard.begin();

  setCpuFrequencyMhz(2);
}

void loop() {
  for(int i = 0; i < cols; i++) {
    digitalWrite(colsPins[i], HIGH);
    for(int j = 0; j < rows; j++) {
      int keyCurr = digitalRead(rowsPins[j]);
      if(keyCurr == HIGH && keyState[j][i] == LOW) {
        bleKeyboard.press(keyMap[j][i]);
        keyState[j][i] = HIGH;
      } else if(keyCurr == LOW && keyState[j][i] == HIGH) {
        bleKeyboard.release(keyMap[j][i]);
        keyState[j][i] = LOW;
      }
      serialPrintln(i, j);
    }
    digitalWrite(colsPins[i], LOW);
  }
}

char * serialPrintln(int i, int j) {
  char buff[100];
  sprintf(buff, "%d; i:%d, j:%d", keyMap[j][i], i, j);
  Serial.println(buff);
}
