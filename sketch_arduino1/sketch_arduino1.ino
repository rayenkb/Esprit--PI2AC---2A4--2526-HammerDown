#include <Arduino.h>

const int segPins[] = {2, 3, 4, 5, 6, 7, 8}; // a, b, c, d, e, f, g
const int buzzerPin = 10;
const int buttonB1 = A2;
const int buttonB2 = 13;
const int buttonB3 = 11;

byte digits[10][7] = {
  {0, 0, 0, 0, 0, 0, 1}, {1, 0, 0, 1, 1, 1, 1}, {0, 0, 1, 0, 0, 1, 0},
  {0, 0, 0, 0, 1, 1, 0}, {1, 0, 0, 1, 1, 0, 0}, {0, 1, 0, 0, 1, 0, 0},
  {0, 1, 0, 0, 0, 0, 0}, {0, 0, 0, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 1, 0, 0}
};

int lastB1State = HIGH;
int lastB2State = HIGH;
int lastB3State = HIGH;
unsigned long b1PressStart = 0;
unsigned long b2PressStart = 0;
unsigned long b3PressStart = 0;
bool b1Pressed = false;
bool b2Pressed = false;
bool b3Pressed = false;
const unsigned long debounceDelay = 50;
const unsigned long longPressThreshold = 800; // 800ms for long press

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 7; i++) {
    pinMode(segPins[i], OUTPUT);
    digitalWrite(segPins[i], HIGH);
  }
  pinMode(buzzerPin, OUTPUT);
  
  pinMode(buttonB1, INPUT_PULLUP);
  pinMode(buttonB2, INPUT_PULLUP);
  pinMode(buttonB3, INPUT_PULLUP);
}

void loop() {
  // Check for serial commands from Qt
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    
    if (cmd.startsWith("TOTAL:")) {
      int val = cmd.substring(6).toInt();
      displayMultiDigit(val);
    } 
    else if (cmd.startsWith("STEP:")) {
      int val = cmd.substring(5).toInt();
      displayMultiDigit(val);
    }
    else if (cmd.startsWith("CYCLE:")) {
      // Format: CYCLE:ID1,ID2,ID3,... (cycle through IDs on display)
      String ids = cmd.substring(6);
      int startIdx = 0;
      int commaIdx = ids.indexOf(',');
      
      while (startIdx < ids.length()) {
        String idStr;
        if (commaIdx == -1) {
          idStr = ids.substring(startIdx);
          startIdx = ids.length();
        } else {
          idStr = ids.substring(startIdx, commaIdx);
          startIdx = commaIdx + 1;
          commaIdx = ids.indexOf(',', startIdx);
        }
        
        int id = idStr.toInt();
        unsigned long t = millis();
        while(millis() - t < 1500) {
          for (int i = 0; i < 7; i++) {
            if (digits[id % 10][i] == 0) {
              digitalWrite(segPins[i], LOW);
              delayMicroseconds(200);
              digitalWrite(segPins[i], HIGH);
              delayMicroseconds(800);
            }
          }
        }
        delay(300); // Pause between IDs
      }
    }
    else if (cmd == "FINAL") {
      tone(buzzerPin, 2000);
      unsigned long t = millis();
      while(millis() - t < 1000) { }
      noTone(buzzerPin);
      clearDisplay();
    }
    else if (cmd == "ALARM") {
      // Alert buzzer - 3 short beeps for warnings
      for (int i = 0; i < 3; i++) {
        tone(buzzerPin, 2500);
        delay(150);
        noTone(buzzerPin);
        delay(100);
      }
    }
  }

  // Read buttons with short/long press detection
  int b1State = digitalRead(buttonB1);
  int b2State = digitalRead(buttonB2);
  int b3State = digitalRead(buttonB3);
  unsigned long now = millis();

  // Button B1 - Equipment Manager (Short=Available, Long=My Borrowed)
  if (b1State == LOW && lastB1State == HIGH && (now - b1PressStart > debounceDelay)) {
    b1PressStart = now;
    b1Pressed = true;
  }
  if (b1State == HIGH && lastB1State == LOW && b1Pressed) {
    b1Pressed = false;
    unsigned long pressDuration = now - b1PressStart;
    if (pressDuration < longPressThreshold) {
      Serial.println("B1:SHORT"); // Show available equipment
    } else {
      Serial.println("B1:LONG");  // Show my borrowed + check overdue
    }
  }
  lastB1State = b1State;

  // Button B2 - Attendance (Short=Total Present, Long=My Status)
  if (b2State == LOW && lastB2State == HIGH && (now - b2PressStart > debounceDelay)) {
    b2PressStart = now;
    b2Pressed = true;
  }
  if (b2State == HIGH && lastB2State == LOW && b2Pressed) {
    b2Pressed = false;
    unsigned long pressDuration = now - b2PressStart;
    if (pressDuration < longPressThreshold) {
      Serial.println("B2:SHORT"); // Total employees checked in
    } else {
      Serial.println("B2:LONG");  // My attendance status
    }
  }
  lastB2State = b2State;

  // Button B3 - Maintenance (Short=Urgent Count, Long=Report Problem)
  if (b3State == LOW && lastB3State == HIGH && (now - b3PressStart > debounceDelay)) {
    b3PressStart = now;
    b3Pressed = true;
  }
  if (b3State == HIGH && lastB3State == LOW && b3Pressed) {
    b3Pressed = false;
    unsigned long pressDuration = now - b3PressStart;
    if (pressDuration < longPressThreshold) {
      Serial.println("B3:SHORT"); // Count needing maintenance
    } else {
      Serial.println("B3:LONG");  // Report problem at station
    }
  }
  lastB3State = b3State;
}

void displaySafe(int num) {
  if (num < 0 || num > 9) return;
  unsigned long t = millis();
  while(millis() - t < 2000) {
    for (int i = 0; i < 7; i++) {
      if (digits[num][i] == 0) {
        digitalWrite(segPins[i], LOW);
        delayMicroseconds(200);
        digitalWrite(segPins[i], HIGH);
        delayMicroseconds(800);
      }
    }
  }
  // Buzzer removed - now only conditional via ALARM command
}

void clearDisplay() {
  for(int i=0; i<7; i++) digitalWrite(segPins[i], HIGH);
}

void displayMultiDigit(int num) {
  // Display numbers > 9 sequentially (e.g., 12 shows 1 then 2)
  if (num < 0) return;
  if (num <= 9) {
    displaySafe(num);
    return;
  }
  
  // For numbers > 9, show each digit for 800ms
  String numStr = String(num);
  for (int i = 0; i < numStr.length(); i++) {
    int digit = numStr[i] - '0';
    unsigned long t = millis();
    while(millis() - t < 800) {
      for (int j = 0; j < 7; j++) {
        if (digits[digit][j] == 0) {
          digitalWrite(segPins[j], LOW);
          delayMicroseconds(200);
          digitalWrite(segPins[j], HIGH);
          delayMicroseconds(800);
        }
      }
    }
    delay(200); // Brief pause between digits
  }
}
