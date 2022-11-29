/* GRIMMBOY
 * Arduino-controlled kids' audio player
 * By Turi Scandurra - https://turiscandurra.com/circuits
 * v1.0.0 - 2022.10.25
 * Inspired by Tonuino (https://www.tonuino.de/)
 *
 * Written for Arduino Nano or Arduino Uno.
 */

#include <SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>            // From https://github.com/miguelbalboa/rfid/
#include <DFPlayerMini_Fast.h>  // From https://github.com/PowerBroker2/DFPlayerMini_Fast
#include <Button.h>             // From https://github.com/madleech/Button
#include <FireTimer.h>          // From https://github.com/PowerBroker2/FireTimer

#define RXPIN     2             // DFPlayer RX
#define TXPIN     3             // DFPlayer TX
#define BTNPIN    A0            // Push button
#define LEDPIN    6             // Status LED
#define SS_PIN    10            // MFRC522 SDA
#define RST_PIN   9             // MFRC522 RST

// Initialization
SoftwareSerial softSerial(2, 3);

Button playPauseButton(BTNPIN);

FireTimer ledTimer;

DFPlayerMini_Fast mp3;

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::StatusCode status;
byte buffer[18];
byte size = sizeof(buffer);
char charBuffer[16];

bool ledStatus = LOW;
long trackId;

void setup()
{
  Serial.begin(115200);
  Serial.println(F("Taleduino v1.0.0"));

	playPauseButton.begin();
  pinMode(LEDPIN, OUTPUT);

  softSerial.begin(9600);
  mp3.begin(softSerial); // Add true as a second parameter for debug info
  mp3.volume(255);
  mp3.playFromMP3Folder(1); // Play startup sound

  SPI.begin(); // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card 
}

void readCard() {
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent())
    return;

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial())
    return;

  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(0x05, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    mp3.playFromMP3Folder(2); // Play an error tone
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Read the track id from the tag
  for (byte i = 0; i < 4; i++) {
    charBuffer[i] = buffer[i + 10];
  }
  charBuffer[4] = 0; // Terminate string

  String trackIdString = String(charBuffer); // Convert char into String
  trackId = trackIdString.toInt();

  if(trackId <= 0){
    Serial.print(F("No valid track number found on card"));
    Serial.println();
    mp3.playFromMP3Folder(2); // Play an error tone
    delay(2000);
    return;
  } else if(trackId > mp3.numSdTracks()){
    Serial.print(F("Track number "));
    Serial.print(charBuffer);
    Serial.print(F(" does not exist"));
    Serial.println();
    mp3.playFromMP3Folder(2); // Play an error tone
    delay(2000);
    return;
  }

  // Track exists, play it
  Serial.print(F("Playing track number "));
  Serial.print(charBuffer);
  Serial.println();
  mp3.play(trackId);
  digitalWrite(LEDPIN, LOW);

  mfrc522.PICC_HaltA();
}

void loop() {
  // Play/pause button routine
	if (playPauseButton.pressed()){
		if(mp3.isPlaying()){
        mp3.pause();
      } else {
        mp3.resume();
        ledStatus = LOW;
        digitalWrite(LEDPIN, ledStatus);
      }
  }

  if(ledTimer.fire() && !mp3.isPlaying()){ // If timer has expired and no track is playing
    ledStatus = !ledStatus; // Flip the LED status
    digitalWrite(LEDPIN, ledStatus);
    ledTimer.begin(500); // Reset the timer
  }

  readCard();
}
