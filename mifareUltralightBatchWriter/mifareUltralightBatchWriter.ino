/* Batch card writer for Grimmboy - Arduino-controlled kids' audio player
 * By Turi Scandurra - https://turiscandurra.com/circuits
 * Works with Ntag213 (MiFare Ultralight C) tags
 */

#include <SPI.h>
#include <MFRC522.h>            // From https://github.com/miguelbalboa/rfid/

#define SS_PIN    10            // MFRC522 SDA
#define RST_PIN   9             // MFRC522 RST

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::StatusCode status;

byte buffer[16] = {
  0x34, 0x03, 0x0B, 0xD1,
  0x01, 0x07, 0x54, 0x02,
  0x65, 0x6E, 0x30, 0x30,
  0x30, 0x30, 0xFE, 0x00,
};

uint16_t currentNumber = 1; // Change this value if the first tag you want to write is not 0001
uint16_t tempNumber;
int numberArray[4];
char c;
int i;

void printCurrentCard(){
  Serial.print(F("Writing card number "));
  Serial.print(currentNumber);
  Serial.println();
}

void setup() {
  Serial.begin(9600); // Initialize serial communication
  SPI.begin(); // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card 
  printCurrentCard();
}

void loop() {
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent())
    return;

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial())
    return;

  // Copy each digit of the number to an array,
  // padding it with leading zeroes
  tempNumber = currentNumber;
  for (i = 0; i < 4; i++, tempNumber /= 10 ) {
    numberArray[i] = tempNumber % 10;
  }

  // Inject character into the buffer
  for (i=0; i < 4; i++) {
    c = (char(numberArray[3-i]) + 0x30); // Reformat character
    buffer[i+10] = c;
  }

  for (i=0; i < 4; i++) {
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Ultralight_Write(0x05+i, &buffer[i*4], 4);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Ultralight_Write() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return;
    }
  }
  
  Serial.println(F("Card write OK"));
  currentNumber += 1;
  printCurrentCard();

  delay(10);
  mfrc522.PICC_HaltA();

}