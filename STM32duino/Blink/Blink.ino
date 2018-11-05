#include <SPI.h>
#include <EtherCard_STM.h>

static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
#define BUFFER_SIZE 700
byte Ethernet::buffer[BUFFER_SIZE];
Stash stash;
//const char website[] PROGMEM = "www.flysys.kr";
const char website[] PROGMEM = "body.ibuild.kr";

static uint32_t ethernet_timer;
static uint32_t rfid_timer;


// called when the client request is complete
static void my_callback (byte status, uint16_t off, uint16_t len) {
  Serial.println(">>>");
  Ethernet::buffer[off+300] = 0;
  Serial.print((const char*) Ethernet::buffer + off);
  Serial.println("...");
}
void vEnc28j60spi1_setup(void) 
{
  Serial.println(F("Enc28j60 spi1 Task..."));
    // Change 'SS' to your Slave Select pin, if you arn't using the default pin

afio_cfg_debug_ports(AFIO_DEBUG_SW_ONLY); // release PB3 and PB5 
afio_remap(AFIO_REMAP_SPI1); // remap SPI1
  if (ether.begin(sizeof Ethernet::buffer, mymac,PA15) == 0)
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

// the setup function runs once when you press reset or power the board
void setup() 
{
     Serial.begin(115200);
   Serial.println(F("Generic STM32F103C8 with bootloader...\r\n"));

  pinMode(PC13, OUTPUT);

  vEnc28j60spi1_setup(); 
}

// the loop function runs over and over again forever
void loop() 
{
  //===============================================================
  //ethernet loop 5000ms
  ether.packetLoop(ether.packetReceive());
  if (millis() > ethernet_timer) 
  {
    ethernet_timer = millis() + 5000;
    ether.browseUrl(PSTR("/door_control/sync.php"),"", website, my_callback);
  }
  //===============================================================
  //rfid loop 500ms
  if (millis() > rfid_timer) 
  {
    rfid_timer = millis() + 500;

  }                 // wait for a second
}
