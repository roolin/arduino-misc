#include <ButtonDebounce.h>
#include <ESPRotary.h>
#include <Math.h>
#include <Encoder.h>

#define STATE 16
#define DIR 4
#define STEP 5
// Driver ENABLE pin, HIGH - driver off, LOW - driver on
#define ENABLE 15

#define STARS 0
#define TIMELAPSE 1

int tlSpeedValues[] = {0, 500, 1000, 2000, 3000, 4000, 5000, 7000, 10000, 15000, 20000, 30000};

ESPRotary knob = ESPRotary(12, 13, 4);
ButtonDebounce button(14, 250);
ButtonDebounce operModeButton(16, 100);

unsigned long lastTime = 0;
int speed = 0;

int direction = HIGH;
byte enable = LOW;

int operMode = STARS;

int microSteppingMultiplier = 8;

int teeths = 61;

// 36.111111...
int steps4degree = 36 * microSteppingMultiplier;
// 800 becouse potentiometer is not going up to 1024 because of a voltage divider
int maxPotentiometerValue = 800;

// 200 steps per rotation
int stepsInOneRotation = teeths * 200 * microSteppingMultiplier;
int secondsInOneRotation = 86164;

// how often we need to step for star tracking
int millis4Step4Stars = secondsInOneRotation * 1000 / stepsInOneRotation;

void setup() {
  Serial.begin(115200);

  knob.setChangedHandler(rotate);

  button.setCallback(buttonPress);
  operModeButton.setCallback(operModeButtonState);

  pinMode(DIR, OUTPUT);
  pinMode(STEP, OUTPUT);
  pinMode(ENABLE, OUTPUT);

  operMode = digitalRead(STATE);

  enableDisable(HIGH);
}

void loop() {
  //Serial.println("1." + String(micros()));
  knob.loop();
  //Serial.println("2." + String(micros()));
  button.update();
  //Serial.println("3." + String(micros()));
  operModeButton.update();
  //Serial.println("4." + String(micros()));

  if (operMode == STARS) {
    modeStars();
  } else if (operMode == TIMELAPSE) {
    modeTimelapse();
  }
  //Serial.println("10." + String(micros()));
}

void modeStars() {
  unsigned long currentTime;
  unsigned long difference;
  //Serial.println("5." + String(micros()));
  currentTime = millis();
  difference = currentTime - lastTime;

  if (speed == 0) {
    return;
  }

  int interval = 0;
  //Serial.println("6." + String(micros()));
  if(speed == 1) {
    interval = millis4Step4Stars;
  } else {
    interval = 1000;
    for(int i = 0; i < speed; i++) {
      interval /= 2;
    }
  }
  
  //Serial.println("7." + String(micros()));
  if (difference >= interval) {
    //Serial.println("Time: " + String(currentTime) + " - " + String(speed));
    lastTime = currentTime;
    //Serial.println("8." + String(micros()));
    digitalWrite(STEP, HIGH);
    delayMicroseconds(2);
    digitalWrite(STEP, LOW);
    //Serial.println("9." + String(micros()));
    //Serial.println("STAR MOVE: " + String(operMode) + ", Potencjometr: " + String(readPotentiometer()) + ", Interval: " + String(interval));
  }
}

void modeTimelapse() {
  unsigned long currentTime;
  unsigned long difference;

  currentTime = millis();
  difference = currentTime - lastTime;

  if (speed == 0) {
    return;
  }

  if (difference >= tlSpeedValues[speed]) {
    lastTime = currentTime;

    int moves = steps4degree * readPotentiometer() / maxPotentiometerValue;

    //Serial.println("Move: " + String(moves) + ", Potentiometer: " + String(readPotentiometer()));
    for (int i = 0; i < moves; i++) {
      digitalWrite(STEP, HIGH);
      delayMicroseconds(2);
      digitalWrite(STEP, LOW);
      delayMicroseconds(1250);
      //Serial.println("Step: " + String(i));
    }
    //Serial.println("EndMove");
  }
}

void rotate(ESPRotary& r) {
  //Serial.println(r.getPosition());
  int position = r.getPosition();
  if (position == 0) {
    enableDisable(HIGH);
  } else {
    enableDisable(LOW);
  }

  direction = position > 0 ? 0 : 1;
  digitalWrite(DIR, direction);
  speed = abs(position);

  //Serial.println("Position: " + String(r.getPosition()) + ", Direction: " + String(r.getDirection()) + ", Speed: " + speed + ", Direction: " + direction);
}

void buttonPress(const int state) {
  if (state == HIGH) {
    reset();
  }
}

void reset() {
  knob.resetPosition();
  speed = 0;
  enableDisable(HIGH);
  //Serial.println("PRESSED, SPEED: " + String(speed) + ", DIR: " + String(direction));
}

void enableDisable(byte state) {
  enable = state;
  digitalWrite(ENABLE, state);
  //Serial.println("ENABLE: " + String(state));
}

int readPotentiometer() {
  int adc = analogRead(0);
  return adc;
}

void operModeButtonState(const int buttonState) {
  //Serial.println("MODE CHANGE PRESSED: " + buttonState);
  operMode = buttonState;
  reset();
}


/*void turnHandlerMultiply(long turn, ESPRotary& r) {
  Serial.println(r.directionToString(r.getDirection()));
  if (speed == 0) {
    enableDisable(LOW);
    direction = turn;
    speed = 1;
  } else if (direction == turn) {
    speed *= 2;
  } else {
    speed /= 2;
  }

  if (speed == 0 && enable == LOW) {
    enableDisable(HIGH);
  }

  digitalWrite(DIR, direction);
  Serial.println("SPEED: " + String(speed) + ", DIR: " + String(direction));
  }*/
