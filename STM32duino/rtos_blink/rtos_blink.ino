#include <SD.h>

#define SDCARD_SS_PIN PB12
#define SDCARD_MOSI_PIN PB15
#define SDCARD_MISO_PIN PB14
#define SDCARD_SCK_PIN PB13

//#include <wirish/wirish.h>
//#include "libraries/FreeRTOS/MapleFreeRTOS.h"
#include <MapleFreeRTOS900.h>


static void vLEDFlashTask(void *pvParameters) 
{
  vTaskDelay(1000);
   Serial.println("LEDFlash Task...\n");
    pinMode(PC13, OUTPUT);
    for (;;)
    {
      vTaskDelay(850);      digitalWrite(PC13, LOW);
      vTaskDelay(50);        digitalWrite(PC13, HIGH);
      vTaskDelay(50);        digitalWrite(PC13, LOW);
      vTaskDelay(50);        digitalWrite(PC13, HIGH);
    }
}

Sd2Card card;
static void vSDCardReadTask(void *pvParameters) 
{
  vTaskDelay(1000);
  Serial.println("SD Read Task...\n");
  Serial.print("\nInitializing SD card...\n");
  
  if (!card.init(SPI_HALF_SPEED, PB12)) 
  {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    while (1);
  } 
  else 
  {
    Serial.println("Wiring is correct and a card is present.");
  }

  Serial.println();
  Serial.print("Card type:         ");
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:      Serial.println("SD1");      break;
    case SD_CARD_TYPE_SD2:      Serial.println("SD2");      break;
    case SD_CARD_TYPE_SDHC:     Serial.println("SDHC");     break;
    default:      Serial.println("Unknown");}

  for (;;)
  {
  
  }
}

void setup() 
{
   Serial.begin(115200);
   Serial.println("Generic STM32F103C8 with bootloader...\r\n");
   //delay(1000); 
   
   xTaskCreate(vLEDFlashTask,"Task1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2,  NULL);
   xTaskCreate(vSDCardReadTask,"Task2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2,  NULL);

    vTaskStartScheduler();
}

void loop() {
    // Insert background code here
}
