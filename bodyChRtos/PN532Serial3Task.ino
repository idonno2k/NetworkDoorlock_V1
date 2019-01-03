#ifdef PN532_ENABLE
#define DEBUG_PN532

#include <PN532_HSU.h>
#include <PN532.h>

//static uint32_t RfidTimer;

PN532_HSU pn532hsu(Serial3);
PN532 nfc(pn532hsu);

void vPN532Serial3Task_setup(void) 
{
  
  #ifdef DEBUG_PN532
  Serial.println("PN532_serial3_setup...");
  #endif
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) 
  {
    Serial.println("Didn't find PN53x board");
    while (1)// halt
    {
      vTaskDelay(5000);
     }
  }
  
  // Got ok data, print it out!
  #ifdef DEBUG_PN532
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  #endif

  nfc.setPassiveActivationRetries(0xFF);
  
  // configure board to read RFID tags
  nfc.SAMConfig();
  #ifdef DEBUG_PN532
  Serial.println("Waiting for an ISO14443A card");
  #endif
}


void vPN532Serial3Task(void) 
{
    boolean success;
    static uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
    uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

	//===============================================================
	//rfid loop 500ms
	//if (millis() > RfidTimer) 
	{
		digitalWrite(LED1_PIN, LOW);
		success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength,50);
//uid[0]=0x89;
//uid[1]=0xAB;
//uid[2]=0xCD;
//uid[3]=0xEF;

//success = true;
		if (success) 
		{
		#ifdef DEBUG_PN532
			Serial.println("Found a card!");
			Serial.print("UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
			Serial.print("UID Value: ");
			for (uint8_t i=0; i < uidLength; i++) 
			{
				Serial.print(" 0x");Serial.print(uid[i], HEX); 
			}
			Serial.println("");
		#endif

		#ifdef DEBUG_PN532
			// Serial.print(rfid_event, HEX);
		#endif


     rtclock.breakTime(rtclock.now(), logTimeStamp);
     vSDCardUidDataLoad(uid) ;

      char arr_logdata[20]; 
      char arr_loguid[10]; 
      
      sprintf(arr_logdata, "%u%02u%02u%02u%02u%02u",  logTimeStamp.year+1970, logTimeStamp.month, logTimeStamp.day,logTimeStamp.hour, logTimeStamp.minute, logTimeStamp.second);
      sprintf(arr_loguid, "%02X%02X%02X%02X", uid[0],uid[1],uid[2],uid[3]);
      
      strLogDate = arr_logdata;
      strLogUID = arr_loguid;
 
      LogAckFlag = true;
      
      //Serial.println(EtherStep);
      if(EtherStep == SyncIdle)
      {
        etherLogData( ) ;
        vTaskDelay(500);//wait log_callback timeout
      }
      
      if(LogAckFlag == true)
      {
       vSDCardLogData() ;
       LogAckFlag = false;
      }
		}
		else
		{
		  #ifdef DEBUG_PN532
		  // PN532 probably timed out waiting for a card
		  //Serial.println("Timed out waiting for a card");
		  #endif
		}
	#ifdef DEBUG_PN532
		//Serial.print(".");
  #endif
digitalWrite(LED1_PIN, HIGH);
	}                 // wait for a second
 
}

static void vPN532TaskLoop(void *pvParameters) 
{

    vPN532Serial3Task_setup();      
    vTaskDelay(100);
  
    for (;;) 
    {
      vPN532Serial3Task(); 
      vTaskDelay(500);
    }
}


#endif
