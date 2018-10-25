//#include <wirish/wirish.h>
//#include "libraries/FreeRTOS/MapleFreeRTOS.h"
#include <MapleFreeRTOS900.h>

static void vLEDFlashTask(void *pvParameters) {
    for (;;) {
      vTaskDelay(850);      digitalWrite(PC13, LOW);
      vTaskDelay(50);        digitalWrite(PC13, HIGH);
      vTaskDelay(50);        digitalWrite(PC13, LOW);
      vTaskDelay(50);        digitalWrite(PC13, HIGH);
    }
}

void setup() {
    // initialize the digital pin as an output:
    pinMode(PC13, OUTPUT);
    xTaskCreate(vLEDFlashTask,"Task1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2,  NULL);

    
    vTaskStartScheduler();
}

void loop() {
    // Insert background code here
}
