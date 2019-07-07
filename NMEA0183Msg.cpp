/*
NMEA0183Msg.cpp

Copyright (c) 2015-2019 Timo Lappalainen, Kave Oy, www.kave.fi

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include <math.h>
#ifndef ARDUINO
#include <cstdio>
#endif
#include "NMEA0183Msg.h"

#ifndef SECS_PER_DAY
#define SECS_PER_DAY 86400UL
#endif

// Teensy 3.2, 3.5, 3.6
// Arduino Mega
// Linux
#if !( defined(__MK20DX256__)||defined(__ATMEGA32U4__) || defined(__MK64FX512__) || defined (__MK66FX1M0__) \
       || defined(__SAM3X8E__) \
       || defined(__linux__)||defined(__linux)||defined(linux) \
     )
#define NO_PRINTF_DOUBLE_SUPPORT
#endif

#ifndef ARDUINO
extern "C" {
// Application execution delay. Must be implemented by application.
extern void delay(uint32_t ms);

// Current uptime in milliseconds. Must be implemented by application.
extern uint32_t millis();
}
#endif

const char *tNMEA0183Msg::EmptyField="";
const char *tNMEA0183Msg::DefDoubleFormat="%.1f";

//*****************************************************************************
tNMEA0183Msg::tNMEA0183Msg() {
  Clear();
}

//*****************************************************************************
bool tNMEA0183Msg::SetMessage(const char *buf) {
  unsigned char csMsg;
  int i=0;
  uint8_t iData=0;
  bool result=false;

  Clear();
  _MessageTime=millis();

  if ( buf[i]!='$' &&  buf[i]!='!' ) return result; // Invalid message
  Prefix=buf[i];
  i++; // Pass start prefix

//  Serial.println(buf);
  // Set sender
  for (; iData<2 && buf[i]!=0; i++, iData++) {
    CheckSum^=buf[i];
    Data[iData]=buf[i];
  }

  if (buf[i]==0) { Clear(); return result; } // Invalid message

  Data[iData]=0; iData++; // null termination for sender
  // Set message code. Read until next comma
  for (; buf[i]!=',' && buf[i]!=0 && iData<MAX_NMEA0183_MSG_LEN; i++, iData++) {
    CheckSum^=buf[i];
    Data[iData]=buf[i];
  }
  if (buf[i]!=',') { Clear(); return result; } // No separation after message code -> invalid message

  // Set the data and calculate checksum. Read until '*'
  for (; buf[i]!='*' && buf[i]!=0 && iData<MAX_NMEA0183_MSG_LEN; i++, iData++) {
    CheckSum^=buf[i];
    Data[iData]=buf[i];
    if (buf[i]==',') { // New field
      Data[iData]=0; // null termination for previous field
      Fields[_FieldCount]=iData+1;   // Set start of field
      _FieldCount++;
    }
  }

  if (buf[i]!='*') { Clear(); return false; } // No checksum -> invalid message
  Data[iData]=0; // null termination for previous field
  i++; // Pass '*';
  csMsg=(buf[i]<=57?buf[i]-48:buf[i]-55)<<4; i++;
  csMsg|=(buf[i]<=57?buf[i]-48:buf[i]-55);

  if (csMsg==CheckSum) {
    result=true;
  } else {
    Clear();
  }

  return result;
}

//*****************************************************************************
bool tNMEA0183Msg::AddToBuf(const char *data, char * &buf, size_t &BufSize) const {
  size_t len=strlen(data);

  if ( len+1>BufSize ) return false;

  strcpy(buf,data);
  buf+=len; BufSize-=len;

  return true;
}

//*****************************************************************************
bool tNMEA0183Msg::GetMessage(char *MsgData, size_t BufSize) const {
  if ( MsgData==0 || BufSize<14 ) return false;

  MsgData[0]=GetPrefix();
  MsgData++; BufSize--;
  AddToBuf(Sender(),MsgData,BufSize);
  AddToBuf(MessageCode(),MsgData,BufSize);

  for (int i=0; i<FieldCount(); i++) {
    AddToBuf(",",MsgData,BufSize);
    if ( !AddToBuf(Field(i),MsgData,BufSize) ) return false;
  }

  if ( BufSize<5 ) return false; // Is there room for termination *xx0
  sprintf(MsgData,"*%02X",GetCheckSum());
  return true;
}

//*****************************************************************************
bool tNMEA0183Msg::Init(const char *_MessageCode, const char *_Sender, char _Prefix) {
  Clear();
  size_t nSender=2;
  size_t nMessageCode=0;
  if ( _Sender==0 && (nSender=strlen(_Sender))>7 ) return false;
  if ( _MessageCode==0 || (nMessageCode=strlen(_MessageCode))>10 ) return false;

  Prefix=_Prefix;
  _MessageTime=millis();
  if ( _Sender!=0 && _Sender[0]!=0 && _Sender[1]!=0 ) {
    Data[0]=_Sender[0]; Data[1]=_Sender[1];
  } else {
    Data[0]='I'; Data[1]='I';
  }
  CheckSum^=Data[0];
  CheckSum^=Data[1];
  Data[2]=0;
  strcpy((Data+3),_MessageCode);
  iAddData=3+nMessageCode+1;

  for ( int i=3; Data[i]!=0; i++ ) CheckSum^=Data[i];

  return true;
}

//*****************************************************************************
bool tNMEA0183Msg::AddEmptyField() {
  if ( iAddData>=MAX_NMEA0183_MSG_LEN ||
       _FieldCount>=MAX_NMEA0183_MSG_FIELDS ) return false; // Is there room for any data

  Data[iAddData]=0;
  CheckSum^=',';
  Fields[_FieldCount]=iAddData;   // Set start of field
  iAddData++;
  _FieldCount++;

  return true;
}

//*****************************************************************************
bool tNMEA0183Msg::AddStrField(const char *FieldData) {
  if ( iAddData>=MAX_NMEA0183_MSG_LEN ||
       _FieldCount>=MAX_NMEA0183_MSG_FIELDS ) return false; // Is there room for any data

  int i=0;
  uint8_t cs=CheckSum;
  uint8_t iAdd=iAddData;

  Data[iAdd]=0;
  cs^=',';
  Fields[_FieldCount]=iAdd;   // Set start of field
  if ( FieldData!=0 ) {
    for (;iAdd<MAX_NMEA0183_MSG_LEN-1 && FieldData[i]!=0; i++,iAdd++) {
      Data[iAdd]=FieldData[i];
      cs^=FieldData[i];
    }
  }
  Data[iAdd]=0;
  if ( FieldData!=0 && FieldData[i]!=0 ) return false; // Return false, if FieldData does not fit.

  iAddData=iAdd+1;
  _FieldCount++;
  CheckSum=cs;
  return true;
}

//*****************************************************************************
bool tNMEA0183Msg::AddUInt32Field(uint32_t val) {
  if ( val==NMEA0183UInt32NA ) return AddEmptyField();

  if ( iAddData>=MAX_NMEA0183_MSG_LEN ||
       _FieldCount>=MAX_NMEA0183_MSG_FIELDS ) return false; // Is there room for any data

  int needSize;
  uint8_t cs=CheckSum;

  cs^=',';
  Fields[_FieldCount]=iAddData;   // Set start of field
  needSize=snprintf((Data+iAddData),MAX_NMEA0183_MSG_LEN-iAddData,"%lu",(unsigned long)val);
  ForceNullTermination();

  if ( needSize>MAX_NMEA0183_MSG_LEN-1-iAddData ) return false;

  for ( int i=iAddData; Data[i]!=0; i++ ) cs^=Data[i];
  iAddData+=needSize+1;

  _FieldCount++;
  CheckSum=cs;
  return true;
}

//*****************************************************************************
bool tNMEA0183Msg::AddDoubleField(double val, double multiplier, const char *Format, const char *Unit) {
  if ( NMEA0183IsNA(val) ) {
    bool ret=AddEmptyField();
    if ( Unit!=0 ) ret=AddStrField(Unit);
    return ret;
  }

  if ( iAddData>=MAX_NMEA0183_MSG_LEN ||
       _FieldCount>=MAX_NMEA0183_MSG_FIELDS ) return false; // Is there room for any data

  int needSize;
  uint8_t cs=CheckSum;

  cs^=',';
  Fields[_FieldCount]=iAddData;   // Set start of field
  #ifndef NO_PRINTF_DOUBLE_SUPPORT
  needSize=snprintf((Data+iAddData),MAX_NMEA0183_MSG_LEN-iAddData,Format,val*multiplier);
  ForceNullTermination();
  #else
  char StrVal[20];
  uint8_t precision=0;
  uint8_t width=1;
  bool Padding=false;
  // Try to solve requested width and precision
  char *DotPos=strchr(Format,'.');
  if ( DotPos!=0 ) {
    char *PrecPos=DotPos+1;
    if ( PrecPos!=0 ) precision=atoi(PrecPos);
    char *WidthPos=DotPos;
    while (WidthPos>Format && (*(WidthPos-1))!='%' ) WidthPos--;
    if ( WidthPos!=DotPos ) {
      width=atoi(WidthPos);
      if ( *WidthPos=='0' ) Padding=true;
    }
  }
  // Convert to string.
  dtostrf(val*multiplier, width, precision, StrVal);
  needSize=strlen(StrVal);
  if ( needSize<MAX_NMEA0183_MSG_LEN-iAddData ) {
    if ( Padding ) for ( char *s=StrVal; *s==' '; *s='0', s++);
    strcpy((Data+iAddData),StrVal);
  }
  #endif

  if ( needSize>MAX_NMEA0183_MSG_LEN-1-iAddData ) return false;

  for ( int i=iAddData; Data[i]!=0; i++ ) cs^=Data[i];
  iAddData+=needSize+1;

  _FieldCount++;
  CheckSum=cs;

  if ( Unit!=0 ) return AddStrField(Unit);

  return true;
}

//*****************************************************************************
bool tNMEA0183Msg::AddTimeField(double GPSTime, const char *Format) {
  return AddDoubleField(GPSTimeToNMEA0183Time(GPSTime),1,Format);
}

//*****************************************************************************
bool tNMEA0183Msg::AddDaysField(unsigned long DaysSince1970) {
  if ( DaysSince1970==NMEA0183UInt32NA  ) return AddEmptyField();

  return AddDoubleField(DaysToNMEA0183Date(DaysSince1970),1,"%06.0f");
}

//*****************************************************************************
bool tNMEA0183Msg::AddLatitudeField(double Latitude, const char *Format) {
  if ( Latitude==NMEA0183DoubleNA ) return AddEmptyField() & AddEmptyField();

  if ( iAddData>=MAX_NMEA0183_MSG_LEN-8 ||
       _FieldCount>=MAX_NMEA0183_MSG_FIELDS-1 ) return false; // Is there room for any data

  if ( ! AddDoubleField(DoubleToddmm((Latitude>=0?Latitude:-Latitude)),1,Format) ) return false; // abs generated -0.00 for 0.00??

  if ( Latitude>=0 ) return AddStrField("N");

  return AddStrField("S");
}

//*****************************************************************************
bool tNMEA0183Msg::AddLongitudeField(double Longitude, const char *Format) {
  if ( Longitude==NMEA0183DoubleNA ) return AddEmptyField() & AddEmptyField();

  if ( iAddData>=MAX_NMEA0183_MSG_LEN-8 ||
       _FieldCount>=MAX_NMEA0183_MSG_FIELDS-1 ) return false; // Is there room for any data

  if ( ! AddDoubleField(DoubleToddmm((Longitude>=0?Longitude:-Longitude)),1,Format) ) return false; // abs generated -0.00 for 0.00??

  if ( Longitude>=0 ) return AddStrField("E");

  return AddStrField("W");
}


//*****************************************************************************
void tNMEA0183Msg::Clear() {
  SourceID=0;
  Data[0]=0;  // Sender is empty
  Data[2]=0;  // Message code is empty
  iAddData=0;
  _FieldCount=0;
  Fields[0]=0;
  _MessageTime=0;
  CheckSum=0;
  Prefix=' ';
}

//*****************************************************************************
//void tNMEA0183Msg::PrintFields(Stream &port) const {
//}

//*****************************************************************************
void tNMEA0183Msg::Send(tNMEA0183Stream &port) const {
  if (FieldCount()==0) return;
  port.print(Prefix);
  port.print(Sender());
  port.print(MessageCode());
  for (int i=0; i<FieldCount(); i++) {
    port.print(",");
    port.print(Field(i));
  }
//  port.print("*");
  char buf[7];
  sprintf(buf,"*%02X\r\n",CheckSum);
  port.print(buf);
  port.print("\r\n");
}

//*****************************************************************************
const char *tNMEA0183Msg::Field(uint8_t index) const {
  if (index<FieldCount()) {
    return Data+Fields[index];
  } else {
    return EmptyField;
  }
}

//*****************************************************************************
unsigned int tNMEA0183Msg::FieldLen(uint8_t index) const {
  if (index<FieldCount()) {
    return strlen(Data+Fields[index]);
  } else {
    return 0;
  }
}

//*****************************************************************************
double tNMEA0183Msg::GPSTimeToNMEA0183Time(double GPSTime) {
  if ( GPSTime==NMEA0183DoubleNA ) return GPSTime;

  double intpart;
  double NMEA0183Time=0;
  GPSTime=modf(GPSTime/3600,&intpart);
  NMEA0183Time+=(unsigned long)(intpart*10000);
  GPSTime=modf(GPSTime*60,&intpart);
  NMEA0183Time+=(unsigned long)(intpart*100);
  NMEA0183Time+=GPSTime*60;
  //GPSTime=modf(GPSTime*60,&intpart);
  //NMEA0183Time+=(unsigned long)(intpart);

  return NMEA0183Time;
}

//*****************************************************************************
double tNMEA0183Msg::DoubleToddmm(double val) {
  if ( val!=NMEA0183DoubleNA  ) {
    double intpart;
    val=modf(val,&intpart);
    val=intpart*100+val*60;
  }

  return val;
}

unsigned long tNMEA0183Msg::TimeTDaysTo1970Offset=tNMEA0183Msg::CalcTimeTDaysTo1970Offset();

//*****************************************************************************
unsigned long tNMEA0183Msg::elapsedDaysSince1970(time_t dt) {
  unsigned long days=dt/SECS_PER_DAY;

  days+=TimeTDaysTo1970Offset;

  return days;
}


#ifndef _Time_h
//*****************************************************************************
time_t tNMEA0183Msg::daysToTime_t(unsigned long val) {
  val-=TimeTDaysTo1970Offset;

  return val*SECS_PER_DAY;
}
#endif

//*****************************************************************************
unsigned long tNMEA0183Msg::CalcTimeTDaysTo1970Offset() {
  // Need better routine. Now just guess between 1.1.1970 and 1.1.2000
  tmElements_t tme;
  SetYear(tme,2010);
  SetMonth(tme,1);
  SetDay(tme,1);
  SetHour(tme,0);
  SetMin(tme,0);
  SetSec(tme,0);

  unsigned long days=makeTime(tme)/SECS_PER_DAY;

  return 14610-days;
}

//*****************************************************************************
unsigned long tNMEA0183Msg::DaysToNMEA0183Date(unsigned long val) {
  if ( val!=NMEA0183UInt32NA  ) {
    tmElements_t tm;
    #ifndef _Time_h
    time_t t=daysToTime_t(val);
    #else
    time_t t=val*86400;
    #endif
    breakTime(t, tm);
    val=(unsigned long)GetDay(tm)*10000+(unsigned long)GetMonth(tm)*100+((unsigned long)GetYear(tm)-2000);
  }

  return val;
}
