#define DEBUG_RTC

#include <RTClock.h>

//RTClock rtclock (RTCSEL_LSE); // initialise
//uint32_t curTimeTag; 
int timezone = 8;      // change to your timezone
time_t curTimeTag, lastTimeTag;
tm_t TimeStamp;
tm_t logTimeStamp;

uint8_t dateread[11];


uint8_t str2month(const char * d)
{
    uint8_t i = 13;
    while ( (--i) && strcmp(months[i], d)!=0 );
    return i;
}
//-----------------------------------------------------------------------------
const char * delim = " :";
char s[128]; // for sprintf
//-----------------------------------------------------------------------------
void ParseBuildTimestamp()
{

    // Timestamp format: "Dec  8 2017, 22:57:54"
    sprintf(s, "Timestamp: %s, %s\n", __DATE__, __TIME__);
    //Serial.print(s);
    char * token = strtok(s, delim); // get first token
    // walk through tokens
    while( token != NULL ) {
        uint8_t m = str2month((const char*)token);
        if ( m>0 ) {
            TimeStamp.month = m;            //Serial.print(" month: "); Serial.println(mt.month);
            
            token = strtok(NULL, delim); // get next token
            TimeStamp.day = atoi(token);       //Serial.print(" day: "); Serial.println(mt.day);
            
            token = strtok(NULL, delim); // get next token
            TimeStamp.year = atoi(token) - 1970;//Serial.print(" year: "); Serial.println(mt.year);
            
            token = strtok(NULL, delim); // get next token
            TimeStamp.hour = atoi(token);       //Serial.print(" hour: "); Serial.println(mt.hour);
            
            token = strtok(NULL, delim); // get next token
            TimeStamp.minute = atoi(token);    //Serial.print(" minute: "); Serial.println(mt.minute);
            
            token = strtok(NULL, delim); // get next token
            TimeStamp.second = atoi(token);     //Serial.print(" second: "); Serial.println(mt.second);
        }
        token = strtok(NULL, delim);
    }
}

void RtcTimeSet(String timeStr)
{
   // curTimeTag = atol((char*)dateread);
 //   rtclock.setTime(rtclock.TimeZone(curTimeTag, timezone)); //adjust to your local date

  String str_year = String(timeStr).substring(0, 4);  TimeStamp.year = atoi(str_year.c_str())-1970; 
  String str_month = String(timeStr).substring(4, 6);  TimeStamp.month = atoi(str_month.c_str()); 
  String str_day = String(timeStr).substring(6, 8);  TimeStamp.day = atoi(str_day.c_str()); 
  String str_hh = String(timeStr).substring(8, 10);  TimeStamp.hour = atoi(str_hh.c_str()); 
  String str_mm = String(timeStr).substring(10, 12);  TimeStamp.minute = atoi(str_mm.c_str()); 
  String str_ss = String(timeStr).substring(12, 14);  TimeStamp.second = atoi(str_ss.c_str()); 

  rtclock.setTime(TimeStamp);

#ifdef DEBUG_RTC
  rtclock.breakTime(rtclock.now(), TimeStamp);
 char s[50];
  sprintf(s, "\n %s %u %u, %s, %02u:%02u:%02u\n", months[TimeStamp.month], TimeStamp.day, TimeStamp.year+1970, weekdays[TimeStamp.weekday], TimeStamp.hour, TimeStamp.minute, TimeStamp.second);
  Serial.println(s);
  #endif
}
//-----------------------------------------------------------------------------
// This function is called in the attachSecondsInterrupt
//-----------------------------------------------------------------------------
void SecondCount ()
{
  curTimeTag++;
}

void rtc_setup() 
{
  ParseBuildTimestamp();  // get the Unix epoch Time counted from 00:00:00 1 Jan 1970
  curTimeTag = rtclock.makeTime(TimeStamp) + 25; // additional seconds to compensate build and upload delay
  rtclock.setTime(curTimeTag);
  lastTimeTag = curTimeTag;
  rtclock.attachSecondsInterrupt(SecondCount);// Call SecondCount
}

//String strLastLogPath="";
