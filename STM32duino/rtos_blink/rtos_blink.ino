//#include <wirish/wirish.h>
//#include "libraries/FreeRTOS/MapleFreeRTOS.h"
#include <MapleFreeRTOS900.h>
#include "utility/event_groups.h"

#define DEBUG
#define ENC28J60_ENABLE
#define PN532_ENABLE
#define SDCARD_ENABLE

EventGroupHandle_t xEventGroup;
EventBits_t uxBits;

static void vBoardAliveLEDTask(void *pvParameters) 
{
  vTaskDelay(4000);  
  pinMode(PC13, OUTPUT);

  #ifdef DEBUG
  Serial.println(F("BoardAlive!! -> STM32F103C(72Mhz/20kRAM/64k Flash) "));
  #endif
  for (;;) 
  {
    vTaskDelay(850);      digitalWrite(PC13, LOW);
    vTaskDelay(50);       digitalWrite(PC13, HIGH);
    vTaskDelay(50);       digitalWrite(PC13, LOW);
    vTaskDelay(50);       digitalWrite(PC13, HIGH);
  }
}

void setup() 
{
    Serial.begin(57600);
    Serial.println(F("Generic STM32F103C8 with bootloader...\r\n"));
    delay(1000); 
    
    xEventGroup = xEventGroupCreate();
    
    xTaskCreate(vBoardAliveLEDTask,"Task1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2,  NULL);
    xTaskCreate(vSDCardSpi2ReadTask,"Task2", configMINIMAL_STACK_SIZE + 100, NULL, tskIDLE_PRIORITY + 2,  NULL);
    xTaskCreate(vPN532Serial3Task,"Task3", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2,  NULL);
    xTaskCreate(vEnc28j60spi1Task,"Task4", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2,  NULL);
    
    vTaskStartScheduler();
}

void loop() 
{
    // Insert background code here

}
