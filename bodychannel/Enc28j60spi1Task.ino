#ifdef ENC28J60_ENABLE
#define DEBUG_ENC28J60

#include <SPI.h>
#include <EtherCard_STM.h>


enum etherState
{
    SyncIdle, SyncInit, SyncData
};
etherState EtherStep = SyncIdle;

uint8_t LogAckFlag = false;

static uint32_t EthernetTimer;
uint32_t LogTimeOut;


static uint8_t static_IP = 0; 
static uint8_t mymac[6] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
static uint8_t myip[4];   ///< IP address
static uint8_t maskip[4]; ///< Netmask
static uint8_t gwip[4];   ///< Gateway
static uint8_t dnsip[4]; ///< DHCP server IP address

static String strWebSite = "";
static String strSubLogUrl = "";
static String strSubSyncUrl = "";
static String strDeviceName  = "67CU65SU7LGE64SQIOuNsOuqqA==";
static String strDeviceSerial = "2000";

#define BUFFER_SIZE 2048
byte Ethernet::buffer[BUFFER_SIZE];
Stash stash;

#define sCntMax 30

void vEnc28j60spi1Task_setup(void) 
{
	#ifdef DEBUG_ENC28J60
	Serial.println(F("Enc28j60 spi1 Task..."));
	#endif
    
	afio_cfg_debug_ports(AFIO_DEBUG_SW_ONLY); // release PB3 and PB5 
	afio_remap(AFIO_REMAP_SPI1); // remap SPI1
	if (ether.begin(sizeof Ethernet::buffer, mymac,PA15) == 0)
	{
		Serial.println(F("Failed to access Ethernet controller"));
  	}

	if( static_IP == 1) 
	{
		Serial.println("STATIC_IP");
		if (!ether.staticSetup(myip, gwip, dnsip, maskip))
	    	Serial.println("static setup failed");
	}
	else
	{
   		Serial.println("DHCP:");
		if (!ether.dhcpSetup())
			Serial.println(F("DHCP failed"));
	}
	ether.printIp("IP:  ", ether.myip);
	ether.printIp("GW:  ", ether.gwip);
	ether.printIp("DNS: ", ether.dnsip);

	// use DNS to resolve the website's IP address
	if (!ether.dnsLookup(strWebSite.c_str()))
		Serial.println("DNS failed");

	ether.printIp("SRV: ", ether.hisip);

	EtherStep = SyncInit;
	EthernetTimer = millis();
}

uint16_t loopCnt;
uint16_t sNo = 0;
uint16_t sCnt = 0;
uint16_t sDateNum=0; 
char paramStr[150]; 
void vEnc28j60spi1Task(void) 
{
	ether.packetLoop(ether.packetReceive());

	if (millis() > EthernetTimer) 
	{
		if(EtherStep == SyncInit)
		{ 
			vSDCardSyncDateLoad( ) ;
			const char *cstr = strSyncDate.c_str();
			sprintf(paramStr,"?SyncType=1&SyncDate=%s", cstr);
			
       		#ifdef DEBUG_ENC28J60
			//Serial.println((const char*)paramStr); 
      		Serial.print(strWebSite);Serial.print(strSubSyncUrl);Serial.println(paramStr);
      		#endif
      		ether.browseUrl((const char*)strSubSyncUrl.c_str(), (const char*)paramStr , (const char*)strWebSite.c_str(), SyncInit_callback);
      		//ether.browseUrl(PSTR("/door_control/sync.php"), (const char*)paramStr , strWebSite.c_str(), SyncInit_callback);
      
			sNo = 0;
			sCnt = sCntMax;
			EtherStep = SyncData;
			EthernetTimer = millis() + 30000;
		}
		else if(EtherStep == SyncData)
		{
			const char *cstr = strSyncDate.c_str();   
			sprintf(paramStr, "?sNo=%d&sCount=%d&SyncDate=%s",sNo,sCnt, cstr); 
      		#ifdef DEBUG_ENC28J60
			//Serial.println((const char*)paramStr);  
      		Serial.print(strWebSite);Serial.print(strSubSyncUrl);Serial.println(paramStr);
      		#endif
      		ether.browseUrl((const char*)strSubSyncUrl.c_str(),(const char*)paramStr, (const char*)strWebSite.c_str(), SyncData_callback);
 			EthernetTimer = millis() + 30000; 
		}
		else //idle
		{
			EtherStep = SyncInit;
			EthernetTimer = millis() + 300000;

		}

	}
  
} 


static void SyncInit_callback (byte status, uint16_t off, uint16_t len)
{
	Ethernet::buffer[off+len] = 0;

	//Serial.print("SyncInit_callback");
	
	char *ptr = strstr((const char*) Ethernet::buffer + off, "[[S]]");  
#ifdef DEBUG_ENC28J60
	Serial.print((const char*)Ethernet::buffer + off);Serial.println("");
#endif
	char *ptrtok = strtok(ptr+5, "[]");
	sDateNum = (uint16_t)strtoul( ptrtok, NULL, 10);
#ifdef DEBUG_ENC28J60
	Serial.print("sDateNum : ");
	Serial.println(sDateNum);
#endif

	ptrtok = strtok(NULL, "[]");
	strSyncDateNew = (const char*)ptrtok;
#ifdef DEBUG_ENC28J60
	Serial.print("strSyncDate : ");
	Serial.print(strSyncDateNew);
	Serial.println("");
#endif

  RtcTimeSet(strSyncDateNew);  
  if(sDateNum == 0)
    EtherStep = SyncIdle;
  
	EthernetTimer = millis() + 100;
}

typedef struct _UID {   // 구조체 이름은 _Person
    uint32_t uid;
    uint8_t auth;        // 구조체 멤버 3
} UID;                  // typedef를 사용하여 구조체 별칭을 Person으로 정의
UID uIDArry[sCntMax];

static void SyncData_callback (byte status, uint16_t off, uint16_t len)
{
	Ethernet::buffer[off+len] = 0;
#ifdef DEBUG_ENC28J60
	//Serial.print("SyncData_callback");
	//Serial.print((const char*)Ethernet::buffer + off);
	//char tmpStr[50]; 
	//sprintf(tmpStr,"off=%d  len=%d sNo=%d & sCount=%d & sDateNum=%d",off,len ,sNo,sCnt,sDateNum); 
	//Serial.println(tmpStr);
#endif

#if 0 //Content-Length filter
	char *ptr = strstr((const char*)Ethernet::buffer + off, "Content-Length:"); 
	if(ptr == NULL)
	{
		Serial.println("HTTP fail...");
		char paramStr[150]; 
		sprintf(paramStr,"?sNo=%d&sCount=%d&SyncDate=0",sNo,sCnt); 
		//Serial.println(paramStr);
		//ether.browseUrl(PSTR("/door_control/sync.php"),(const char*)paramStr, strWebSite, SyncData_callback);
		ether.browseUrl((const char*)strSubSyncUrl.c_str(), (const char*)paramStr , (const char*)strWebSite.c_str(), SyncInit_callback);
	}
	else
#endif
	{
	   // Serial.print((const char*)Ethernet::buffer + off);Serial.println("");
		  
		char *ptr = strstr((const char*) Ethernet::buffer + off, "[[S]]");  
    #ifdef DEBUG_ENC28J60
		Serial.print((const char*)ptr);Serial.println("");
    #endif
		ptr = ptr +5;
		ptr = strtok(ptr,">");
		#if 1
		if(ptr != NULL)
		{
			
			for(int index=0 ; index < sCntMax ; index++)
			{
        #ifdef DEBUG_ENC28J60
				//Serial.print((const char*)ptr);Serial.print(" ");
        #endif
				uIDArry[index].uid = strtoul( ptr, NULL, 16);
				uIDArry[index].auth = *(ptr+9);
				ptr = strtok(NULL,">");

				char s[10];
				char sa[2]="";
				sprintf(s, "%08X " ,uIDArry[index].uid);
				sprintf(sa, "%02X" ,uIDArry[index].auth);
				//Serial.println(sa);

				String folder01 = String(s).substring(0, 2);
				String folder02 = String(s).substring(2, 5);
				String folderFile = String(s).substring(2, 8);
				String FileState = String(sa);     

				vSDCardFolder(folder01 + "/" + folder02);
				vSDCardFile(folder01 + "/" + folder02 + "/" + folderFile , FileState);                   
        
       			vPN532Serial3Task();
        
			}
      #ifdef DEBUG_ENC28J60
			Serial.println("");
	  #endif
		  
			sNo = sNo + sCnt;
			if((sDateNum - sNo) < sCnt)
				sCnt = sDateNum - sNo;
		  
			if(sDateNum <=  sNo)
			{
				sNo = 0;
				sCnt = 0;
				sDateNum = 0;
				EtherStep = SyncIdle;
				Serial.println("SyncData finished");
				vSDCardSyncDate( strSyncDateNew );       
			}

		}
		#endif
	}
	
	EthernetTimer = millis() + 100;

}


//String strLogDate; 
String strLogUID; 
void etherLogData(uint8_t * uid ) 
{
	char arr_logdata[128]; 
	char arr_loguid[20]; 
	char arr_logfilename[128]; 

	rtclock.breakTime(rtclock.now(), logTimeStamp);

	//sprintf(arr_logdata, "%s %u %u, %s, %02u:%02u:%02u : ", months[logTimeStamp.month], logTimeStamp.day, logTimeStamp.year+1970, weekdays[logTimeStamp.weekday], logTimeStamp.hour, logTimeStamp.minute, logTimeStamp.second);
	sprintf(arr_logdata, "%u%u%u%02u%02u%02u",  logTimeStamp.year+1970, logTimeStamp.month, logTimeStamp.day,logTimeStamp.hour, logTimeStamp.minute, logTimeStamp.second);
	sprintf(arr_loguid, "%02X%02X%02X%02X", uid[0],uid[1],uid[2],uid[3]);

	strLogDate = arr_logdata;
	strLogUID = arr_loguid;

	String str_logData = strLogDate + "-" + strLogUID;

	//const char *cstr = str_logData.c_str();
	//sprintf(paramStr,"?%s", cstr);
	ether.browseUrl((const char*)strSubLogUrl.c_str(), (const char*)str_logData.c_str(), (const char*)strWebSite.c_str(), log_callback);

	LogAckFlag = true;

	LogTimeOut = millis() + 500;

}


static void log_callback (byte status, uint16_t off, uint16_t len)
{
	Ethernet::buffer[off+len] = 0;

	//Serial.print("log_callback");

	char *ptr = strstr((const char*) Ethernet::buffer + off, "[[S]]");  
#ifdef DEBUG_ENC28J60
	Serial.print((const char*)ptr);Serial.println("");
#endif
	char *ptrtok = strtok(ptr+5, "[]");
	String str_log = (const char*)ptrtok;
#ifdef DEBUG_ENC28J60
	Serial.print("log ack : ");
	Serial.print(str_log);
	Serial.println("");
#endif

	LogAckFlag = false;
}
#endif  
