#ifdef SDCARD_ENABLE
#define DEBUG_SDCARD

#include <SPI.h>
#include <SD.h>

Sd2Card card;
SdVolume volume;
SdFile root;

//byte tmpBuf[512];
void vSDCardSpi2ReadTask_setup(void) 
{

	#ifdef DEBUG_SDCARD
	Serial.print(F("Initializing SD card...\n"));
	#endif

	if (!SD.begin(PB12))
	{
		Serial.println("initialization failed!");
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
		Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
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
	if(divi == "79")
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

String IpPath = "IP";
String SetPath = "SET";
String MacPath = "MAC";
String SetDateStr = "0"; 
void vSDCardSetDateLoad( ) 
{
	File myFile;

	SetDateStr = ""; 
	if (SD.exists(MacPath))  
	{
		myFile = SD.open(MacPath);
		if (myFile) 
		{
			while (myFile.available()) 
			{
				SetDateStr += (char)(myFile.read());
			}
			SetDateStr += "\0";
			myFile.close();

			String sss ;
			sss = SetDateStr.substring(0,2);  mymac[0] = (uint8_t)strtoul( sss.c_str(), NULL, 16);
			sss = SetDateStr.substring(3,5);  mymac[1] = (uint8_t)strtoul( sss.c_str(), NULL, 16);
			sss = SetDateStr.substring(6,8);  mymac[2] = (uint8_t)strtoul( sss.c_str(), NULL, 16);
			sss = SetDateStr.substring(9,11); mymac[3] = (uint8_t)strtoul( sss.c_str(), NULL, 16);
			sss = SetDateStr.substring(12,14);mymac[4] = (uint8_t)strtoul( sss.c_str(), NULL, 16);
			sss = SetDateStr.substring(15,17);mymac[5] = (uint8_t)strtoul( sss.c_str(), NULL, 16);

		} 
	}

  SetDateStr = ""; 
	if (SD.exists(IpPath))  
	{
  	myFile = SD.open(IpPath);
		if (myFile) 
		{
			while (myFile.available()) 
			{
				SetDateStr += (char)(myFile.read());
			}
			SetDateStr += "\0";
			myFile.close();
    }
    
   // String cc = SetDateStr.substring(0,1);delay(10);
    //static_IP = (uint8_t)strtoul( cc.c_str(), NULL, 10);delay(10);
    //Serial.println(static_IP);  
    if(static_IP == 1)
    {
       Serial.println("test");   
      //uint16_t offset = 1;        uint16_t end_offset;        String ip ;
      //end_offset = SetDateStr.indexOf("\n",offset);  ip = SetDateStr.substring(offset,end_offset); ether.parseIp(myip ,(char*)ip.c_str()); offset =  end_offset + 1;
      //end_offset = SetDateStr.indexOf("\n",offset);  ip = SetDateStr.substring(offset,end_offset); ether.parseIp(maskip ,(char*)ip.c_str()); offset = end_offset + 1;
      //end_offset = SetDateStr.indexOf("\n",offset);  ip = SetDateStr.substring(offset,end_offset); ether.parseIp(gwip ,(char*)ip.c_str()); offset = end_offset + 1;
      //end_offset = SetDateStr.indexOf("\0",offset);  ip = SetDateStr.substring(offset,end_offset); ether.parseIp(dnsip ,(char*)ip.c_str());
   	} 

	}

  SetDateStr = ""; 
	if (SD.exists(SetPath))  
	{
		myFile = SD.open(SetPath);
		if (myFile) 
		{
			while (myFile.available()) 
			{
				SetDateStr += (char)(myFile.read());
			}
			SetDateStr += "\0";
			myFile.close();

			//Serial.println(SetDateStr);   

			uint16_t from = 0;      uint16_t to;      String s0 = "";
			to = SetDateStr.indexOf("\n",from);  s0 = SetDateStr.substring(from,to); from =  to + 1; //Serial.println(s0);   

			uint16_t idx = 0;      uint16_t edx;      String s1="";
			edx = s0.indexOf("/",7);     s1 = s0.substring(idx + 7, edx);         s1.toCharArray(website,edx); idx =  edx; //Serial.print(website);   
			edx = s0.indexOf("\0",idx);  s1 = s0.substring(idx, edx + idx + 2);   s1.toCharArray(suburl,edx + 2);   //Serial.println(suburl);   

			to = SetDateStr.indexOf("\n",from);  s0 = SetDateStr.substring(from,to); s0.toCharArray(device_name,to);  from =  to + 1;    //Serial.println(device_name);   
			to = SetDateStr.indexOf("\n",from);  s0 = SetDateStr.substring(from,to); s0.toCharArray(device_serial,to);  from = to + 1;   //Serial.println(device_serial);  
			to = SetDateStr.indexOf("\n",from);  s0 = SetDateStr.substring(from,to); RelayONTime = (uint16_t)(strtoul( s0.c_str(), NULL, 10)*1000);  from = to + 1;  //Serial.println(RelayOpTime);  
			//to = SetDateStr.indexOf("\0",from-5);
			s0 = SetDateStr.substring(from,from + 2); FireVoltage = (uint16_t)strtoul( s0.c_str(), NULL, 10);  //Serial.println(FireVoltage);  

		} 
	}
 
}

String LogFolder = "LOG";
void vSDCardLogDate(uint8_t * uid ) 
{
  char logFilename[128]; 
  char logData[128]; 
  char StringChar[20]; 

   rtclock.breakTime(rtclock.now(), mtt);
   sprintf(logFilename, "log_%u%u%u",mtt.year+1970, mtt.month, mtt.day, mtt.hour);
   sprintf(logData, "%s %u %u, %s, %02u:%02u:%02u : ", months[mtt.month], mtt.day, mtt.year+1970, weekdays[mtt.weekday], mtt.hour, mtt.minute, mtt.second);
   sprintf(StringChar,"%02X%02X%02X%02X", uid[0],uid[1],uid[2],uid[3]);
   //Serial.print(logPath);

  vSDCardFolder("LogFolder");

  String logFile = logFilename;
  File myFile;
  myFile = SD.open(LogFolder +"/"+logFile, FILE_WRITE);
  myFile.seek(myFile.size());
  myFile.print(logData);
  myFile.print(StringChar);
  myFile.close();
}


void vSDCardLogRead(String logPath ) 
{
  File myFile;
 
//   rtclock.breakTime(rtclock.now(), mtt);
//   sprintf(logPath, "log_%u%u%u",mtt.year+1970, mtt.month, mtt.day, mtt.hour);

  if (SD.exists(logPath))  
  {
    myFile = SD.open(logPath);
    LogDateStr = "";
    if (myFile) 
    {
      // read from the file until there's nothing else in it:
      while (myFile.available()) 
      {
        LogDateStr += (char)(myFile.read());
      }
      myFile.close();
    } 
  }
  
}

String SyncPath = "SYNCDATE";
void vSDCardSyncDate( String sDate ) 
{
	if (SD.exists(SyncPath))
		SD.remove(SyncPath);
	
	File myFile;
	myFile = SD.open(SyncPath, FILE_WRITE);
	myFile.print(sDate);
	myFile.close();
}

void vSDCardSyncDateLoad( ) 
{
	File myFile;
	int StringDec;
	char StringChar;

	SyncDateStr = "0";
	if (SD.exists(SyncPath))	
	{
		myFile = SD.open(SyncPath);
		SyncDateStr = "";
		if (myFile) 
		{
			// read from the file until there's nothing else in it:
			while (myFile.available()) 
			{
       SyncDateStr += (char)(myFile.read());
			}

			myFile.close();
		} 
	}
  
}

String authUid = "0";
void vSDCardUidDateLoad(uint8_t * uid ) 
{
	File myFile;
	char StringChar[20]; 
	sprintf(StringChar,"%02X%02X%02X%02X", uid[0],uid[1],uid[2],uid[3]);

	int StrDec;
	char StrChar;

	String folder01 = String(StringChar).substring(0, 2);
	String folder02 = String(StringChar).substring(2, 5);
	String folderFile = String(StringChar).substring(2, 8);
	//String FileState = String(sa);     

	// vSDCardFolder(folder01 + "/" + folder02);
	// vSDCardFile(folder01 + "/" + folder02 + "/" + folderFile , FileState);  

	String pathStr = folder01 + "/" + folder02 + "/" + folderFile;
    
	//Serial.println((const char*)pathStr.c_str());       

setEvent(&ActiveEvent , RFID_DONE);

	if (SD.exists(pathStr))
	{
		authUid = "";
		myFile = SD.open(pathStr);

		if (myFile) 
		{
		// read from the file until there's nothing else in it:
			while (myFile.available()) 
			{
				authUid += (char)(myFile.read());
			}
			myFile.close();
		}
		Serial.println((const char*)authUid.c_str());     

    //setEvent(&ActiveEvent , RFID_DONE);
	}
	else
	{
     #ifdef DEBUG_SDCARD
		//Serial.println("permission denied...");   
    #endif
    #if 0
		 vSDCardFolder(folder01 + "/" + folder02);
		 vSDCardFile(folder01 + "/" + folder02 + "/" + folderFile , "y");   

		  File myFile;
		   myFile = SD.open(folder01 + "/" + folder02 + "/" + folderFile, FILE_WRITE);
		   myFile.print("y");
		   myFile.close();  
    #endif
	}
}


#endif
