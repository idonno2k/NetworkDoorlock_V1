#ifdef PN532_ENABLE

#include <PN532_HSU.h>
#include <PN532.h>

PN532_HSU pn532hsu(Serial3);
PN532 nfc(pn532hsu);

#define RFID_DONE ( 1 << 0 )

static void vPN532Serial3Task(void *pvParameters) 
{
  vTaskDelay(3000);
  #ifdef DEBUG
  Serial.println("PN532_serial3_setup...");
  #endif
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) 
  {
      Serial.print("Didn't find PN53x board");
      while (1)vTaskDelay(5000); // halt
  }
 
  // Got ok data, print it out!
  #ifdef DEBUG
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  #endif
  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
  nfc.setPassiveActivationRetries(0xFF);
  
  // configure board to read RFID tags
  nfc.SAMConfig();
  #ifdef DEBUG
  Serial.println("Waiting for an ISO14443A card");
  #endif
  for (;;) 
  {
    
    boolean success;
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
    uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    
    // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
    // 'uid' will be populated with the UID, and uidLength will indicate
    // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
  
    if (success) 
    {
      #ifdef DEBUG
      Serial.println("Found a card!");
      Serial.print("UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
      Serial.print("UID Value: ");
      for (uint8_t i=0; i < uidLength; i++) 
      {
        Serial.print(" 0x");Serial.print(uid[i], HEX); 
      }
      Serial.println("");
      #endif
      /* Set bit 0 and bit 4 in xEventGroup. */
      uxBits = xEventGroupSetBits( xEventGroup, RFID_DONE );/* The bits being set. */
      
      // Wait 1 second before continuing
      vTaskDelay(1000); 
    }
    else
    {
      #ifdef DEBUG
      // PN532 probably timed out waiting for a card
      //Serial.println("Timed out waiting for a card");
      #endif
    }
    
  }
}

#endif
