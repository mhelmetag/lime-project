#include <ezButton.h>

#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

// Switches
ezButton lockedSwitch(2);
ezButton unlockingSwitch(3);

// NFC config
PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);

// Motor
int motorPin = 4;

void setup() {
  Serial.begin(115200);
  Serial.println("Fancy Lock V0.3");

  // Switch setup
  lockedSwitch.setDebounceTime(50);

  // NFC setup
  nfc.begin();
  nfc.setPassiveActivationRetries(0xFF);
  nfc.SAMConfig();

  // Motor
  pinMode(motorPin, OUTPUT);
}

void loop() {
  // NFC state
  // Valid NFC card data
  uint8_t validUid[] = { 131, 182, 44, 146, 0, 0, 0 };

  // State for NFC reading
  boolean success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  // State for NFC matching
  boolean match;

  if(lockedSwitch.getStateRaw() == LOW) {
    Serial.println("Locked!");

    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
    if (success) {
      Serial.println("Card read!");

      match = true;
      for (uint8_t i=0; i < 4; i++) {
        if (validUid[i] != uid[i]) {
          match = false;
          break;
        }
      }

      if (match) {
        Serial.println("Card match!");

        Serial.println("Unlocking in 3 seconds!");
        delay(3000);

        Serial.println("Unlocking...");
        digitalWrite(motorPin, HIGH);
        
        boolean unlocking = true;
        while (unlocking) {
          unlockingSwitch.loop();

          if (unlockingSwitch.isReleased()) {
            digitalWrite(motorPin, LOW);
            unlocking = false;
          }
        }
        
        Serial.println("Unlocked!");
        delay(3000);
      } else {
        Serial.println("Card mismatch!");
      }
    }
  }
}
