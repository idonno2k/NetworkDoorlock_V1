#ifdef SDCARD_ENABLE
#define DEBUG_SDCARD

#include <SPI.h>
#include <SD.h>

Sd2Card card;
SdVolume volume;
SdFile root;

String strAuthUid = "ABCDEFGH";
String strSyncPath = "SYNCDATE";
String strIpPath = "IP";
String strSetPath = "SET";
String strMacPath = "MAC";
String strSetDate = "20181219000000";
String strLogDate = "";

String strSyncDate = "";
String strSyncDateNew = "";


//byte tmpBuf[512];
void vSDCardSpi2ReadTask_setup(void)
{

#ifdef DEBUG_SDCARD
  Serial.print(F("Initializing SD card...\n"));
#endif

  if (!SD.begin(PB12))
  {
#ifdef DEBUG_SDCARD
    Serial.println("initialization failed!");
#endif
    while (1);
  }

  if (!card.init(SPI_HALF_SPEED, PB12))
  {
#ifdef DEBUG_SDCARD
    Serial.println(F("initialization failed. Things to check:"));
    Serial.println(F("* is a card inserted?"));
    Serial.println(F("* is your wiring correct?"));
    Serial.println(F("* did you change the chipSelect pin to match your shield or module?"));
#endif
    while (1);
  }
  else
  {
#ifdef DEBUG_SDCARD
    Serial.println(F("Wiring is correct and a card is present."));
#endif
  }

#ifdef DEBUG_SDCARD
  Serial.println();
  Serial.print("Card type:         ");
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:      Serial.println("SD1");      break;
    case SD_CARD_TYPE_SD2:      Serial.println("SD2");      break;
    case SD_CARD_TYPE_SDHC:     Serial.println("SDHC");     break;
    default:      Serial.println("Unknown");
  }
#endif

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card))
  {
#ifdef DEBUG_SDCARD
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
#endif
    while (1);
  }
}

void vSDCardFolder(String ns )
{
  //Serial.println(ns);
  if (SD.exists(ns))
  {
#ifdef DEBUG_SDCARD
    //Serial.println("folder ok");
#endif
  }
  else
  {
#ifdef DEBUG_SDCARD
    //Serial.println("folder no");
    SD.mkdir(ns);
#endif
  }
}
void vSDCardFile(String ns , String divi)
{
  //Serial.println(divi);
  if (divi == "79")
  {
    //Serial.println("y");
    if (SD.exists(ns))
    {
      //Serial.println("folder ok");
    }
    else
    {
      File myFile;
      myFile = SD.open(ns, FILE_WRITE);
      myFile.println("y");
      myFile.close();
    }
  }
  else
  {
    //Serial.println("n");
    SD.remove(ns);
  }
}


void vSDCardSetParmLoad( )
{
  File myFile;

  strSetDate = "";
  if (SD.exists(strMacPath))
  {
    myFile = SD.open(strMacPath);
    if (myFile)
    {
      while (myFile.available())
      {
        char tmpchar = (char)(myFile.read());
        if (tmpchar != '\r')
          strSetDate += tmpchar;
      }
      //strSetDate += "\0";
      myFile.close();

      String sss ;
      sss = strSetDate.substring(0, 2);  mymac[0] = (uint8_t)strtoul( sss.c_str(), NULL, 16);
      sss = strSetDate.substring(3, 5);  mymac[1] = (uint8_t)strtoul( sss.c_str(), NULL, 16);
      sss = strSetDate.substring(6, 8);  mymac[2] = (uint8_t)strtoul( sss.c_str(), NULL, 16);
      sss = strSetDate.substring(9, 11); mymac[3] = (uint8_t)strtoul( sss.c_str(), NULL, 16);
      sss = strSetDate.substring(12, 14); mymac[4] = (uint8_t)strtoul( sss.c_str(), NULL, 16);
      sss = strSetDate.substring(15, 17); mymac[5] = (uint8_t)strtoul( sss.c_str(), NULL, 16);

    }
  }

  strSetDate = "";
  if (SD.exists(strIpPath))
  {
    myFile = SD.open(strIpPath);
    if (myFile)
    {
      while (myFile.available())
      {
        char tmpchar = (char)(myFile.read());
        if (tmpchar != '\r')
          strSetDate += tmpchar;
      }
      //strSetDate += "\0";
      myFile.close();
    }


    String cc = strSetDate.substring(0, 1); delay(10);
    static_IP = (uint8_t)strtoul( cc.c_str(), NULL, 10); delay(10);
#ifdef DEBUG_SDCARD
    Serial.println(static_IP);
#endif

    if (static_IP == 1)
    {
      uint16_t offset = 2;        uint16_t end_offset;        String ip ;
      uint16_t offset1 = 0;        uint16_t end_offset1;   String ip1 ;
      //Serial.println(strSetDate);

      end_offset = strSetDate.indexOf("\n", offset);  ip = strSetDate.substring(offset, end_offset); ether.parseIp(myip , (char*)ip.c_str()); offset =  end_offset + 1;
      end_offset = strSetDate.indexOf("\n", offset);  ip = strSetDate.substring(offset, end_offset); ether.parseIp(maskip , (char*)ip.c_str()); offset = end_offset + 1;
      end_offset = strSetDate.indexOf("\n", offset);  ip = strSetDate.substring(offset, end_offset); ether.parseIp(gwip , (char*)ip.c_str()); offset = end_offset + 1;
      end_offset = strSetDate.indexOf("\n", offset);  ip = strSetDate.substring(offset, end_offset); ether.parseIp(dnsip , (char*)ip.c_str());
    }

  }

  strSetDate = "";
  if (SD.exists(strSetPath))
  {
    myFile = SD.open(strSetPath);
    if (myFile)
    {
      while (myFile.available())
      {
        char tmpchar = (char)(myFile.read());
        if (tmpchar != '\r')
          strSetDate += tmpchar;
      }
      //strSetDate += "\0";
      myFile.close();

      //Serial.println(strSetDate);

      uint16_t from = 0;      uint16_t to;      String s0 = "";
      //to = strSetDate.indexOf("\n",from);  s0 = strSetDate.substring(from,to); from =  to + 1; //Serial.println(s0);

      //uint16_t idx = 0;      uint16_t edx;      String s1="";
      //edx = s0.indexOf("/",idx);     s1 = s0.substring(idx , edx + 1);         s1.toCharArray(strWebSite,edx + 1); idx =  edx; //Serial.print(website);
      //edx = s0.indexOf("\n",idx);  s1 = s0.substring(idx, edx + 1);   s1.toCharArray((suburl),edx + 1);   //Serial.println((suburl));

      to = strSetDate.indexOf("\n", from);  s0 = strSetDate.substring(from, to); from = to + 1;	strWebSite = s0;	//Serial.println(strWebSite);
      to = strSetDate.indexOf("\n", from);  s0 = strSetDate.substring(from, to); from = to + 1; strSubSyncUrl = strSubLogUrl = s0; //Serial.println(arrDeviceSerial);
      to = strSetDate.indexOf("\n", from);  s0 = strSetDate.substring(from, to); from = to + 1; strSubSyncUrl += s0; //Serial.println(strSubSyncUrl);
      to = strSetDate.indexOf("\n", from);  s0 = strSetDate.substring(from, to); from = to + 1; strSubLogUrl += s0; //Serial.println(strSubLogUrl);

      to = strSetDate.indexOf("\n", from);  s0 = strSetDate.substring(from, to); from = to + 1; strDeviceName = s0; //Serial.println(arrDeviceName);
      to = strSetDate.indexOf("\n", from);  s0 = strSetDate.substring(from, to); from = to + 1; strDeviceSerial = s0; //Serial.println(arrDeviceSerial);
      to = strSetDate.indexOf("\n", from);  s0 = strSetDate.substring(from, to); RelayONTime = (uint16_t)(strtoul( s0.c_str(), NULL, 10));  from = to + 1; //Serial.println(RelayONTime);
      //to = strSetDate.indexOf("\0",from-5);
      s0 = strSetDate.substring(from, from + 2); FireVoltage = (uint16_t)strtoul( s0.c_str(), NULL, 10); //Serial.println(FireVoltage);

    }
  }

}




void vSDCardLogData( )
{

  //char arr_logdata[128];
  //char arr_loguid[20];
  //char arr_logfilename[128];

  //rtclock.breakTime(rtclock.now(), TimeStamp);
  //sprintf(arr_logfilename, "log_%u%u%u",logTimeStamp.year+1970, logTimeStamp.month, logTimeStamp.day, logTimeStamp.hour);
  //sprintf(arr_logdata, "%s %u %u, %s, %02u:%02u:%02u : ", months[logTimeStamp.month], logTimeStamp.day, logTimeStamp.year+1970,
  //	                                                  weekdays[logTimeStamp.weekday], logTimeStamp.hour, logTimeStamp.minute, logTimeStamp.second);
  //sprintf(arr_loguid, "%02X%02X%02X%02X", uid[0],uid[1],uid[2],uid[3]);

  String str_logFolder = "LOG";
  //String str_logFile = strLogDate;
  String str_logFile = strLogDate.substring(8, 14);
  //String str_logData = strLogDate + "-" + strLogUID;
  String str_logData = "?log=" + strLogDate + "&rf=" + strLogUID;
  str_logData += "&dn=" + strDeviceName + "&ds=" + strDeviceSerial;

#ifdef DEBUG_SDCARD
  Serial.print("str_logFile=");
  Serial.println(str_logFile);
  Serial.print("str_logData=");
  Serial.println(str_logData);
#endif

  vSDCardFolder(str_logFolder);

  //Serial.println(str_logFolder +"/"+ str_logFile);
  File myFile;
  myFile = SD.open(str_logFolder + "/" + str_logFile, FILE_WRITE);
  myFile.seek(myFile.size());
  myFile.println(str_logData.c_str());
  myFile.close();


}



void vSDCardLogRead(String logPath )
{
  File myFile;

  //   rtclock.breakTime(rtclock.now(), TimeStamp);
  //   sprintf(logPath, "log_%u%u%u",TimeStamp.year+1970, TimeStamp.month, TimeStamp.day, TimeStamp.hour);

  if (SD.exists(logPath))
  {
    myFile = SD.open(logPath);
    strLogDate = "";
    if (myFile)
    {
      // read from the file until there's nothing else in it:
      while (myFile.available())
      {
        char tmpchar = (char)(myFile.read());
        if (tmpchar != '\r')
          strLogDate += tmpchar;
      }
      myFile.close();
    }
  }

}

void vSDCardSyncDate( String sDate )
{
  if (SD.exists(strSyncPath))
    SD.remove(strSyncPath);

  File myFile;
  myFile = SD.open(strSyncPath, FILE_WRITE);
  myFile.print(sDate);
  myFile.close();
}

void vSDCardSyncDateLoad( )
{
  File myFile;
  //int StringDec;
  //char arrUIDchar;

  strSyncDate = "";
  if (SD.exists(strSyncPath))
  {
    myFile = SD.open(strSyncPath);
    strSyncDate = "";
    if (myFile)
    {
      // read from the file until there's nothing else in it:
      while (myFile.available())
      {
        char tmpchar = (char)(myFile.read());
        if (tmpchar != '\r')
          strSyncDate += tmpchar;
      }

      myFile.close();
    }
  }

}


void vSDCardUidDataLoad(uint8_t * uid )
{
  File myFile;
  char arrUIDchar[20];
  sprintf(arrUIDchar, "%02X%02X%02X%02X", uid[0], uid[1], uid[2], uid[3]);

  int StrDec;
  char StrChar;

  String folder01 = String(arrUIDchar).substring(0, 2);
  String folder02 = String(arrUIDchar).substring(2, 5);
  String folderFile = String(arrUIDchar).substring(2, 8);
  //String FileState = String(sa);

  // vSDCardFolder(folder01 + "/" + folder02);
  // vSDCardFile(folder01 + "/" + folder02 + "/" + folderFile , FileState);

  String pathStr = folder01 + "/" + folder02 + "/" + folderFile;

  //Serial.println((const char*)pathStr.c_str());

  //setEvent(&ActiveEvent , RFID_DONE);

  RelayOntime = millis() + RelayONTime;
  clearEvent(&ActiveEvent , RFID_DONE);
  //digitalWrite(RELAY_PIN, HIGH);

  if (SD.exists(pathStr))
  {
    strAuthUid = "";
    myFile = SD.open(pathStr);

    if (myFile)
    {
      // read from the file until there's nothing else in it:
      while (myFile.available())
      {
        char tmpchar = (char)(myFile.read());
        if (tmpchar != '\r')
          strAuthUid += tmpchar;
      }
      myFile.close();
    }

#ifdef DEBUG_SDCARD
    Serial.println((const char*)strAuthUid.c_str());
#endif

    setEvent(&ActiveEvent , RFID_DONE);
    tone(BUZZ_PIN, 1000, 100);
    digitalWrite(LED2_PIN, HIGH);
    delay(150);
    digitalWrite(LED2_PIN, LOW);

  }
  else
  {
    tone(BUZZ_PIN, 1000, 100);
    digitalWrite(LED1_PIN, HIGH);
    delay(150);
    tone(BUZZ_PIN, 1000, 100);
    digitalWrite(LED1_PIN, LOW);

#ifdef DEBUG_SDCARD
    //Serial.println("permission denied...");
#endif

  }
}


#endif
