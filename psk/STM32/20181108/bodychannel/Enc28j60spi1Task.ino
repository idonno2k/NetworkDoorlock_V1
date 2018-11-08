#ifdef ENC28J60_ENABLE

#include <SPI.h>
#include <EtherCard_STM.h>



static uint32_t ethernet_timer;

static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
#define BUFFER_SIZE 2048
byte Ethernet::buffer[BUFFER_SIZE];
Stash stash;

const char website[] PROGMEM = "body.ibuild.kr";

#define sCntMax 30

void vEnc28j60spi1Task_setup(void) 
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

  etherStep = SyncInit;
  ethernet_timer = millis();
}

uint16_t loopCnt;
uint16_t sNo = 0;
uint16_t sCnt = 0;
uint16_t sDateNum=0; 

void vEnc28j60spi1Task(void)
{
  uint16_t RcvLen = ether.packetReceive();
  ether.packetLoop(RcvLen);

/*
  if(RcvLen)
  {
    Serial.print( loopCnt, DEC); Serial.print(" ");Serial.println(RcvLen,DEC);
    loopCnt = 0;
  }
  else
  {
    loopCnt++;
  }
*/    
  
  if (millis() > ethernet_timer) 
  {
    if(etherStep == SyncInit)
    { 
      char paramStr[180]; 
      uint64_t value1 = strtoull(SyncDateStr.c_str(), NULL, 10);
      sprintf(paramStr,"?SyncType=1&SyncDate=%d", value1); 
      Serial.println(value1);
      
      ether.browseUrl(PSTR("/door_control/sync.php"), paramStr , website, SyncInit_callback);
      sNo = 0;
      sCnt = sCntMax;
      etherStep = SyncData;
      ethernet_timer = millis() + 30000;//for timeout 300s
    }
    else if(etherStep == SyncData)
    {
      char paramStr[180]; 
      uint64_t value1 = strtoull(SyncDateStr.c_str(), NULL, 10);
     
      sprintf(paramStr, "?sNo=%d&sCount=%d&SyncDate=%s",sNo,sCnt, SyncDateStr); 
      //Serial.println(paramStr);
      ether.browseUrl(PSTR("/door_control/sync.php"),paramStr, website, SyncData_callback);
      ethernet_timer = millis() + 5000;//for timeout 300s
      
    }
    else if(etherStep == SyncUpdate)
    {
     // ether.browseUrl(PSTR("/door_control/sync.php"),"?SyncType=1&SyncDate=SyncDate", website, my_callback);
      etherStep = SyncIdle;
      ethernet_timer = millis() + 30000;//for timeout 300s
    }
    else //idle
    {

    }
  }
  
} 

// called when the client request is complete

static void SyncInit_callback (byte status, uint16_t off, uint16_t len)
{
  Ethernet::buffer[off+len] = 0;
  
  char *ptr = strstr((const char*) Ethernet::buffer + off, "[[S]]");  
  Serial.print((const char*)ptr);Serial.println("");
  
  char *ptrtok = strtok(ptr+5, "[]");
  sDateNum = (uint16_t)strtoul( ptrtok, NULL, 10);
  Serial.println(sDateNum);
  
  ptrtok = strtok(NULL, "[]");
  SyncDateStr = (const char*)ptrtok;
  Serial.print(SyncDateStr);
  Serial.println("");

  ethernet_timer = millis() + 100;
}

typedef struct _UID {   // 구조체 이름은 _Person
    uint32_t uid;
    uint8_t auth;        // 구조체 멤버 3
} UID;                  // typedef를 사용하여 구조체 별칭을 Person으로 정의
UID uIDArry[sCntMax];

static void SyncData_callback (byte status, uint16_t off, uint16_t len)
{
  Ethernet::buffer[off+len] = 0;

  Serial.print((const char*)Ethernet::buffer + off);
  
  //char tmpStr[50]; 
  //sprintf(tmpStr,"off=%d  len=%d sNo=%d & sCount=%d & sDateNum=%d",off,len ,sNo,sCnt,sDateNum); 
  //Serial.println(tmpStr);

    #if 1
    char *ptr = strstr((const char*)Ethernet::buffer + off, "Content-Length:"); 
  if(ptr == NULL)
  {
      Serial.println("HTTP fail...");
      char paramStr[150]; 
      sprintf(paramStr,"?sNo=%d&sCount=%d&SyncDate=0",sNo,sCnt); 
      //Serial.println(paramStr);
      ether.browseUrl(PSTR("/door_control/sync.php"),paramStr, website, SyncData_callback);
  
  }
  else
  {
    //Serial.print((const char*)Ethernet::buffer + off);Serial.println("");
  
    char *ptr = strstr((const char*) Ethernet::buffer + off, "[[S]]");  
    //Serial.print((const char*)ptr);Serial.println("");
    ptr = ptr +5;
    ptr = strtok(ptr,">");
    if(ptr != NULL)
    {
      for(int index=0 ; index < sCntMax ; index++)
      {
        //Serial.print((const char*)ptr);Serial.print(" ");
        
        uIDArry[index].uid = strtoul( ptr, NULL, 16);
        uIDArry[index].auth = *(ptr+9);
        ptr = strtok(NULL,">");
  
        char s[10];
        char sa[2]="";
        sprintf(s, "%08X " ,uIDArry[index].uid);
        sprintf(sa, "%02X" ,uIDArry[index].auth);
        //Serial.println(sa);

        /*
        String folder01 = "CardInfo/" + String(s).substring(0, 2);
        String folder02 = String(s).substring(2, 4);
        String folder03 = String(s).substring(4, 6);
        //String folderFile = String(s).substring(6, 8);
        String folderFile = String(s).substring(2, 8);
        String FileState = String(sa);

        //vSDCardFolder(folder01);
        //vSDCardFolder(folder01 + "/" + folder02);
        vSDCardFolder(folder01 + "/" + folder02 + "/" + folder03);
        vSDCardFile(folder01 + "/" + folder02 + "/" + folder03 + "/" + folderFile , FileState);
        //vSDCardFile(folder01 + "/" + folder02 + "/" + folderFile , FileState);
        //vSDCardFile(folder01 + "/" + folderFile , FileState);
        */

        String folder01 = String(s).substring(0, 2);
        String folder02 = String(s).substring(2, 5);
        String folderFile = String(s).substring(2, 8);
        String FileState = String(sa);     
        
        vSDCardFolder(folder01 + "/" + folder02);
        vSDCardFile(folder01 + "/" + folder02 + "/" + folderFile , FileState);                   
        
      }
      Serial.println("");
      
      sNo = sNo + sCnt;
      if((sDateNum - sNo) < sCnt)
      sCnt = sDateNum - sNo;
      
      if(sDateNum <=  sNo)
      {
        sNo = 0;
        sCnt = 0;
        sDateNum = 0;
        etherStep = SyncIdle;
        Serial.println("SyncData finished");
        vSDCardSyncDate( SyncDateStr );       
      }
    }
    

  }
 #endif
  ethernet_timer = millis() + 100;
}

#endif
