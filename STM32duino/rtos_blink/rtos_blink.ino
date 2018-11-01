//#include <wirish/wirish.h>
//#include "libraries/FreeRTOS/MapleFreeRTOS.h"
#include <MapleFreeRTOS900.h>
#include "utility/event_groups.h"

#include <SD.h>

#if 0
#include <Wire.h>
#include <Adafruit_PN532.h>
#define PN532_IRQ   PB1
#define PN532_RESET PB0  // Not connected by default on the NFC Shield

Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

#else

//#include <PN532_I2C.h>
//#include <PN532.h>
// #include <NfcAdapter.h>

//PN532_I2C pn532i2c(Wire);
//PN532 nfc(pn532i2c);

#include <PN532_HSU.h>
#include <PN532.h>

PN532_HSU pn532hsu(Serial3);
PN532 nfc(pn532hsu);

#endif

#include <EtherCard_STM.h>

// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
static byte myip[] = { 10, 0, 1, 95 };
static byte gwip[] = { 10, 0, 1, 1 };
static byte dnsip[] = { 8, 8, 8, 8 };
static byte maskip[] = { 255, 255, 255, 0 };
static byte hisip[] = { 10, 0, 1, 96 };

#define BUFFER_SIZE 700
byte Ethernet::buffer[BUFFER_SIZE];
Stash stash;
const char website[] PROGMEM = "www.flysys.kr";

#define RFID_DONE ( 1 << 0 )
EventGroupHandle_t xEventGroup;
EventBits_t uxBits;

static void vBoardAliveLEDTask(void *pvParameters) 
{
  pinMode(PC13, OUTPUT);
  
  vTaskDelay(3000); 
  
  Serial.println(F("BoardAlive!! -> STM32F103C(72Mhz/20kRAM/64k Flash) "));
  
  for (;;) 
  {
    vTaskDelay(850);      digitalWrite(PC13, LOW);
    vTaskDelay(50);       digitalWrite(PC13, HIGH);
    vTaskDelay(50);       digitalWrite(PC13, LOW);
    vTaskDelay(50);       digitalWrite(PC13, HIGH);
  }
}

static void vPN532Serial3Task(void *pvParameters) 
{
  vTaskDelay(1000);
  Serial.println("PN532_serial3_setup...");
  
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) 
  {
      Serial.print("Didn't find PN53x board");
      while (1)vTaskDelay(5000); // halt
  }
 
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
  nfc.setPassiveActivationRetries(0xFF);
  
  // configure board to read RFID tags
  nfc.SAMConfig();
    
  Serial.println("Waiting for an ISO14443A card");
 
  for (;;) 
  {
    
    boolean success;
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
    uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    
    // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
    // 'uid' will be populated with the UID, and uidLength will indicate
    // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
  
    if (success) 
    {
      Serial.println("Found a card!");
      Serial.print("UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
      Serial.print("UID Value: ");
      for (uint8_t i=0; i < uidLength; i++) 
      {
        Serial.print(" 0x");Serial.print(uid[i], HEX); 
      }
      Serial.println("");

      /* Set bit 0 and bit 4 in xEventGroup. */
      uxBits = xEventGroupSetBits( xEventGroup, RFID_DONE );/* The bits being set. */
      
      // Wait 1 second before continuing
      vTaskDelay(1000); 
    }
    else
    {
      // PN532 probably timed out waiting for a card
      //Serial.println("Timed out waiting for a card");
    }
    
  }
}

// called when the client request is complete
static void my_callback (byte status, word off, word len) {
  Serial.println(">>>");
  Ethernet::buffer[off+300] = 0;
  Serial.print((const char*) Ethernet::buffer + off);
  Serial.println("...");
}

void Enc28j60_sendToWebServer(int source)
{
    if (source == 0)
    {
        Serial.println("Sending POST Source");

        byte sd = stash.create();
        stash.print("uid=");
        stash.println((word) millis() / 123);
        stash.save();

        Stash::prepare(PSTR("POST http://$F/curl/test.php HTTP/1.1" "\r\n"
            "Host: $F" "\r\n"
            "Content-Length: $D" "\r\n"
            "Content-Type: application/x-www-form-urlencoded" "\r\n"
            "\r\n"/*이것때문에 안된거였음 헐~~*/
            "$H"),
            website,website,stash.size(),sd);

        ether.tcpSend();
    }
    else 
    {
        Serial.println("Sending GET Source");
       //ether.browseUrl(PSTR("/curl/test.php"), "?uid=55&var2=10&var3=100", website, my_callback);

    }

}

static void vEnc28j60spi1Task(void *pvParameters) 
{
  vTaskDelay(4000); 
  Serial.println(F("Enc28j60 spi1 Task..."));
    // Change 'SS' to your Slave Select pin, if you arn't using the default pin
  if (ether.begin(sizeof Ethernet::buffer, mymac, PA2) == 0)
    Serial.println(F("Failed to access Ethernet controller"));

  //static setup
  //if (!ether.staticSetup(myip, gwip, dnsip, maskip))
  //    Serial.println("static setup failed");
    
  if (!ether.dhcpSetup())
    Serial.println(F("DHCP failed"));

  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);
  ether.printIp("DNS: ", ether.dnsip);

  // use DNS to resolve the website's IP address
  if (!ether.dnsLookup(website))
    Serial.println("DNS failed");

  ether.printIp("SRV: ", ether.hisip);
  
  for (;;) 
  {
    Enc28j60_sendToWebServer(0);
    vTaskDelay(5000);  
  }
}

Sd2Card card;
byte tmpBuf[512];
static void vSDCardSpi2ReadTask(void *pvParameters) 
{
  vTaskDelay(2000);
  Serial.println(F("SD Read Task..."));
  Serial.print(F("Initializing SD card...\n"));
  
  if (!card.init(SPI_HALF_SPEED, PB12)) 
  {
    Serial.println(F("initialization failed. Things to check:"));
    Serial.println(F("* is a card inserted?"));
    Serial.println(F("* is your wiring correct?"));
    Serial.println(F("* did you change the chipSelect pin to match your shield or module?"));
    while (1)vTaskDelay(5000);;
  } 
  else 
  {
    Serial.println(F("Wiring is correct and a card is present."));
  }

  Serial.println();
  Serial.print("Card type:         ");
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:      Serial.println("SD1");      break;
    case SD_CARD_TYPE_SD2:      Serial.println("SD2");      break;
    case SD_CARD_TYPE_SDHC:     Serial.println("SDHC");     break;
    default:      Serial.println("Unknown");
    }


  for (;;) 
  {
    const TickType_t xTicksToWait = 100 / portTICK_PERIOD_MS;
    
      /* Wait a maximum of 100ms for either bit 0 or bit 4 to be set within
      the event group.  Clear the bits before exiting. */
      uxBits = xEventGroupWaitBits(
                xEventGroup,   /* The event group being tested. */
                RFID_DONE,      /* The bits within the event group to wait for. */
                pdTRUE,        /* RFID_DONE should be cleared before returning. */
                pdFALSE,       /* Don't wait for both bits, either bit will do. */
                xTicksToWait );/* Wait a maximum of 100ms for either bit to be set. */


        if(  uxBits & RFID_DONE == RFID_DONE )
        {
            /* xEventGroupWaitBits() returned because both bits were set. */
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

                vTaskDelay(1);  
              }
            }
        }
        else
        {
          //Serial.println("wait");

        }
  }
}

void setup() 
{
   Serial.begin(115200);
   Serial.println(F("Generic STM32F103C8 with bootloader...\r\n"));
   //delay(1000); 
   
    xEventGroup = xEventGroupCreate();

   xTaskCreate(vBoardAliveLEDTask,"Task1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2,  NULL);
   xTaskCreate(vSDCardSpi2ReadTask,"Task2", configMINIMAL_STACK_SIZE + 100, NULL, tskIDLE_PRIORITY + 4,  NULL);
   xTaskCreate(vPN532Serial3Task,"Task3", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1,  NULL);
   xTaskCreate(vEnc28j60spi1Task,"Task4", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3,  NULL);

    vTaskStartScheduler();
}

void loop() {
    // Insert background code here
}
