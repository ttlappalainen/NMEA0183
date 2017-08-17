// Demo: NMEA2000/NMEA0183 library.
// Converts some NMEA 2000 messages to NMEA 0183 messages to given output stream.

#define N2k_CAN_INT_PIN 21
#include <Arduino.h>
#include "NMEA2000_CAN.h"
#include "NMEA0183.h"
#include "N2kMessages.h"
#include "NMEA0183Messages.h"

tNMEA0183 NMEA0183;

//*****************************************************************************
class tN2kPosInfoToNMEA0183 : public tNMEA2000::tMsgHandler {
protected:
  static const unsigned long RMCPeriod=1000;
  double Latitude;
  double Longitude;
  double Altitude;
  double Variation;
  double Heading;
  double COG;
  double SOG;
  unsigned long LastPosSend;
  uint16_t DaysSince1970;
  double SecondsSinceMidnight;
  unsigned long NextRMCSend;

  tNMEA0183 *pNMEA0183;

protected:
  void HandleHeading(const tN2kMsg &N2kMsg); // 127250
  void HandleVariation(const tN2kMsg &N2kMsg); // 127258
  void HandlePosition(const tN2kMsg &N2kMsg); // 129025
  void HandleCOGSOG(const tN2kMsg &N2kMsg); // 129026
  void HandleGNSS(const tN2kMsg &N2kMsg); // 129029
  void SetNextRMCSend() { NextRMCSend=millis()+RMCPeriod; }
  void SendRMC();
public:
  tN2kPosInfoToNMEA0183(tNMEA2000 *_pNMEA2000, tNMEA0183 *_pNMEA0183) : tNMEA2000::tMsgHandler(0,_pNMEA2000) {
    pNMEA0183=_pNMEA0183;
    Latitude=N2kDoubleNA; Longitude=N2kDoubleNA; Altitude=N2kDoubleNA;
    Variation=N2kDoubleNA; Heading=N2kDoubleNA; COG=N2kDoubleNA; SOG=N2kDoubleNA;
    SecondsSinceMidnight=N2kDoubleNA; DaysSince1970=N2kUInt16NA;
    LastPosSend=0;
    NextRMCSend=millis()+RMCPeriod;
  }
  void HandleMsg(const tN2kMsg &N2kMsg);
  void SendMessages() { SendRMC(); }
};

tN2kPosInfoToNMEA0183 *N2kPosInfoToNMEA0183;

//*****************************************************************************
void tN2kPosInfoToNMEA0183::HandleMsg(const tN2kMsg &N2kMsg) {
  switch (N2kMsg.PGN) {
    case 127250: HandleHeading(N2kMsg);
    case 127258: HandleVariation(N2kMsg);
    case 129025: HandlePosition(N2kMsg);
    case 129026: HandleCOGSOG(N2kMsg);
    case 129029: HandleGNSS(N2kMsg);
  }
}

//*****************************************************************************
void tN2kPosInfoToNMEA0183::HandleHeading(const tN2kMsg &N2kMsg) {
unsigned char SID;
tN2kHeadingReference ref;
double _Deviation;
double _Variation;
tNMEA0183Msg NMEA0183Msg;

  if ( ParseN2kHeading(N2kMsg, SID, Heading, _Deviation, _Variation, ref) ) {
    if ( ref==N2khr_magnetic ) {
      Heading-=Variation;
    }
    if ( NMEA0183SetHDG(NMEA0183Msg,Heading-Variation,_Deviation,Variation) ) {
      NMEA0183.SendMessage(NMEA0183Msg);
    }
  }
}

//*****************************************************************************
void tN2kPosInfoToNMEA0183::HandleVariation(const tN2kMsg &N2kMsg) {
unsigned char SID;
tN2kMagneticVariation Source;

  ParseN2kMagneticVariation(N2kMsg,SID,Source,DaysSince1970,Variation);
}

//*****************************************************************************
void tN2kPosInfoToNMEA0183::HandlePosition(const tN2kMsg &N2kMsg) {

  if ( ParseN2kPGN129025(N2kMsg, Latitude, Longitude) ) {
  }
}

//*****************************************************************************
void tN2kPosInfoToNMEA0183::HandleCOGSOG(const tN2kMsg &N2kMsg) {
unsigned char SID;
tN2kHeadingReference HeadingReference;
tNMEA0183Msg NMEA0183Msg;

  if ( ParseN2kCOGSOGRapid(N2kMsg,SID,HeadingReference,COG,SOG) ) {
    if ( HeadingReference==N2khr_magnetic ) COG-=Variation;
    if ( NMEA0183SetVTG(NMEA0183Msg,COG,COG-Variation,SOG) ) {
      NMEA0183.SendMessage(NMEA0183Msg);
    }
  }
}

//*****************************************************************************
void tN2kPosInfoToNMEA0183::HandleGNSS(const tN2kMsg &N2kMsg) {
unsigned char SID;
tN2kGNSStype GNSStype;
tN2kGNSSmethod GNSSmethod;
unsigned char nSatellites;
double HDOP;
double PDOP;
double GeoidalSeparation;
unsigned char nReferenceStations;
tN2kGNSStype ReferenceStationType;
uint16_t ReferenceSationID;
double AgeOfCorrection;

  if ( ParseN2kGNSS(N2kMsg,SID,DaysSince1970,SecondsSinceMidnight,Latitude,Longitude,Altitude,GNSStype,GNSSmethod,
                    nSatellites,HDOP,PDOP,GeoidalSeparation,
                    nReferenceStations,ReferenceStationType,ReferenceSationID,AgeOfCorrection) ) {
  }
}

//*****************************************************************************
void tN2kPosInfoToNMEA0183::SendRMC() {
    if ( NextRMCSend<=millis() && Latitude!=N2kDoubleNA ) {
      tNMEA0183Msg NMEA0183Msg;
      if ( NMEA0183SetRMC(NMEA0183Msg,SecondsSinceMidnight,Latitude,Longitude,COG,SOG,DaysSince1970,Variation) ) {
        NMEA0183.SendMessage(NMEA0183Msg);
      }
      SetNextRMCSend();
    }
}

//*****************************************************************************
void setup() {
  Serial.begin(115200);
  NMEA2000.SetN2kCANMsgBufSize(8);
  NMEA2000.SetN2kCANReceiveFrameBufSize(100);
  NMEA2000.SetForwardStream(&Serial);  // PC output on due native port
  // NMEA2000.SetForwardType(tNMEA2000::fwdt_Text); // Show in clear text
  NMEA2000.EnableForward(false);                      // Disable all msg forwarding to USB (=Serial)
  NMEA0183.SetMessageStream(&Serial);
  N2kPosInfoToNMEA0183=new tN2kPosInfoToNMEA0183(&NMEA2000,&NMEA0183);
  NMEA2000.Open();
  NMEA0183.Open();
}

//*****************************************************************************
void loop() {
  NMEA2000.ParseMessages();
  NMEA0183.ParseMessages();
  N2kPosInfoToNMEA0183->SendMessages();
}

