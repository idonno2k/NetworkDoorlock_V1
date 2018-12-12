#define DEBUG
#define ENC28J60_ENABLE
#define PN532_ENABLE
#define SDCARD_ENABLE


String SyncDateStr = "0"; 
String SyncDateStrNew = "0"; 

//#include <MapleFreeRTOS900.h>

static char device_name[]  = "67CU65SU7LGE64SQIOuNsOuqqA==";
static char device_serial[] = "2000";
uint16_t RelayONTime = 3000;
uint8_t FireVoltage = 24;

enum etherState
{
    SyncIdle, SyncInit, SyncData
};
etherState etherStep = SyncIdle;

#define RFID_DONE ( 1 << 0 )
#define FIRE_ON ( 1 << 1 )
#define REMOTE_ON ( 1 << 2 )
static uint8_t ActiveEvent = 0;
static uint32_t relay_timer = 0;
static uint32_t relay_ontime = 0;

void vEventTask(void) 
{
    if (millis() > relay_timer) 
    {
      if((ActiveEvent & RFID_DONE) == RFID_DONE)
      {
        relay_ontime = millis() + RelayONTime;
        clearEvent(&ActiveEvent ,RFID_DONE);
        digitalWrite(PC13, LOW);
        #ifdef DEBUG
        //Serial.print("-");   
        #endif
      }
      else if((ActiveEvent & FIRE_ON) == FIRE_ON)
      {
        digitalWrite(PC13, LOW);
      }
      else if((ActiveEvent & REMOTE_ON) == REMOTE_ON)
      {
        digitalWrite(PC13, LOW);
      }
      else
      {
        if (millis() > relay_ontime) 
        {
          digitalWrite(PC13, HIGH);
          #ifdef DEBUG
           //Serial.print("_");   
           #endif
        }
      }
      relay_timer = relay_timer + 100;
    }
}


// the setup function runs once when you press reset or power the board
void setup() 
{
    Serial.begin(115200);
    delay(1000);
    
    Serial.println(F("Generic STM32F103C8 with bootloader...\r\n"));

    vSDCardSpi2ReadTask_setup();    delay(100);    
    vSDCardSyncDateLoad();          delay(100);
    vSDCardSetDateLoad() ;          delay(100);
    
    vPN532Serial3Task_setup();      delay(100);
    vEnc28j60spi1Task_setup();      delay(100);

   //xTaskCreate(vPN532Serial3Task,"Task3", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1,  NULL);
   //xTaskCreate(vEnc28j60spi1Task,"Task4", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3,  NULL);
   //vTaskStartScheduler();
    pinMode(PC13, OUTPUT);
    digitalWrite(PC13, LOW);delay(100);

    relay_timer = millis();
}

// the loop function runs over and over again forever
void loop() 
{
	vEnc28j60spi1Task();

	vPN532Serial3Task(); 

  vEventTask(); 
}



void setEvent(uint8_t* event ,uint8_t event_code)
{
  *event = *event | event_code;
}
void clearEvent(uint8_t* event ,uint8_t event_code)
{
  *event = *event  & ~event_code;
 }
