
#include <SPI.h>
#include <MFRC522.h>
#include <EtherCard.h>

// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
static byte myip[] = { 10, 0, 1, 95 };
static byte gwip[] = { 10, 0, 1, 1 };
static byte dnsip[] = { 8, 8, 8, 8 };
static byte maskip[] = { 255, 255, 255, 0 };
static byte hisip[] = { 10, 0, 1, 96 };

#define BUFFER_SIZE 700

byte Ethernet::buffer[BUFFER_SIZE];
static uint32_t ethernet_timer;
static uint32_t rfid_timer;

Stash stash;

const char website[] PROGMEM = "www.flysys.kr";

//#define SS_PIN 8

#define SS_PIN 10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key; 
byte nuidPICC[4];


// called when the client request is complete
static void my_callback (byte status, word off, word len) {
  Serial.println(">>>");
  Ethernet::buffer[off+300] = 0;
  Serial.print((const char*) Ethernet::buffer + off);
  Serial.println("...");
}

void setup () 
{
  Serial.begin(57600);

  //Enc28j60_setup () ;

  MFRC522_setup() ;
 
}

void loop () 
{
  uint16_t payloadPos = ether.packetLoop(ether.packetReceive());

  if (payloadPos)
  {
    char* incomingData = (char *) Ethernet::buffer + payloadPos;
    Serial.println(incomingData);
  
   // int sz=BUFFER_SIZE-payloadPos; // max payload size (TCP header is also kept in the buffer)
   // if(sizeof(page)<sz)
   //   sz=sizeof(page);
   // memcpy_P(Ethernet::buffer + payloadPos, page, sz); // Copy data from flash to RAM
   // ether.httpServerReply(sz-1);
  }

  //===============================================================
  //ethernet loop 5000ms
  if (millis() > ethernet_timer) 
  {
    ethernet_timer = millis() + 5000;

   // sendToWebServer(0);

  }
  //===============================================================
  //rfid loop 500ms
  if (millis() > rfid_timer) 
  {
    rfid_timer = millis() + 500;

    MFRC522_ReadNUID_test();

  }

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

void sendToWebServer(int source)
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

void MFRC522_setup() 
{ 
  Serial.println(F("\n[MFRC522_setup]"));
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
}

void MFRC522_ReadNUID_test() 
{
//Serial.println(F("\n[MFRC522_loop]")); 


  // Look for new cards
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  if (rfid.uid.uidByte[0] != nuidPICC[0] || 
    rfid.uid.uidByte[1] != nuidPICC[1] || 
    rfid.uid.uidByte[2] != nuidPICC[2] || 
    rfid.uid.uidByte[3] != nuidPICC[3] ) {
    Serial.println(F("A new card has been detected."));

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }
   
    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
  }
  else Serial.println(F("Card read previously."));

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}


/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}
