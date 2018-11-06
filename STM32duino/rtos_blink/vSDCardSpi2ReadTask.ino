#ifdef SDCARD_ENABLE

#include <SPI.h>
#include <SD.h>

Sd2Card card;
byte tmpBuf[512];
static void vSDCardSpi2ReadTask(void *pvParameters) 
{
vTaskDelay(1000);  
  
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
    while (1)vTaskDelay(5000);;
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

  for (;;) 
  {
    const TickType_t xTicksToWait = 100 / portTICK_PERIOD_MS;
    
      /* Wait a maximum of 100ms for either bit 0 or bit 4 to be set within
      the event group.  Clear the bits before exiting. */
      uxBits = xEventGroupWaitBits(
                xEventGroup,   /* The event group being tested. */
                RFID_DONE,      /* The bits within the event group to wait for. */
                pdTRUE,        /* RFID_DONE should be cleared before returning. */
                pdFALSE,       /* Don't wait for both bits, either bit will do. */
                xTicksToWait );/* Wait a maximum of 100ms for either bit to be set. */


        if(  uxBits & RFID_DONE == RFID_DONE )
        {
            /* xEventGroupWaitBits() returned because both bits were set. */
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

                vTaskDelay(1);  
              }
            }
        }
        else
        {
          //Serial.println("wait");

        }
  }
}

#endif
