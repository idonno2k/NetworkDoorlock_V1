#define DEBUG
#define ENC28J60_ENABLE
#define PN532_ENABLE
#define SDCARD_ENABLE
String SyncDateStr = "0"; 

enum etherState
{
    SyncIdle, SyncInit, SyncData, SyncUpdate
};
etherState etherStep = SyncIdle;

// the setup function runs once when you press reset or power the board
void setup() 
{
    Serial.begin(115200);
    //delay(1000);
    
    Serial.println(F("Generic STM32F103C8 with bootloader...\r\n"));
    
    pinMode(PC13, OUTPUT);

    vSDCardSpi2ReadTask_setup();    delay(1000);    
    vPN532Serial3Task_setup();      delay(1000);
    vSDCardSyncDateLoad();          delay(1000);
    vEnc28j60spi1Task_setup();      delay(1000);

}

// the loop function runs over and over again forever
void loop() 
{
    vSDCardSyncDateLoad();
    //Serial.println(SyncDateStr);
    char paramStr[180]; 
    uint64_t value1 = strtoull(SyncDateStr.c_str(), NULL, 10);
    sprintf(paramStr,"?SyncType=1&SyncDate=%u", value1); 
    Serial.println(value1);
    Serial.println(paramStr);
    delay(1000);

    
    
  //vEnc28j60spi1Task();
  if(etherStep == SyncIdle)
  {
    vPN532Serial3Task(); 
  }
  //vSDCardSpi2ReadTask();


}


void setEvent(uint8_t* event ,uint8_t event_code)
{
  *event = *event | event_code;
}
void clearEvent(uint8_t* event ,uint8_t event_code)
{
  *event = *event  & ~event_code;

}
