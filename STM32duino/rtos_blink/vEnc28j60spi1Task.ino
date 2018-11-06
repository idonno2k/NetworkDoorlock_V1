#ifdef ENC28J60_ENABLE

#include <EtherCard_STM.h>

// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };

#define BUFFER_SIZE 700
byte Ethernet::buffer[BUFFER_SIZE];
Stash stash;
const char website[] PROGMEM = "body.ibuild.kr";
static uint32_t ethernet_timer;

// called when the client request is complete
static void my_callback (byte status, uint16_t off, uint16_t len) 
{
  char s[7];
  Serial.print(">>>");
  
                sprintf(s, "%02d, %02d",off,len);
                Serial.println(s);
  
  Ethernet::buffer[off+300] = 0;
  Serial.print((const char*) Ethernet::buffer + off);
  Serial.println("...");
}

void Enc28j60_SendingPost(void)
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
    }
    //vTaskDelay(1);
  }
}

#endif
