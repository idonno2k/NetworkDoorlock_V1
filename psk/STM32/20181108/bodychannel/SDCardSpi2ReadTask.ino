#ifdef SDCARD_ENABLE

#include <SPI.h>
#include <SD.h>

Sd2Card card;
SdVolume volume;
SdFile root;

byte tmpBuf[512];
void vSDCardSpi2ReadTask_setup(void) 
{

#ifdef DEBUG
  Serial.println(F("SD Read Task..."));
  Serial.print(F("Initializing SD card...\n"));
  #endif

  Serial.println("Initializing SD card...");
  if (!SD.begin(PB12)) {
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
  if (!volume.init(card)) {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    while (1);
  }
  
}

void vSDCardFolder(String ns ) 
{
  //Serial.println(ns);    
  if (SD.exists(ns)){
    //Serial.println("folder ok");    
  }else{
    //Serial.println("folder no");    
    SD.mkdir(ns);
  }   
}
void vSDCardFile(String ns , String divi) 
{
  //Serial.println(divi);    
  if(divi == "79"){
    //Serial.println("y");    
    if (SD.exists(ns)){
      //Serial.println("folder ok");    
    }else{
      File myFile;
      myFile = SD.open(ns, FILE_WRITE);
      myFile.println("y");
      myFile.close();
    }      
  }else{
    //Serial.println("n");    
    SD.remove(ns);
  }
 
}

void vSDCardSyncDate( String sDate ) 
{
  File myFile;
  myFile = SD.open("SETTING/SYNCDATE", FILE_WRITE);
  myFile.print(sDate);
  myFile.close();
}

void vSDCardSyncDateLoad( ) 
{
  File myFile;
  int StringDec;
  char StringChar;
  
  myFile = SD.open("SETTING/SYNCDATE");
  SyncDateStr = "";
  if (myFile) {
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      StringDec = (int)(myFile.read());
      StringChar = (char)StringDec;   
      SyncDateStr = SyncDateStr + StringChar;
    }
    
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    SyncDateStr = "0";
  }
  
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
