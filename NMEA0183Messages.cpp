/*
NMEA0183Messages.cpp

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

#include "NMEA0183Messages.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

const double pi=3.1415926535897932384626433832795;
const double kmhToms=1000.0/3600.0;
const double knToms=1852.0/3600.0;
const double degToRad=pi/180.0;
const double radToDeg=180.0/pi;
const double msTokmh=3600.0/1000.0;
const double msTokn=3600.0/1852.0;
const double nmTom=1.852*1000;
const double mToFathoms=0.546806649;
const double mToFeet=3.2808398950131;

//*****************************************************************************
void NMEA0183AddChecksum(char* msg) {
  unsigned int i=1; // First character not included in checksum
  uint8_t chkSum = 0;
  char ascChkSum[4];
  uint8_t tmp;

  while (msg[i] != '\0') {
    chkSum ^= msg[i];
    i++;
  }
  ascChkSum[0] ='*';
  tmp = chkSum/16;
  ascChkSum[1] = tmp > 9 ? 'A' + tmp-10 : '0' + tmp;
  tmp = chkSum%16;
  ascChkSum[2] = tmp > 9 ? 'A' + tmp-10 : '0' + tmp;
  ascChkSum[3] = '\0';
  strcat(msg, ascChkSum);
}

//*****************************************************************************
double NMEA0183GetDouble(const char *data) {
  double val=NMEA0183DoubleNA;
  if ( data==0 ) return val;

  for ( ;*data==' ';data++); // Pass spaces

  if ( *data!=0 ) {
    val=atof(data);
  }

  return val;
}

//*****************************************************************************
double NMEA0183GetDouble(const char *data, const double &multiplier) {
  double val=NMEA0183GetDouble(data);

  if ( val!=NMEA0183DoubleNA ) val*=multiplier;

  return val;
}

//*****************************************************************************
double LatLonToDouble(const char *data, const char sign) {
  double val=atof(data);
  double deg=floor(val/100);

  val=deg+(val-deg*100.0)/60.0;
  if ( sign=='S' || sign=='W' ) val=-val;

  return val;
}

//*****************************************************************************
double NMEA0183GPTimeToSeconds(const char *data) {
  double val=atof(data);
  double hh=floor(val/10000);
  double mm=floor((val-hh*10000)/100);

  val=hh*3600+mm*60+(val-hh*10000.0-mm*100);

  return val;
}

//*****************************************************************************
time_t NMEA0183GPSDateTimetotime_t(const char *dateStr, const char *timeStr) {
  tmElements_t TimeElements;
  char StrCvt[3]="00";

    if (timeStr!=0 && strlen(timeStr)>=6) {
      StrCvt[0]=timeStr[0]; StrCvt[1]=timeStr[1];
      tNMEA0183Msg::SetHour(TimeElements,atoi(StrCvt));
      StrCvt[0]=timeStr[2]; StrCvt[1]=timeStr[3];
      tNMEA0183Msg::SetMin(TimeElements,atoi(StrCvt));
      StrCvt[0]=timeStr[4]; StrCvt[1]=timeStr[5];
      tNMEA0183Msg::SetSec(TimeElements,atoi(StrCvt));
    } else {
      tNMEA0183Msg::SetHour(TimeElements,0);
      tNMEA0183Msg::SetMin(TimeElements,0);
      tNMEA0183Msg::SetSec(TimeElements,0);
    }

    if (dateStr!=0 && strlen(dateStr)==6) {
      StrCvt[0]=dateStr[0]; StrCvt[1]=dateStr[1];
      tNMEA0183Msg::SetDay(TimeElements,atoi(StrCvt));
      StrCvt[0]=dateStr[2]; StrCvt[1]=dateStr[3];
      tNMEA0183Msg::SetMonth(TimeElements,atoi(StrCvt));
      StrCvt[0]=dateStr[4]; StrCvt[1]=dateStr[5];
      tNMEA0183Msg::SetYear(TimeElements,2000+atoi(StrCvt));
    } else {
      tNMEA0183Msg::SetYear(TimeElements,0);
      tNMEA0183Msg::SetMonth(TimeElements,0);
      tNMEA0183Msg::SetDay(TimeElements,0);
    }

    return tNMEA0183Msg::makeTime(TimeElements);
}

//*****************************************************************************
// $IIDBx,32.0,f,10.5,M,5.7,F*hh
bool NMEA0183SetDepth(tNMEA0183Msg &NMEA0183Msg, const char *Prefix, double Depth, const char *Src) {
  if ( !NMEA0183Msg.Init(Prefix,Src) ) return false;
  if ( !NMEA0183Msg.AddDoubleField(Depth,mToFeet,tNMEA0183Msg::DefDoubleFormat,"f") ) return false;
  if ( !NMEA0183Msg.AddDoubleField(Depth,1,tNMEA0183Msg::DefDoubleFormat,"M") ) return false;
  if ( !NMEA0183Msg.AddDoubleField(Depth,mToFathoms,tNMEA0183Msg::DefDoubleFormat,"F") ) return false;

  return true;
}

//*****************************************************************************
// $IIDBK,32.0,f,10.5,M,5.7,F*hh
bool NMEA0183SetDBK(tNMEA0183Msg &NMEA0183Msg, double Depth, const char *Src) {
  return NMEA0183SetDepth(NMEA0183Msg,"DBK",Depth,Src);
}

//*****************************************************************************
// $IIDBS,32.0,f,10.5,M,5.7,F*hh
bool NMEA0183SetDBS(tNMEA0183Msg &NMEA0183Msg, double Depth, const char *Src) {
  return NMEA0183SetDepth(NMEA0183Msg,"DBS",Depth,Src);
}

//*****************************************************************************
// $IIDBT,32.0,f,10.5,M,5.7,F*hh
bool NMEA0183SetDBT(tNMEA0183Msg &NMEA0183Msg, double Depth, const char *Src) {
  return NMEA0183SetDepth(NMEA0183Msg,"DBT",Depth,Src);
}

//*****************************************************************************
bool NMEA0183SetDBx(tNMEA0183Msg &NMEA0183Msg, double DepthBelowTransducer, double Offset, const char *Src) {
  if ( !NMEA0183IsNA(Offset) ) {
    double Depth=( !NMEA0183IsNA(DepthBelowTransducer)?DepthBelowTransducer+Offset:DepthBelowTransducer );
    if ( Offset>0 ) {
      return NMEA0183SetDBS(NMEA0183Msg,Depth,Src);
    } else {
      return NMEA0183SetDBK(NMEA0183Msg,Depth,Src);
    }
  }

  return NMEA0183SetDBT(NMEA0183Msg,DepthBelowTransducer,Src);
}

//*****************************************************************************
// $IIDPT,10.5,0.9*hh
bool NMEA0183SetDPT(tNMEA0183Msg &NMEA0183Msg, double DepthBelowTransducer, double Offset, const char *Src) {
  if ( !NMEA0183Msg.Init("DPT",Src) ) return false;
  if ( !NMEA0183Msg.AddDoubleField(DepthBelowTransducer) ) return false;
  if ( !NMEA0183Msg.AddDoubleField(Offset) ) return false;

  return true;
}

//*****************************************************************************
// $GPGGA,182435.00,6023.20859,N,02219.99442,E,2,10,0.9,4.0,M,20.6,M,5.0,0120*4D
bool NMEA0183ParseGGA_nc(const tNMEA0183Msg &NMEA0183Msg, double &GPSTime, double &Latitude, double &Longitude,
                      int &GPSQualityIndicator, int &SatelliteCount, double &HDOP, double &Altitude, double &GeoidalSeparation,
                      double &DGPSAge, int &DGPSReferenceStationID) {
  bool result=( NMEA0183Msg.FieldCount()>=14 );

  if ( result ) {
    GPSTime=NMEA0183GPTimeToSeconds(NMEA0183Msg.Field(0));
    Latitude=LatLonToDouble(NMEA0183Msg.Field(1),NMEA0183Msg.Field(2)[0]);
    Longitude=LatLonToDouble(NMEA0183Msg.Field(3),NMEA0183Msg.Field(4)[0]);
    GPSQualityIndicator=atoi(NMEA0183Msg.Field(5));
    SatelliteCount=atoi(NMEA0183Msg.Field(6));
    HDOP=atof(NMEA0183Msg.Field(7));
    Altitude=atof(NMEA0183Msg.Field(8));
    // Check units of antenna altitude NMEA0183Msg.Field(9)
    GeoidalSeparation=atof(NMEA0183Msg.Field(10));
    // Check units of GeoidalSeparation NMEA0183Msg.Field(11)
    DGPSAge=atof(NMEA0183Msg.Field(12));
    DGPSReferenceStationID=atoi(NMEA0183Msg.Field(13));
  }

  return result;
}

//*****************************************************************************
bool NMEA0183SetGGA(tNMEA0183Msg &NMEA0183Msg, double GPSTime, double Latitude, double Longitude,
          	uint32_t GPSQualityIndicator, uint32_t SatelliteCount, double HDOP, double Altitude, double GeoidalSeparation,
	          double DGPSAge, uint32_t DGPSReferenceStationID, const char *Src) {

  if ( !NMEA0183Msg.Init("GGA",Src) ) return false;
  if ( !NMEA0183Msg.AddTimeField(GPSTime) ) return false;
  if ( !NMEA0183Msg.AddLatitudeField(Latitude) ) return false;
  if ( !NMEA0183Msg.AddLongitudeField(Longitude) ) return false;
  if ( !NMEA0183Msg.AddUInt32Field(GPSQualityIndicator) ) return false;
  if ( !NMEA0183Msg.AddUInt32Field(SatelliteCount) ) return false;
  if ( !NMEA0183Msg.AddDoubleField(HDOP) ) return false;
  if ( !NMEA0183Msg.AddDoubleField(Altitude) ) return false;
  if ( !NMEA0183Msg.AddDoubleField(GeoidalSeparation) ) return false;
  if ( !NMEA0183Msg.AddDoubleField(DGPSAge) ) return false;
  if ( !NMEA0183Msg.AddUInt32Field(DGPSReferenceStationID) ) return false;

  return true;
}

//*****************************************************************************
//$GPGLL,5246.241,N,00506.648,E,155957,A*2B
//$GPGLL,,,,,155648,*5B
bool NMEA0183ParseGLL_nc(const tNMEA0183Msg &NMEA0183Msg, tGLL &GLL) {

  bool result=( NMEA0183Msg.FieldCount()>= 6);

  if ( result ) {
    GLL.latitude=LatLonToDouble(NMEA0183Msg.Field(0),NMEA0183Msg.Field(1)[0]);
    GLL.longitude=LatLonToDouble(NMEA0183Msg.Field(2),NMEA0183Msg.Field(3)[0]);
    GLL.GPSTime=NMEA0183GPTimeToSeconds(NMEA0183Msg.Field(4));
    GLL.status=NMEA0183Msg.Field(5)[0];
  }
  return result;
}

//*****************************************************************************
bool NMEA0183SetGLL(tNMEA0183Msg &NMEA0183Msg, double GPSTime, double Latitude, double Longitude, const char *Src) {

  if ( !NMEA0183Msg.Init("GLL",Src) ) return false;
  if ( !NMEA0183Msg.AddLatitudeField(Latitude) ) return false;
  if ( !NMEA0183Msg.AddLongitudeField(Longitude) ) return false;
  if ( !NMEA0183Msg.AddTimeField(GPSTime) ) return false;
  if ( GPSTime!=NMEA0183DoubleNA && Latitude!=NMEA0183DoubleNA && Longitude!=NMEA0183DoubleNA ) {
	//Data Active
	if ( !NMEA0183Msg.AddStrField("A") ) return false;
  } else {
	//Data Void (invalid)
	if ( !NMEA0183Msg.AddStrField("V") ) return false;
  }
  return true;
}


//*****************************************************************************
//$GPRMB,A,0.15,R,WOUBRG,WETERB,5213.400,N,00438.400,E,009.4,180.2,,V*07
bool NMEA0183ParseRMB_nc(const tNMEA0183Msg &NMEA0183Msg, tRMB &RMB) {

  bool result=( NMEA0183Msg.FieldCount()>=13 );

  if ( result ) {

  //Ignore Field(0). Assume status is OK.
	RMB.status=NMEA0183Msg.Field(0)[0];
  RMB.xte=atof(NMEA0183Msg.Field(1))*nmTom;
	//Left is negative in NMEA2000. Right is positive.
	if (NMEA0183Msg.Field(2)[0]=='R') RMB.xte=-RMB.xte;
    strncpy(RMB.originID,NMEA0183Msg.Field(3),sizeof(RMB.originID)/sizeof(char));
    RMB.originID[sizeof(RMB.originID)/sizeof(char)-1]='\0';
    strncpy(RMB.destID,NMEA0183Msg.Field(4),sizeof(RMB.destID)/sizeof(char));
    RMB.destID[sizeof(RMB.destID)/sizeof(char)-1]='\0';
    RMB.latitude=LatLonToDouble(NMEA0183Msg.Field(5),NMEA0183Msg.Field(6)[0]);
    RMB.longitude=LatLonToDouble(NMEA0183Msg.Field(7),NMEA0183Msg.Field(8)[0]);
    RMB.dtw=atof(NMEA0183Msg.Field(9))*nmTom;
    RMB.btw=atof(NMEA0183Msg.Field(10))*degToRad;
    RMB.vmg=atof(NMEA0183Msg.Field(11))*knToms;
	  RMB.arrivalAlarm=NMEA0183Msg.Field(12)[0];
  }

  return result;

}

//*****************************************************************************
// $GPRMC,092348.00,A,6035.04228,N,02115.15472,E,0.01,272.61,060815,7.2,E,D*34
bool NMEA0183ParseRMC_nc(const tNMEA0183Msg &NMEA0183Msg, double &GPSTime, double &Latitude, double &Longitude,
                      double &TrueCOG, double &SOG, unsigned long &DaysSince1970, double &Variation, time_t *DateTime) {
  bool result=( NMEA0183Msg.FieldCount()>=11 );

  if ( result ) {
    time_t lDT;

    GPSTime=NMEA0183GPTimeToSeconds(NMEA0183Msg.Field(0));
    Latitude=LatLonToDouble(NMEA0183Msg.Field(2),NMEA0183Msg.Field(3)[0]);
    Longitude=LatLonToDouble(NMEA0183Msg.Field(4),NMEA0183Msg.Field(5)[0]);
    SOG=atof(NMEA0183Msg.Field(6))*knToms;
    TrueCOG=atof(NMEA0183Msg.Field(7))*degToRad;

    lDT=NMEA0183GPSDateTimetotime_t(NMEA0183Msg.Field(8),0)+floor(GPSTime);
    DaysSince1970=tNMEA0183Msg::elapsedDaysSince1970(lDT);
    if (DateTime!=0) *DateTime=lDT;
    Variation=atof(NMEA0183Msg.Field(9))*degToRad; if (NMEA0183Msg.Field(10)[0]=='W') Variation=-Variation;
  }

  return result;
}

//*****************************************************************************
bool NMEA0183SetRMC(tNMEA0183Msg &NMEA0183Msg, double GPSTime, double Latitude, double Longitude,
                      double TrueCOG, double SOG, unsigned long DaysSince1970, double Variation, const char *Src) {

  if ( SOG!=NMEA0183DoubleNA && SOG<0 ) {
    if ( TrueCOG!=NMEA0183DoubleNA  ) TrueCOG+=pi;
  }
  if ( TrueCOG!=NMEA0183DoubleNA  ) TrueCOG=fmod(TrueCOG,2*pi);

  if ( !NMEA0183Msg.Init("RMC",Src) ) return false;
  if ( !NMEA0183Msg.AddTimeField(GPSTime) ) return false;
  if ( GPSTime!=NMEA0183DoubleNA ) {
    NMEA0183Msg.AddStrField("A");
  } else {
    NMEA0183Msg.AddEmptyField();
  }
  if ( !NMEA0183Msg.AddLatitudeField(Latitude) ) return false;
  if ( !NMEA0183Msg.AddLongitudeField(Longitude) ) return false;
  if ( !NMEA0183Msg.AddDoubleField(SOG,msTokn) ) return false;
  if ( !NMEA0183Msg.AddDoubleField(TrueCOG,radToDeg) ) return false;
  if ( !NMEA0183Msg.AddDaysField(DaysSince1970) ) return false;
  if ( !NMEA0183IsNA(Variation)  ) {
    if ( !NMEA0183Msg.AddDoubleField((Variation>=0?Variation:-Variation),radToDeg) ) return false; // abs generated -0.00 for 0.00??
    if ( Variation>=0 ) {
      if ( !NMEA0183Msg.AddStrField("E") ) return false;
    } else {
      if ( !NMEA0183Msg.AddStrField("W") ) return false;
    }
  } else {
    if ( !NMEA0183Msg.AddStrField("") ) return false;
    if ( !NMEA0183Msg.AddStrField("") ) return false;
  }

  return true;
}


//*****************************************************************************
// $GPVTG,89.34,T,81.84,M,0.00,N,0.01,K*24
bool NMEA0183ParseVTG_nc(const tNMEA0183Msg &NMEA0183Msg, double &TrueCOG, double &MagneticCOG, double &SOG) {
  bool result=( NMEA0183Msg.FieldCount()>=8 );

  if ( result ) {
    TrueCOG=atof(NMEA0183Msg.Field(0))*degToRad;
    MagneticCOG=atof(NMEA0183Msg.Field(2))*degToRad;
    if (NMEA0183Msg.Field(6)[0]!=0) {  // km/h is valid
      SOG=atof(NMEA0183Msg.Field(6))*kmhToms;
    } else {
      SOG=atof(NMEA0183Msg.Field(4))*knToms;
    }
  }

  return result;
}

bool NMEA0183SetVTG(tNMEA0183Msg &NMEA0183Msg, double TrueCOG, double MagneticCOG, double SOG, const char *Src) {
  if ( SOG!=NMEA0183DoubleNA && SOG<0 ) {
    if ( TrueCOG!=NMEA0183DoubleNA  ) TrueCOG+=pi;
    if ( MagneticCOG!=NMEA0183DoubleNA  ) MagneticCOG+=pi;
  }
  if ( TrueCOG!=NMEA0183DoubleNA  ) TrueCOG=fmod(TrueCOG,2*pi);
  if ( MagneticCOG!=NMEA0183DoubleNA  ) MagneticCOG=fmod(MagneticCOG,2*pi);

  if ( !NMEA0183Msg.Init("VTG",Src) ) return false;
  if ( !NMEA0183Msg.AddDoubleField(TrueCOG,radToDeg,tNMEA0183Msg::DefDoubleFormat,"T") ) return false;
  if ( !NMEA0183Msg.AddDoubleField(MagneticCOG,radToDeg,tNMEA0183Msg::DefDoubleFormat,"M") ) return false;
  if ( !NMEA0183Msg.AddDoubleField(SOG,msTokn,tNMEA0183Msg::DefDoubleFormat,"N") ) return false;
  if ( !NMEA0183Msg.AddDoubleField(SOG,msTokmh,tNMEA0183Msg::DefDoubleFormat,"K") ) return false;

  return true;
}

//*****************************************************************************
// $VWVHW,x.x,T,x.x,M,x.x,N,x.x,K*24
bool NMEA0183ParseVHW_nc(const tNMEA0183Msg &NMEA0183Msg, double &TrueHeading, double &MagneticHeading, double &SOW) {
  bool result=( NMEA0183Msg.FieldCount()>=8 );

  if ( result ) {
    TrueHeading=NMEA0183GetDouble(NMEA0183Msg.Field(0),degToRad);
    MagneticHeading=NMEA0183GetDouble(NMEA0183Msg.Field(2),degToRad);
    if (NMEA0183Msg.Field(6)[0]!=0) {  // km/h is valid
      SOW=NMEA0183GetDouble(NMEA0183Msg.Field(6),kmhToms);
    } else {
      SOW=NMEA0183GetDouble(NMEA0183Msg.Field(4),knToms);
    }
  }

  return result;
}

//*****************************************************************************
// VHW - Water speed and heading
bool NMEA0183SetVHW(tNMEA0183Msg &NMEA0183Msg, double TrueHeading, double MagneticHeading, double BoatSpeed, const char *Src) {
  if ( !NMEA0183Msg.Init("VHW",Src) ) return false;
  if ( !NMEA0183Msg.AddDoubleField(TrueHeading,radToDeg,tNMEA0183Msg::DefDoubleFormat,"T") ) return false;
  if ( !NMEA0183Msg.AddDoubleField(MagneticHeading,radToDeg,tNMEA0183Msg::DefDoubleFormat,"M") ) return false;
  if ( !NMEA0183Msg.AddDoubleField(BoatSpeed,msTokn,tNMEA0183Msg::DefDoubleFormat,"N") ) return false;
  if ( !NMEA0183Msg.AddDoubleField(BoatSpeed,msTokmh,tNMEA0183Msg::DefDoubleFormat,"K") ) return false;

  return true;
}

//*****************************************************************************
// Helper to avoid enabling floating point support
int sprintfDouble2(char* msg, double val)
{
  int valInt = val;
  int valFrag;

  val -= valInt;
  valFrag = val * 100;

  if (valFrag < 10) {
	  return sprintf(msg,"%d.0%d",valInt, valFrag);
  } else {
	  return sprintf(msg,"%d,%d", valInt, valFrag);
  }
}

//*****************************************************************************
bool NMEA0183BuildVTG(char* msg, const char Src[], double TrueCOG, double MagneticCOG, double SOG)
{
  char scratch[20];

  msg[0] = '$';
  msg[1] = Src[0];
  msg[2] = Src[1];
  strcpy(&msg[3],"VTG,");
  TrueCOG /= degToRad;
  if (TrueCOG < 360) {
    sprintfDouble2(scratch, TrueCOG);
    strcat(msg,scratch);
  }
  strcat(msg,",T,");
  MagneticCOG /= degToRad;
  if (MagneticCOG < 360) {
	sprintfDouble2(scratch, MagneticCOG);
    strcat(msg,scratch);
  }
  strcat(msg,",M,");
  if (SOG >= 0.00) {
     sprintfDouble2(scratch, SOG*msTokn);
     strcat(msg, scratch);
     strcat(msg,",N,,K");
  } else {
     strcat(msg, ",N,,K,");
  }

  NMEA0183AddChecksum(msg);
  return true;
}

//*****************************************************************************
// $HEROT,4.71,A*1B
bool NMEA0183ParseROT_nc(const tNMEA0183Msg &NMEA0183Msg,double &RateOfTurn) {
  bool result=( NMEA0183Msg.FieldCount()>=2 );
  if ( result ) {
    RateOfTurn=atof(NMEA0183Msg.Field(0))*degToRad;
  }

  return result;
}

bool NMEA0183SetROT(tNMEA0183Msg &NMEA0183Msg, double RateOfTurn, const char *Src) {
  if ( !NMEA0183Msg.Init("ROT",Src) ) return false;
  if ( !NMEA0183Msg.AddDoubleField(RateOfTurn,radToDeg,tNMEA0183Msg::DefDoubleFormat,"A") ) return false;
  return true;
}

//*****************************************************************************
// $HEHDT,244.71,T*1B
bool NMEA0183ParseHDT_nc(const tNMEA0183Msg &NMEA0183Msg,double &TrueHeading) {
  bool result=( NMEA0183Msg.FieldCount()>=2 );
  if ( result ) {
    TrueHeading=atof(NMEA0183Msg.Field(0))*degToRad;
  }

  return result;
}

bool NMEA0183SetHDT(tNMEA0183Msg &NMEA0183Msg, double Heading, const char *Src) {
  if ( !NMEA0183Msg.Init("HDT",Src) ) return false;
  if ( !NMEA0183Msg.AddDoubleField(Heading,radToDeg) ) return false;
  if ( !NMEA0183Msg.AddStrField("T") ) return false;
  return true;
}

//*****************************************************************************
// $HEHDM,244.71,M*1B
bool NMEA0183ParseHDM_nc(const tNMEA0183Msg &NMEA0183Msg,double &MagneticHeading) {
  bool result=( NMEA0183Msg.FieldCount()>=2 );
  if ( result ) {
    MagneticHeading=atof(NMEA0183Msg.Field(0))*degToRad;
  }

  return result;
}

bool NMEA0183SetHDM(tNMEA0183Msg &NMEA0183Msg, double Heading, const char *Src) {
  if ( !NMEA0183Msg.Init("HDM",Src) ) return false;
  if ( !NMEA0183Msg.AddDoubleField(Heading,radToDeg) ) return false;
  if ( !NMEA0183Msg.AddStrField("M") ) return false;
  return true;
}

//*****************************************************************************
bool NMEA0183SetHDG(tNMEA0183Msg &NMEA0183Msg, double Heading, double Deviation, double Variation, const char *Src) {
  if ( !NMEA0183Msg.Init("HDG",Src) ) return false;
  if ( !NMEA0183Msg.AddDoubleField(Heading,radToDeg) ) return false;
  // Set deviation, if value is valid
  if ( !NMEA0183IsNA(Deviation) ) {
    if ( !NMEA0183Msg.AddDoubleField((Deviation>=0?Deviation:-Deviation),radToDeg) ) return false; // abs generated -0.00 for 0.00??
    if ( Deviation>=0 ) {
      if ( !NMEA0183Msg.AddStrField("E") ) return false;
    } else {
      if ( !NMEA0183Msg.AddStrField("W") ) return false;
    }
  } else {
    if ( !NMEA0183Msg.AddStrField("") ) return false;
    if ( !NMEA0183Msg.AddStrField("") ) return false;
  }
  // Set Variation, if value is valid
  if ( !NMEA0183IsNA(Variation) ) {
    if ( !NMEA0183Msg.AddDoubleField((Variation>=0?Variation:-Variation),radToDeg) ) return false; // abs generated -0.00 for 0.00??
    if ( Variation>=0 ) {
      if ( !NMEA0183Msg.AddStrField("E") ) return false;
    } else {
      if ( !NMEA0183Msg.AddStrField("W") ) return false;
    }
  } else {
    if ( !NMEA0183Msg.AddStrField("") ) return false;
    if ( !NMEA0183Msg.AddStrField("") ) return false;
  }

  return true;
}

//*****************************************************************************
// !AIVDM,1,1,,B,177KQJ5000G?tO`K>RA1wUbN0TKH,0*5C
// PkgCnt (1)
// PkgNmb (1)
// SeqMessageId (empty)
// Radio Channel Code (B): A/B or 1/2
// Payload - 6bit encoded
// Fillbits (0)
bool NMEA0183ParseVDM_nc(const tNMEA0183Msg &NMEA0183Msg,
			uint8_t &pkgCnt, uint8_t &pkgNmb,
			unsigned int &seqMessageId, char &channel,
			unsigned int &length, char *bitstream,
			unsigned int &fillBits)
{
  bool result=( NMEA0183Msg.FieldCount()>=6);

  if ( result ) {
    unsigned int payloadLen = NMEA0183Msg.FieldLen(4);
    if ( payloadLen > length)
      return false;
    length = payloadLen;
    memcpy(bitstream, NMEA0183Msg.Field(4), length);
    fillBits=atoi(NMEA0183Msg.Field(5));
    seqMessageId=atoi(NMEA0183Msg.Field(2));
    pkgNmb=atoi(NMEA0183Msg.Field(1));
    pkgCnt=atoi(NMEA0183Msg.Field(0));
    channel = *NMEA0183Msg.Field(3);
    if (channel == '1') channel = 'A';
    if (channel == '2') channel = 'B';
  }

  return result;
}

//*****************************************************************************
//$GPRTE,2,1,c,0,W3IWI,DRIVWY,32CEDR,32-29,32BKLD,32-I95,32-US1,BW-32,BW-198*69
bool NMEA0183ParseRTE_nc(const tNMEA0183Msg &NMEA0183Msg, tRTE &tRTE) {

    bool result=( NMEA0183Msg.FieldCount()>=4);

    if ( result ) {

	 tRTE.nrOfsentences = atoi(NMEA0183Msg.Field(0));
	 tRTE.currSentence = atoi(NMEA0183Msg.Field(1));
	 tRTE.type = NMEA0183Msg.Field(2)[0];
	 tRTE.routeID = atoi(NMEA0183Msg.Field(3));
	 tRTE.nrOfwp = NMEA0183Msg.FieldCount() - 4;

	 byte wpIndex=0;
	 //Copy WP's into 1D-array separated by null terminator.
	 for (byte i=4; i < NMEA0183Msg.FieldCount(); i++) {
		for (byte j=0; j < NMEA0183Msg.FieldLen(i); j++) {
			tRTE._wp[wpIndex++] = NMEA0183Msg.Field(i)[j];
		}
		tRTE._wp[wpIndex++] = 0;
	 }
    }
    return result;
}

//*****************************************************************************
//$GPWPL,5208.700,N,00438.600,E,MOLENB*4D
bool NMEA0183ParseWPL_nc(const tNMEA0183Msg &NMEA0183Msg, tWPL &wpl) {

    bool result=( NMEA0183Msg.FieldCount()>=5);

    if ( result ) {
      wpl.latitude = LatLonToDouble(NMEA0183Msg.Field(0),NMEA0183Msg.Field(1)[0]);
      wpl.longitude = LatLonToDouble(NMEA0183Msg.Field(2),NMEA0183Msg.Field(3)[0]);
      strncpy(wpl.name,NMEA0183Msg.Field(4),sizeof(wpl.name)/sizeof(char));
      wpl.name[sizeof(wpl.name)/sizeof(char)-1]='\0';
	  }
    return result;
}

//*****************************************************************************
//$GPBOD,001.1,T,003.4,M,WETERB,WOUBRG*49
bool NMEA0183ParseBOD_nc(const tNMEA0183Msg &NMEA0183Msg, tBOD &bod) {

    bool result=( NMEA0183Msg.FieldCount()>=6);

    if ( result ) {
      bod.trueBearing = atof(NMEA0183Msg.Field(0))*degToRad;
      bod.magBearing = atof(NMEA0183Msg.Field(2))*degToRad;
      strncpy(bod.destID,NMEA0183Msg.Field(4),sizeof(bod.destID)/sizeof(char));
      bod.destID[sizeof(bod.destID)/sizeof(char)-1]='\0';
      strncpy(bod.originID,NMEA0183Msg.Field(5),sizeof(bod.originID)/sizeof(char));
      bod.originID[sizeof(bod.originID)/sizeof(char)-1]='\0';
	  }
    return result;
}

//*****************************************************************************
// MWV - Wind Speed and Angle
//$IIMWV,120.1,R,9.5,M,A,a*hh
bool NMEA0183ParseMWV_nc(const tNMEA0183Msg &NMEA0183Msg,double &WindAngle, tNMEA0183WindReference &Reference, double &WindSpeed) {
  bool result=( NMEA0183Msg.FieldCount()>=4 );

  if ( result ) {
    WindAngle=atof(NMEA0183Msg.Field(0));
    switch ( NMEA0183Msg.Field(1)[0] ) {
      case 'T' : Reference=NMEA0183Wind_True; break;
      case 'R' :
      default : Reference=NMEA0183Wind_Apparent; break;
    }
    WindSpeed=atof(NMEA0183Msg.Field(2));
    switch ( NMEA0183Msg.Field(3)[0] ) {
      case 'K' : WindSpeed*=kmhToms; break;
      case 'N' : WindSpeed*=knToms; break;
      case 'M' :
      default : ;
    }
  }

  return result;
}

bool NMEA0183SetMWV(tNMEA0183Msg &NMEA0183Msg, double WindAngle, tNMEA0183WindReference Reference, double WindSpeed, const char *Src) {
  if ( !NMEA0183Msg.Init("MWV",Src) ) return false;
  if ( !NMEA0183Msg.AddDoubleField(WindAngle) ) return false;
  if ( !NMEA0183Msg.AddStrField(Reference==NMEA0183Wind_True?"T":"R") ) return false;
  if ( !NMEA0183Msg.AddDoubleField(WindSpeed) ) return false;
  if ( !NMEA0183Msg.AddStrField("M") ) return false;
  if ( !NMEA0183Msg.AddStrField("A") ) return false;
  return true;
}
