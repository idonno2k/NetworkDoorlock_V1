#define DEBUG
#define ENC28J60_ENABLE
#define PN532_ENABLE
#define SDCARD_ENABLE
String SyncDateStr = "0"; 
String SyncDateStrNew = "0"; 

#include <MapleFreeRTOS900.h>

enum etherState
{
    SyncIdle, SyncInit, SyncData
};
etherState etherStep = SyncIdle;

// the setup function runs once when you press reset or power the board
void setup() 
{
    Serial.begin(115200);
    delay(1000);
    
    Serial.println(F("Generic STM32F103C8 with bootloader...\r\n"));

    vSDCardSpi2ReadTask_setup();    delay(1000);    
    vSDCardSyncDateLoad();          delay(1000);
    
    vPN532Serial3Task_setup();      delay(1000);
    vEnc28j60spi1Task_setup();      delay(1000);

   //xTaskCreate(vPN532Serial3Task,"Task3", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1,  NULL);
   //xTaskCreate(vEnc28j60spi1Task,"Task4", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3,  NULL);
   //vTaskStartScheduler();
    pinMode(PC13, OUTPUT);
}

// the loop function runs over and over again forever
void loop() 
{

	vEnc28j60spi1Task();

	vPN532Serial3Task(); 

}
