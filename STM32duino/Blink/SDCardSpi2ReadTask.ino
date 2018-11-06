#ifdef SDCARD_ENABLE

#include <SPI.h>
#include <SD.h>

Sd2Card card;
byte tmpBuf[512];
void vSDCardSpi2ReadTask_setup(void) 
{

#ifdef DEBUG
  Serial.println(F("SD Read Task..."));
  Serial.print(F("Initializing SD card...\n"));
  #endif
  
  if (!card.init(SPI_HALF_SPEED, PB12)) 
  {
    #ifdef DEBUG
    Serial.println(F("initialization failed. Things to check:"));
    Serial.println(F("* is a card inserted?"));
    Serial.println(F("* is your wiring correct?"));
    Serial.println(F("* did you change the chipSelect pin to match your shield or module?"));
    #endif
    while (1);
  } 
  else 
  {
    #ifdef DEBUG
    Serial.println(F("Wiring is correct and a card is present."));
    #endif
  }

  #ifdef DEBUG
  Serial.println();
  Serial.print("Card type:         ");
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:      Serial.println("SD1");      break;
    case SD_CARD_TYPE_SD2:      Serial.println("SD2");      break;
    case SD_CARD_TYPE_SDHC:     Serial.println("SDHC");     break;
    default:      Serial.println("Unknown");
    }
  #endif

}


void vSDCardSpi2ReadTask(void) 
{
  if((rfid_event & RFID_DONE) == RFID_DONE)
  {
    clearEvent(&rfid_event ,RFID_DONE);
    int readBlockIndex = 0;
    char s[7];
    if(card.readBlock (readBlockIndex, tmpBuf))
    {
      Serial.println("read success...\n");
      sprintf(s, "block %d",readBlockIndex);
      Serial.println(s);
      for(int i = 1;i<=512;i++)
      {
        sprintf(s, "%02x",tmpBuf[i-1]);
        Serial.print(s);
      
        if(i%16 == 0)Serial.print("\n");
        else Serial.print(" ");
  
      }
    }
    
  }
}

#endif
