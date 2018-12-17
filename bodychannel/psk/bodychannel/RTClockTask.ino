#include <RTClock.h>

RTClock rtclock (RTCSEL_LSE); // initialise
//uint32_t tt; 
int timezone = 8;      // change to your timezone
time_t tt, tt1;
tm_t mtt;
uint8_t dateread[11];

String LogDateStr = ""; 
//-----------------------------------------------------------------------------
const char * weekdays[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
const char * months[] = {"Dummy", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
//-----------------------------------------------------------------------------
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
            mtt.month = m;            //Serial.print(" month: "); Serial.println(mt.month);
            
            token = strtok(NULL, delim); // get next token
            mtt.day = atoi(token);       //Serial.print(" day: "); Serial.println(mt.day);
            
            token = strtok(NULL, delim); // get next token
            mtt.year = atoi(token) - 1970;//Serial.print(" year: "); Serial.println(mt.year);
            
            token = strtok(NULL, delim); // get next token
            mtt.hour = atoi(token);       //Serial.print(" hour: "); Serial.println(mt.hour);
            
            token = strtok(NULL, delim); // get next token
            mtt.minute = atoi(token);    //Serial.print(" minute: "); Serial.println(mt.minute);
            
            token = strtok(NULL, delim); // get next token
            mtt.second = atoi(token);     //Serial.print(" second: "); Serial.println(mt.second);
        }
        token = strtok(NULL, delim);
    }
}
//-----------------------------------------------------------------------------
// This function is called in the attachSecondsInterrupt
//-----------------------------------------------------------------------------
void SecondCount ()
{
  tt++;
}
// This function is called in the attachSecondsInterrpt
void blink () 
{
 digitalWrite(PC13,!digitalRead(PC13));
}

void rtc_setup() 
{
  ParseBuildTimestamp();  // get the Unix epoch Time counted from 00:00:00 1 Jan 1970
  tt = rtclock.makeTime(mtt) + 25; // additional seconds to compensate build and upload delay
  rtclock.setTime(tt);
  tt1 = tt;
  rtclock.attachSecondsInterrupt(SecondCount);// Call SecondCount
}

String lastLogPath="";
void rtc_loop() 
{
 /* 
  if ( Serial.available()>10 ) {
    for (uint8_t i = 0; i<11; i++) {
      dateread[i] = Serial.read();
    }
    Serial.flush();
    tt = atol((char*)dateread);
    rtclock.setTime(rtclock.TimeZone(tt, timezone)); //adjust to your local date
  }
*/

  char currLogPath[128]; 
  if (tt >= tt1)//log
  {
     tt1 = tt + 60;
    // get and print actual RTC timestamp
    rtclock.breakTime(rtclock.now(), mtt);
    //sprintf(s, "\nlog - %s %u %u, %s, %02u:%02u:%02u\n", months[mtt.month], mtt.day, mtt.year+1970, weekdays[mtt.weekday], mtt.hour, mtt.minute, mtt.second);
    sprintf(currLogPath, "log_%u%u%u",mtt.year+1970, mtt.month, mtt.day, mtt.hour);
    
    Serial.print(currLogPath);

    if(mtt.minute == 0)
    {
       vSDCardLogRead(lastLogPath )  ;

        byte sd = stash.create();
        stash.println(LogDateStr.c_str() );
        stash.save();

        Stash::prepare(PSTR("POST http://$F/$F" "\r\n"
            "Host: $F" "\r\n"
            "Content-Length: $D" "\r\n"
            "Content-Type: application/x-www-form-urlencoded" "\r\n"
            "\r\n"/*이것때문에 안된거였음 헐~~*/
            "$H"),
            website,suburl,stash.size(),sd);

        ether.tcpSend();
     
     }
     lastLogPath = "";
     lastLogPath = currLogPath;
  }
}
