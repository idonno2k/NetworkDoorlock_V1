#define DEBUG
#define ENC28J60_ENABLE
#define PN532_ENABLE
#define SDCARD_ENABLE

#include <SPI.h>
#include <SD.h>
#include <RTClock.h>

RTClock rtclock (RTCSEL_LSE); // initialise
//-----------------------------------------------------------------------------
const char * weekdays[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
const char * months[] = {"Dummy", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
//-----------------------------------------------------------------------------


//============================================
//extern
extern String strLogDate; 
extern String strLogUID;  
extern String strSyncDate; 
extern String strSyncDateNew;

extern tm_t logTimeStamp;

//============================================
//global

uint16_t RelayONTime = 3000;
uint8_t FireVoltage = 24;

#define RELAY_PIN PB9
#define BUZZ_PIN PB7
#define LED2_PIN PC13
#define LED1_PIN PB6
#define FIRE_PIN PB8

#define RFID_DONE ( 1 << 0 )
#define FIRE_ON ( 1 << 1 )
#define REMOTE_ON ( 1 << 2 )
static uint8_t ActiveEvent = 0;
static uint32_t RelayTimer = 0;
static uint32_t RelayOntime = 0;

#include <MapleFreeRTOS900.h>

xSemaphoreHandle xBinarySemaphore = NULL; 

void setup() 
{
    Serial.begin(115200);
    delay(1000);
   
    Serial.println(F("Generic STM32F103C8 with bootloader...\r\n"));

   vSemaphoreCreateBinary( xBinarySemaphore );   delay(1000);

    vSDCardSpi2ReadTask_setup();    delay(100);    
    vSDCardSyncDateLoad();          delay(100);
    vSDCardSetParmLoad() ;          delay(100);

    rtc_setup(); delay(100);

    pinMode(LED2_PIN, OUTPUT);    digitalWrite(LED2_PIN, LOW);delay(100);//led
    pinMode(RELAY_PIN, OUTPUT);    digitalWrite(RELAY_PIN, LOW);delay(100);//relay
    pinMode(BUZZ_PIN, OUTPUT);    delay(100);//buzzer
    pinMode(FIRE_PIN, INPUT);    delay(100);//fire alert

    xTaskCreate(vLEDFlashTask, "Task1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2,  NULL);
    xTaskCreate(vEn28j60TaskLoop, "Task2", configMINIMAL_STACK_SIZE+512, NULL, tskIDLE_PRIORITY + 2,  NULL);
    xTaskCreate(vPN532TaskLoop, "Task3", configMINIMAL_STACK_SIZE+512, NULL, tskIDLE_PRIORITY + 2,  NULL);
      
    vTaskStartScheduler();
}

void loop() 
{
    // Insert background code here
}

static void vLEDFlashTask(void *pvParameters) 
{
    for (;;) 
    {
        vTaskDelay(50);      digitalWrite(LED1_PIN, HIGH);
        vTaskDelay(50);      digitalWrite(LED1_PIN, LOW);
        vTaskDelay(50);      digitalWrite(LED1_PIN, HIGH);
        vTaskDelay(50);      digitalWrite(LED1_PIN, LOW);
        vTaskDelay(300); 
        vEventTask();
    }
}

void vEventTask(void) 
{
  if (digitalRead(FIRE_PIN) == HIGH)setEvent(&ActiveEvent ,FIRE_ON);
  else  clearEvent(&ActiveEvent ,FIRE_ON);

      if((ActiveEvent & RFID_DONE) == RFID_DONE)
      {
        clearEvent(&ActiveEvent ,RFID_DONE);
        digitalWrite(RELAY_PIN, HIGH);  digitalWrite(LED2_PIN, HIGH);
        vTaskDelay(3000);
        digitalWrite(RELAY_PIN, LOW);   digitalWrite(LED2_PIN, LOW);

        #ifdef DEBUG
        Serial.print("-");   
        #endif
      }
      else if((ActiveEvent & FIRE_ON) == FIRE_ON)
      {
        digitalWrite(RELAY_PIN, HIGH);
      }
      else if((ActiveEvent & REMOTE_ON) == REMOTE_ON)
      {
        digitalWrite(RELAY_PIN, HIGH);
      }
      else
      {
          digitalWrite(RELAY_PIN, LOW);
          #ifdef DEBUG
          Serial.print("_");   
          #endif
      }

}

void setEvent(uint8_t* event ,uint8_t event_code)
{
  *event = *event | event_code;
}
void clearEvent(uint8_t* event ,uint8_t event_code)
{
  *event = *event  & ~event_code;
 }
