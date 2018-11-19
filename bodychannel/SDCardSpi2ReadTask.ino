#ifdef SDCARD_ENABLE

#include <SPI.h>
#include <SD.h>

Sd2Card card;
SdVolume volume;
SdFile root;

//byte tmpBuf[512];
void vSDCardSpi2ReadTask_setup(void) 
{

	#ifdef DEBUG
	Serial.println(F("SD Read Task..."));
	Serial.print(F("Initializing SD card...\n"));
	#endif

	Serial.println("Initializing SD card...");
	if (!SD.begin(PB12))
	{
		Serial.println("initialization failed!");
		while (1);
	}
	Serial.println("initialization done.");

  
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
		#ifdef DEBUG
		//Serial.println("folder ok"); 
		#endif		
	}
	else
	{
		#ifdef DEBUG
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

String IP_Path = "IP";
String SettingPath = "SET";
/*void vSDCardSetDate( String sDate ) 
{
  if (SD.exists(SettingPath))
    SD.remove(SettingPath);
  
  File myFile;
  myFile = SD.open(SettingPath, FILE_WRITE);
  myFile.print(sDate);
  myFile.close();
}*/

void vSDCardSetDateLoad( ) 
{
	File myFile;
	int StringDec;
	char StringChar;

	SetDateStr = "0"; 
	IPDateStr = "0"; 
	if (SD.exists(IP_Path))  
	{
		myFile = SD.open(IP_Path);
		IPDateStr = "";
		if (myFile) 
		{
			while (myFile.available()) 
			{
				//StringDec = (int)(myFile.read());
				//StringChar = (char)StringDec;   
				//IPDateStr = IPDateStr + StringChar;

       IPDateStr += (char)(myFile.read());
			}
			myFile.close();
		} 
    static_IP = IPDateStr.charAt(0);
    if(static_IP == '1')
    {
      uint8_t offset = 1;
      uint8_t length;
      String ip ;
      length = IPDateStr.indexOf("\n",offset);  ip = IPDateStr.substring(offset,length); ether.parseIp(myip ,(char*)ip.c_str()); offset = offset + length;
      length = IPDateStr.indexOf("\n",offset);  ip = IPDateStr.substring(offset,length); ether.parseIp(maskip ,(char*)ip.c_str()); offset = offset + length;
      length = IPDateStr.indexOf("\n",offset);  ip = IPDateStr.substring(offset,length); ether.parseIp(gwip ,(char*)ip.c_str()); offset = offset + length;
      length = IPDateStr.indexOf("\n",offset);  ip = IPDateStr.substring(offset,length); ether.parseIp(dnsip ,(char*)ip.c_str()); offset = offset + length;
    }
	}

	if (SD.exists(SettingPath))  
	{
		myFile = SD.open(SettingPath);
		SetDateStr = "";
		if (myFile) 
		{
			while (myFile.available()) 
			{
        SetDateStr += (char)(myFile.read());
			}
			myFile.close();

      uint8_t offset = 0;      uint8_t length;      String s ;
      length = IPDateStr.indexOf("\n",offset);  s = IPDateStr.substring(offset,length); offset = offset + length;

      uint8_t idx = 0;      uint8_t len;      String s1;
      len = s.indexOf("/",0);     s1 = s.substring(idx,len);  s1.toCharArray(website,len); idx = idx + len;
      len = s.indexOf("\0",idx);  s1 = s.substring(idx,len);  s1.toCharArray(suburl,len);   
      
      length = IPDateStr.indexOf("\n",offset);  s = IPDateStr.substring(offset,length); s.toCharArray(device_name,length); offset = offset + length;
      length = IPDateStr.indexOf("\n",offset);  s = IPDateStr.substring(offset,length); s.toCharArray(device_serial,length); offset = offset + length;
      length = IPDateStr.indexOf("\n",offset);  s = IPDateStr.substring(offset,length); RelayOpTime = (uint16_t)(strtoul( s.c_str(), NULL, 10)*1000);  offset = offset + length;
      length = IPDateStr.indexOf("\n",offset);  s = IPDateStr.substring(offset,length); FireVoltage = (uint16_t)strtoul( s.c_str(), NULL, 10);
    
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
				//StringDec = (int)(myFile.read());
				//StringChar = (char)StringDec;   
				//SyncDateStr = SyncDateStr + StringChar;

       SyncDateStr += (char)(myFile.read());
			}

			myFile.close();
		} 
	}
  
}

String authUid;
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
    
	Serial.println((const char*)pathStr.c_str());       

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

				//StrDec = (int)(myFile.read());
				//StrChar = (char)StrDec;   
				//authUid = authUid + StrChar;
			}
			myFile.close();
		}
		Serial.println((const char*)authUid.c_str());     

    setEvent(&ActiveEvent , RFID_DONE);
	}
	else
	{
		Serial.println("permission denied...");   
		// vSDCardFolder(folder01 + "/" + folder02);
		// vSDCardFile(folder01 + "/" + folder02 + "/" + folderFile , "y");   

		//  File myFile;
		//   myFile = SD.open(folder01 + "/" + folder02 + "/" + folderFile, FILE_WRITE);
		//   myFile.print("y");
		//   myFile.close();  
	}
}


#endif
