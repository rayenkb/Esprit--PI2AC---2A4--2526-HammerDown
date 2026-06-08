#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

#define SS_PIN 10
#define RST_PIN 9

MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo doorServo;

byte authorizedUID[] = {0x52, 0x08, 0x6A, 0xDE};

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  
  doorServo.attach(3);
  doorServo.write(0); // Door locked position
  
  Serial.println("Scan your card...");
}

void loop() {
  // Check for serial commands from Qt
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "GRANT") {
      openDoor();
    } else if (cmd == "DENY") {
      // Access denied
    }
  }

  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  String uidStr = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) uidStr += "0";
    uidStr += String(mfrc522.uid.uidByte[i], HEX);
  }
  uidStr.toUpperCase();
  
  // Send the UID to Qt for verification
  Serial.println("UID:" + uidStr);

  mfrc522.PICC_HaltA();
  delay(1000); // 1 second debounce to prevent spamming
}

void openDoor() {
  doorServo.write(90);   // Open position
  delay(5000);           // Door open 5 seconds
  doorServo.write(0);    // Close position
}