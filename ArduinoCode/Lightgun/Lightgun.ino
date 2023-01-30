#include "Arduino.h"
#include "Transformation.h"
#include "DFRobotIRPosition.h"
#include "AbsMouse.h"
#include <Keyboard.h>
#include "Wire.h"

#define triggerA 10
#define triggerB 11
#define mag 14
#define lid 8
#define fireLED 9
#define warningLED 13

DFRobotIRPosition camera;
int positionX[] = { 1023, 1023, 1023, 1023 };
int positionY[] = { 1023, 1023, 1023, 1023 };

int cornersX[] = { 1023, 1023, 1023, 1023 };
int cornersY[] = { 1023, 1023, 1023, 1023 };

int button_triggerA = 0;
int button_triggerB = 0;
int button_mag = 0;
int button_lid = 0;

int warningLEDState = 0;

int screenW = 1920;
int screenH = 1080;

int gunCenterX = 1024 / 2 - 128;
int gunCenterY = 1024 / 2 - 128;

void setup() {

  pinMode(fireLED, OUTPUT);
  pinMode(warningLED, OUTPUT);
  pinMode(triggerA, INPUT_PULLUP);
  pinMode(triggerB, INPUT_PULLUP);
  pinMode(mag, INPUT_PULLUP);
  pinMode(lid, INPUT_PULLUP);

  Serial.begin(19200);
  AbsMouse.init(screenW, screenH);
  camera.begin();
  Keyboard.begin();

  digitalWrite(fireLED, LOW);
  digitalWrite(warningLED, LOW);
}

void loop() {
  handleButtons();
  if (getCameraData()) {
    sortPoints();
    moveCursor();
  }
}

void handleButtons() {
  int triggerA_now = digitalRead(triggerA);
  int triggerB_now = digitalRead(triggerB);
  int mag_now = digitalRead(mag);
  int lid_now = digitalRead(lid);

  if (triggerA_now != button_triggerA) {
    button_triggerA = triggerA_now;
    if (button_triggerA == 0) {
      Keyboard.press("1");
      digitalWrite(fireLED, HIGH);
    } else {
      Keyboard.release("1");
      digitalWrite(fireLED, LOW);
    }
  }

  if (triggerB_now != button_triggerB) {
    button_triggerB = triggerB_now;
    if (button_triggerB == 0) {
      Keyboard.press("2");
    } else {
      Keyboard.release("2");
    }
  }

  if (lid_now != button_lid) {
    button_lid = lid_now;
    if (button_lid == 0) {
      Keyboard.press("3");
    } else {
      Keyboard.release("3");
    }
  }

  if (mag_now != button_mag) {
    button_mag = mag_now;
    if (button_mag == 0) {
      Keyboard.press("4");
    }
  } else {
    Keyboard.release("4");
  }
}

bool getCameraData() {
  camera.requestPosition();
  if (camera.available()) {
    for (int i = 0; i < 4; i++) {
      positionX[i] = camera.readX(i);
      positionY[i] = camera.readY(i);
    }

    if (Serial.available()) {
      Serial.read();
      for (int i = 0; i < 4; i++) {
        Serial.print(positionX[i]);
        Serial.print(",");
        Serial.print(positionY[i]);
        if (i < 3)
          Serial.print(",");
      }
      Serial.println("");
    }
  }

  if (positionX[3] == 1023 && positionY[3] == 1023) {
    setWarningLED(1);
    return false;
  } else {
    setWarningLED(0);
    return true;
  }
}

void sortPoints() {
  int orderedX[] = { 0, 1, 2, 3 };

  for (int i = 0; i < 3; i++) {
    for (int j = i + 1; j < 4; j++) {
      if (positionX[orderedX[i]] < positionX[orderedX[j]]) {
        int temp = orderedX[i];
        orderedX[i] = orderedX[j];
        orderedX[j] = temp;
      }
    }
  }

  if (positionY[orderedX[0]] < positionY[orderedX[1]]) {
    cornersX[0] = positionX[orderedX[0]];
    cornersY[0] = positionY[orderedX[0]];
    cornersX[2] = positionX[orderedX[1]];
    cornersY[2] = positionY[orderedX[1]];
  } else {
    cornersX[0] = positionX[orderedX[1]];
    cornersY[0] = positionY[orderedX[1]];
    cornersX[2] = positionX[orderedX[0]];
    cornersY[2] = positionY[orderedX[0]];
  }

  if (positionY[orderedX[2]] < positionY[orderedX[3]]) {
    cornersX[1] = positionX[orderedX[2]];
    cornersY[1] = positionY[orderedX[2]];
    cornersX[3] = positionX[orderedX[3]];
    cornersY[3] = positionY[orderedX[3]];
  } else {
    cornersX[1] = positionX[orderedX[3]];
    cornersY[1] = positionY[orderedX[3]];
    cornersX[3] = positionX[orderedX[2]];
    cornersY[3] = positionY[orderedX[2]];
  }
}

void moveCursor() {
  Transformation trans(cornersX, cornersY, screenW, screenH, gunCenterX, gunCenterY);
  AbsMouse.move(trans.u(), trans.v());
}

void setWarningLED(int x) {
  if (x == 1) {
    digitalWrite(warningLED, HIGH);
  } else {
    digitalWrite(warningLED, LOW);
  }
}
