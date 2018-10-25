// PIN Connections (Using STM32F103):
//
// ENC28J60 -  STM32F103
//   VCC    -    3.3V
//   GND    -    GND
//   SCK    -    Pin PA5
//   SO     -    Pin PA6
//   SI     -    Pin PA7
//   CS     -    Pin PA8



#include <SPI.h>
#include <EtherCard_STM.h>

static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
#define BUFFER_SIZE 700

byte Ethernet::buffer[BUFFER_SIZE];
const char website[] PROGMEM = "www.flysys.kr";

void setup () {
  Serial.begin(115200); 
  Serial.println("ok");
  ethernet_init();
}

void ethernet_init() {
  
  if (ether.begin(sizeof Ethernet::buffer, mymac,PA8) == 0) {
      Serial.println(F("Failed to access Ethernet controller"));
  }
  Serial.println(F("Setting up DHCP"));
  if (!ether.dhcpSetup())
    Serial.println(F("DHCP failed"));
  
  ether.printIp("My IP: ", ether.myip);
  ether.printIp("Netmask: ", ether.netmask);
  ether.printIp("GW IP: ", ether.gwip);
  ether.printIp("DNS IP: ", ether.dnsip);

  // use DNS to resolve the website's IP address
  if (!ether.dnsLookup(website))
    Serial.println("DNS failed");
    
  ether.printIp("SRV: ", ether.hisip); 
  
}

void loop () {}
