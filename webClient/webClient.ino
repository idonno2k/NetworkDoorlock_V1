// Demo using DHCP and DNS to perform a web client request.
// 2011-06-08 <jc@wippler.nl>
//
// License: GPLv2

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
static uint32_t timer;
Stash stash;

const char website[] PROGMEM = "www.flysys.kr";

// called when the client request is complete
static void my_callback (byte status, word off, word len) {
  Serial.println(">>>");
  Ethernet::buffer[off+300] = 0;
  Serial.print((const char*) Ethernet::buffer + off);
  Serial.println("...");
}

void setup () {
  Serial.begin(57600);
  Serial.println(F("\n[webClient]"));

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
  
  if (millis() > timer) 
  {
    timer = millis() + 5000;
    Serial.println();
    sendToWebServer(0);

  }
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
