#ifdef P08I_ENABLE
#define DEBUG_08I

#define mySerial Serial3
static uint32_t RfidTimer;
String CardNo;
unsigned long CardNoD;
String CardNo_Hex;

void vSerial3Task_setup(void)
{

  mySerial.begin(9600, SERIAL_8N1);
#ifdef DEBUG_08I
  Serial.print("Serial Port Open");
#endif
}

void vSerial3Task(void)
{
  iwdg_feed();

  if (millis() > RfidTimer)
  {
    while (mySerial.available())
    {
      int StringDec = (int)(mySerial.read());
      if (StringDec != 10 or StringDec != 13) {
        CardNo += (char)StringDec;
      }
    }
    if (CardNo != "") {
#ifdef DEBUG_08I
      //Serial.println(CardNo);
#endif

      CardNoD = (unsigned long)atof(CardNo.c_str());


      static uint8_t uid[] = { 0, 0, 0, 0 };
      CardNo_Hex = String(CardNoD, HEX);
      CardNo_Hex.toUpperCase();

      //sprintf(CardNo_Hex, "08x", CardNo_Hex);

      for (uint8_t i = CardNo_Hex.length() ; i < 8; i++)
      {
        CardNo_Hex = "0" + CardNo_Hex;
      }

      //Serial.println(CardNo_Hex);
      String sss ;
      sss = CardNo_Hex.substring(0, 2);  uid[0] = (uint8_t)strtoul( sss.c_str(), NULL, 16);
      sss = CardNo_Hex.substring(2, 4);  uid[1] = (uint8_t)strtoul( sss.c_str(), NULL, 16);
      sss = CardNo_Hex.substring(4, 6);  uid[2] = (uint8_t)strtoul( sss.c_str(), NULL, 16);
      sss = CardNo_Hex.substring(6, 8);  uid[3] = (uint8_t)strtoul( sss.c_str(), NULL, 16);

#ifdef DEBUG_08I
      for (uint8_t i = 0; i < 4; i++)
      {
        Serial.print(" 0x"); Serial.print(uid[i], HEX);
      }
#endif

      rtclock.breakTime(rtclock.now(), logTimeStamp);
      vSDCardUidDataLoad(uid) ;


      char arr_logdata[20];
      char arr_loguid[10];

      sprintf(arr_logdata, "%u%02u%02u%02u%02u%02u",  logTimeStamp.year + 1970, logTimeStamp.month, logTimeStamp.day, logTimeStamp.hour, logTimeStamp.minute, logTimeStamp.second);
      sprintf(arr_loguid, "%02X%02X%02X%02X", uid[0], uid[1], uid[2], uid[3]);

      strLogDate = arr_logdata;
      strLogUID = arr_loguid;

      LogAckFlag = true;

      if (EtherStep == SyncIdle)
      {

        etherLogData( ) ;

        // vTaskDelay(500);//wait log_callback timeout
        LogTimeOut = millis() + 300;
      }
      
    }
    CardNo = "";
  
    RfidTimer = millis() + 1000;
  }

  if (millis() > LogTimeOut)
  {
    if (LogAckFlag == true)
    {
      vSDCardLogData() ;
      LogAckFlag = false;
    }
  }

}

#endif
