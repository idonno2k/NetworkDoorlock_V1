
#include <SPI.h>
#include <EtherCard.h>

#include <Wire.h>
#include <Adafruit_NFCShield_I2C.h>

#include "Sd2Card.h"

#define IRQ   (2)
#define RESET (3)  // Not connected by default on the NFC Shield

Adafruit_NFCShield_I2C nfc(IRQ, RESET);

// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
static byte myip[] = { 10, 0, 1, 95 };
static byte gwip[] = { 10, 0, 1, 1 };
static byte dnsip[] = { 8, 8, 8, 8 };
static byte maskip[] = { 255, 255, 255, 0 };
static byte hisip[] = { 10, 0, 1, 96 };

#define BUFFER_SIZE 100

byte Ethernet::buffer[BUFFER_SIZE];
static uint32_t ethernet_timer;
static uint32_t rfid_timer;

Stash stash;

const char website[] PROGMEM = "www.flysys.kr";

#define SDA_PIN 18
#define SCL_PIN 19
#define RST_PIN 9
#define SS_PIN 10

byte nuidPICC[4];

void setup () 
{
  Serial.begin(57600);

  Enc28j60_setup () ;

  PN532_i2c_setup() ;

  delay(1000);
 
}

void loop () 
{
 // uint16_t payloadPos = ether.packetLoop(ether.packetReceive());

/*  if (payloadPos)
  {
    char* incomingData = (char *) Ethernet::buffer + payloadPos;
    Serial.println(incomingData);
  
   // int sz=BUFFER_SIZE-payloadPos; // max payload size (TCP header is also kept in the buffer)
   // if(sizeof(page)<sz)
   //   sz=sizeof(page);
   // memcpy_P(Ethernet::buffer + payloadPos, page, sz); // Copy data from flash to RAM
   // ether.httpServerReply(sz-1);
  }*/

  //===============================================================
  //ethernet loop 5000ms
  if (millis() > ethernet_timer) 
  {
    ethernet_timer = millis() + 5000;

    Enc28j60_sendToWebServer(0);

  }
  //===============================================================
  //rfid loop 500ms
  if (millis() > rfid_timer) 
  {
    rfid_timer = millis() + 500;

    Serial.println(".");
   PN532_i2c_loop(); 

  }

}




// called when the client request is complete
static void my_callback (byte status, word off, word len) {
  Serial.println(">>>");
  Ethernet::buffer[off+300] = 0;
  Serial.print((const char*) Ethernet::buffer + off);
  Serial.println("...");
}
void Enc28j60_setup () 
{
   Serial.println(F("\n[Enc28j60_setup]"));

  // Change 'SS' to your Slave Select pin, if you arn't using the default pin
  if (ether.begin(sizeof Ethernet::buffer, mymac, SS) == 0)
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
       ether.browseUrl(PSTR("/curl/test.php"), "?uid=55&var2=10&var3=100", website, my_callback);

    }

}


void PN532_i2c_setup(void) 
{

  Serial.println("PN532_i2c_setup...");

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
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
}


void PN532_i2c_loop(void) 
{
  boolean success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
  
  if (success) {
    Serial.println("Found a card!");
    Serial.print("UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("UID Value: ");
    for (uint8_t i=0; i < uidLength; i++) 
    {
      Serial.print(" 0x");Serial.print(uid[i], HEX); 
    }
    Serial.println("");
    // Wait 1 second before continuing
    delay(1000);
  }
  else
  {
    // PN532 probably timed out waiting for a card
    Serial.println("Timed out waiting for a card");
  }
}
