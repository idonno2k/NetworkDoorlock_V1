/*
 * --------------------------------------------------------------------------------------------------------------------
 * Example sketch/program showing how to read new NUID from a PICC to serial.
 * --------------------------------------------------------------------------------------------------------------------
 * This is a MFRC522 library example; for further details and other examples see: https://github.com/miguelbalboa/rfid
 * 
 * Example sketch/program showing how to the read data from a PICC (that is: a RFID Tag or Card) using a MFRC522 based RFID
 * Reader on the Arduino SPI interface.
 * 
 * When the Arduino and the MFRC522 module are connected (see the pin layout below), load this sketch into Arduino IDE
 * then verify/compile and upload it. To see the output: use Tools, Serial Monitor of the IDE (hit Ctrl+Shft+M). When
 * you present a PICC (that is: a RFID Tag or Card) at reading distance of the MFRC522 Reader/PCD, the serial output
 * will show the type, and the NUID if a new card has been detected. Note: you may see "Timeout in communication" messages
 * when removing the PICC from reading distance too early.
 * 
 * @license Released into the public domain.
 * 
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 */

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9

int speakerpin = 7; //부저가 연결된 디지털핀 설정
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

// Init array that will store new NUID 
byte nuidPICC[4];
String rfid_uid_hex;
String rfid_uid_hex_r;
String rfid_uid_dec;
String rfid_uid_dec_r;
unsigned long uid_dec;

void setup() { 
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 
 
  tone(speakerpin,1000,200);  //200: 음의 높낮이(주파수), 1000: 음의 지속시간(1초)
  Serial.println("ok");
}

void loop() {
  
  // Look for new cards
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }
  
    printHex(rfid.uid.uidByte, rfid.uid.size);
    playTone();
    playTone();

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();

}


/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void printHex(byte *buffer, byte bufferSize) {
  
  rfid_uid_hex = "";
  rfid_uid_hex_r = "";
  rfid_uid_dec = "";
  rfid_uid_dec_r = "";

  for (byte i = 0; i < bufferSize; i++) {
 
    if ( buffer[i] < 0x10  ){
      rfid_uid_hex += "0"+String(buffer[i] , HEX);
      rfid_uid_hex_r = "0"+String(buffer[i] , HEX) + rfid_uid_hex_r;     
    }else{
      rfid_uid_hex += String(buffer[i] , HEX);
      rfid_uid_hex_r = String(buffer[i] , HEX) + rfid_uid_hex_r;
    }

  }

  rfid_uid_hex.toUpperCase();
  rfid_uid_hex_r.toUpperCase();
  Serial.println();
  Serial.print("Hex : ");
  Serial.print(rfid_uid_hex);
  Serial.println();
  Serial.print("Hex_r : ");
  Serial.print(rfid_uid_hex_r);
  Serial.println();

  char hex_tmp[] = {0};
  rfid_uid_hex.toCharArray(hex_tmp , rfid_uid_hex.length()+1 );
  uid_dec = (unsigned long)strtoul(hex_tmp, NULL, 16);

  Serial.print("Dec : ");
  Serial.print(uid_dec);
  Serial.println();

  rfid_uid_hex_r.toCharArray(hex_tmp , rfid_uid_hex_r.length()+1 );
  uid_dec = (unsigned long)strtoul(hex_tmp, NULL, 16);
  
  Serial.print("Dec_r : ");
  Serial.print(uid_dec);
  Serial.println();

}

void playTone() {
  tone(speakerpin,1000,100);
  delay(100);
}
