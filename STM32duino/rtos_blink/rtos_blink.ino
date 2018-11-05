//#include <wirish/wirish.h>
//#include "libraries/FreeRTOS/MapleFreeRTOS.h"
#include <MapleFreeRTOS900.h>
#include "utility/event_groups.h"

#define DEBUG

#include <SPI.h>
#include <SD.h>


#include <PN532_HSU.h>
#include <PN532.h>

PN532_HSU pn532hsu(Serial3);
PN532 nfc(pn532hsu);


#include <EtherCard_STM.h>

// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };

#define BUFFER_SIZE 700
byte Ethernet::buffer[BUFFER_SIZE];
Stash stash;
const char website[] PROGMEM = "body.ibuild.kr";
static uint32_t ethernet_timer;

#define RFID_DONE ( 1 << 0 )
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

static void vPN532Serial3Task(void *pvParameters) 
{
  vTaskDelay(3000);
  #ifdef DEBUG
  Serial.println("PN532_serial3_setup...");
  #endif
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) 
  {
      Serial.print("Didn't find PN53x board");
      while (1)vTaskDelay(5000); // halt
  }
 
  // Got ok data, print it out!
  #ifdef DEBUG
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  #endif
  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
  nfc.setPassiveActivationRetries(0xFF);
  
  // configure board to read RFID tags
  nfc.SAMConfig();
  #ifdef DEBUG
  Serial.println("Waiting for an ISO14443A card");
  #endif
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
      #ifdef DEBUG
      Serial.println("Found a card!");
      Serial.print("UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
      Serial.print("UID Value: ");
      for (uint8_t i=0; i < uidLength; i++) 
      {
        Serial.print(" 0x");Serial.print(uid[i], HEX); 
      }
      Serial.println("");
      #endif
      /* Set bit 0 and bit 4 in xEventGroup. */
      uxBits = xEventGroupSetBits( xEventGroup, RFID_DONE );/* The bits being set. */
      
      // Wait 1 second before continuing
      vTaskDelay(1000); 
    }
    else
    {
      #ifdef DEBUG
      // PN532 probably timed out waiting for a card
      //Serial.println("Timed out waiting for a card");
      #endif
    }
    
  }
}


// called when the client request is complete
static void my_callback (byte status, uint16_t off, uint16_t len) 
{
  Serial.println(">>>");
  Ethernet::buffer[off+300] = 0;
  Serial.print((const char*) Ethernet::buffer + off);
  Serial.println("...");
}

void Enc28j60_sendToWebServer(int source)
{
    if (source == 0)
    {
      #ifdef DEBUG
        Serial.println("Sending POST Source");
      #endif
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
      #ifdef DEBUG
        Serial.println("Sending GET Source");
      #endif
       // ether.browseUrl(PSTR("/curl/test.php"), "?uid=55&var2=10&var3=100", website, my_callback);

    }

}

static void vEnc28j60spi1Task(void *pvParameters) 
{
vTaskDelay(2000);  
  
  #ifdef DEBUG
  Serial.println(F("Enc28j60 spi1 Task..."));
  #endif

  afio_cfg_debug_ports(AFIO_DEBUG_SW_ONLY); // release PB3 and PB5 
  afio_remap(AFIO_REMAP_SPI1); // remap SPI1
  if (ether.begin(sizeof Ethernet::buffer, mymac,PA15) == 0)
  {
    #ifdef DEBUG
    Serial.println(F("Failed to access Ethernet controller"));
    #endif
  }

  //static setup
  //if (!ether.staticSetup(myip, gwip, dnsip, maskip))
  //    Serial.println("static setup failed");
    
  if (!ether.dhcpSetup())
  {
    #ifdef DEBUG
    Serial.println(F("DHCP failed"));
    #endif
  }
  #ifdef DEBUG
  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);
  ether.printIp("DNS: ", ether.dnsip);
  #endif

  // use DNS to resolve the website's IP address
  if (!ether.dnsLookup(website))
    Serial.println("DNS failed");

  ether.printIp("SRV: ", ether.hisip);
  
  for (;;) 
  {
    ether.packetLoop(ether.packetReceive());
    if (millis() > ethernet_timer) 
    {
      ethernet_timer = millis() + 300000;
      ether.browseUrl(PSTR("/door_control/sync.php"),"", website, my_callback);
      vTaskDelay(1);
    }
  }
}

Sd2Card card;
byte tmpBuf[512];
static void vSDCardSpi2ReadTask(void *pvParameters) 
{
vTaskDelay(1000);  
  
#ifdef DEBUG
  Serial.println(F("SD Read Task..."));
  Serial.print(F("Initializing SD card...\n"));
  #endif
  
  if (!card.init(SPI_HALF_SPEED, PB12)) 
  {
    #ifdef DEBUG
    Serial.println(F("initialization failed. Things to check:"));
    Serial.println(F("* is a card inserted?"));
    Serial.println(F("* is your wiring correct?"));
    Serial.println(F("* did you change the chipSelect pin to match your shield or module?"));
    #endif
    while (1)vTaskDelay(5000);;
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
   delay(1000); 
   
    xEventGroup = xEventGroupCreate();

   xTaskCreate(vBoardAliveLEDTask,"Task1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1,  NULL);
   xTaskCreate(vSDCardSpi2ReadTask,"Task2", configMINIMAL_STACK_SIZE + 100, NULL, tskIDLE_PRIORITY + 2,  NULL);
   xTaskCreate(vPN532Serial3Task,"Task3", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2,  NULL);
   xTaskCreate(vEnc28j60spi1Task,"Task4", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2,  NULL);

    vTaskStartScheduler();
}

void loop() {
    // Insert background code here

}
