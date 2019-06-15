/* 
NMEA0183Handlers.cpp

2015 Copyright (c) Kave Oy, www.kave.fi  All right reserved.

Author: Timo Lappalainen

  This library is free software; you can redistribute it and/or
  modify it as you like.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/
 
#include <N2kMsg.h>
#include <NMEA2000.h>
#include <N2kMessages.h>
#include <NMEA0183Messages.h>
#include "NMEA0183Handlers.h"

extern tNMEA0183 NMEA0183_Out;

struct tNMEA0183Handler {
  const char *Code;
  void (*Handler)(const tNMEA0183Msg &NMEA0183Msg); 
};

// Predefinition for functions to make it possible for constant definition for NMEA0183Handlers
void HandleRMC(const tNMEA0183Msg &NMEA0183Msg);
void HandleGGA(const tNMEA0183Msg &NMEA0183Msg);
void HandleHDT(const tNMEA0183Msg &NMEA0183Msg);
void HandleVTG(const tNMEA0183Msg &NMEA0183Msg);

// Internal variables
tNMEA2000 *pNMEA2000=0;
tBoatData *pBD=0;
Stream* NMEA0183HandlersDebugStream=0;

tNMEA0183Handler NMEA0183Handlers[]={
  {"GGA",&HandleGGA},
  {"HDT",&HandleHDT},
  {"VTG",&HandleVTG},
  {"RMC",&HandleRMC},
  {0,0}
};

// *****************************************************************************
void InitNMEA0183Handlers(tNMEA2000 *_NMEA2000, tBoatData *_BoatData) {
  pNMEA2000=_NMEA2000;
  pBD=_BoatData;
}

// *****************************************************************************
void DebugNMEA0183Handlers(Stream* _stream) {
  NMEA0183HandlersDebugStream=_stream;
}

// *****************************************************************************
tN2kGNSSmethod GNSMethofNMEA0183ToN2k(int Method) {
  switch (Method) {
    case 0: return N2kGNSSm_noGNSS;
    case 1: return N2kGNSSm_GNSSfix;
    case 2: return N2kGNSSm_DGNSS;
    default: return N2kGNSSm_noGNSS;  
  }
}

// *****************************************************************************
void HandleNMEA0183Msg(const tNMEA0183Msg &NMEA0183Msg) {
  int iHandler;
  // Find handler
  for (iHandler=0; NMEA0183Handlers[iHandler].Code!=0 && !NMEA0183Msg.IsMessageCode(NMEA0183Handlers[iHandler].Code); iHandler++);
  
  if (NMEA0183Handlers[iHandler].Code!=0) {
    NMEA0183Handlers[iHandler].Handler(NMEA0183Msg); 
  }
  
  // Forward all NMEA0183 messages to the NMEA0183 out stream
  NMEA0183_Out.SendMessage(NMEA0183Msg);
}

// NMEA0183 message Handler functions

// *****************************************************************************
void HandleRMC(const tNMEA0183Msg &NMEA0183Msg) {
  if (pBD==0) return;
  
  if (NMEA0183ParseRMC_nc(NMEA0183Msg,pBD->GPSTime,pBD->Latitude,pBD->Longitude,pBD->COG,pBD->SOG,pBD->DaysSince1970,pBD->Variation)) {
  } else if (NMEA0183HandlersDebugStream!=0) { NMEA0183HandlersDebugStream->println("Failed to parse RMC"); }
}

// *****************************************************************************
void HandleGGA(const tNMEA0183Msg &NMEA0183Msg) {
  if (pBD==0) return;
  
  if (NMEA0183ParseGGA_nc(NMEA0183Msg,pBD->GPSTime,pBD->Latitude,pBD->Longitude,
                   pBD->GPSQualityIndicator,pBD->SatelliteCount,pBD->HDOP,pBD->Altitude,pBD->GeoidalSeparation,
                   pBD->DGPSAge,pBD->DGPSReferenceStationID)) {
    if (pNMEA2000!=0) {
      tN2kMsg N2kMsg;
      SetN2kMagneticVariation(N2kMsg,1,N2kmagvar_Calc,pBD->DaysSince1970,pBD->Variation);
      pNMEA2000->SendMsg(N2kMsg); 
      SetN2kGNSS(N2kMsg,1,pBD->DaysSince1970,pBD->GPSTime,pBD->Latitude,pBD->Longitude,pBD->Altitude,
                N2kGNSSt_GPS,GNSMethofNMEA0183ToN2k(pBD->GPSQualityIndicator),pBD->SatelliteCount,pBD->HDOP,0,
                pBD->GeoidalSeparation,1,N2kGNSSt_GPS,pBD->DGPSReferenceStationID,pBD->DGPSAge
                );
      pNMEA2000->SendMsg(N2kMsg); 
    }

    if (NMEA0183HandlersDebugStream!=0) {
      NMEA0183HandlersDebugStream->print("Time="); NMEA0183HandlersDebugStream->println(pBD->GPSTime);
      NMEA0183HandlersDebugStream->print("Latitude="); NMEA0183HandlersDebugStream->println(pBD->Latitude,5);
      NMEA0183HandlersDebugStream->print("Longitude="); NMEA0183HandlersDebugStream->println(pBD->Longitude,5);
      NMEA0183HandlersDebugStream->print("Altitude="); NMEA0183HandlersDebugStream->println(pBD->Altitude,1);
      NMEA0183HandlersDebugStream->print("GPSQualityIndicator="); NMEA0183HandlersDebugStream->println(pBD->GPSQualityIndicator);
      NMEA0183HandlersDebugStream->print("SatelliteCount="); NMEA0183HandlersDebugStream->println(pBD->SatelliteCount);
      NMEA0183HandlersDebugStream->print("HDOP="); NMEA0183HandlersDebugStream->println(pBD->HDOP);
      NMEA0183HandlersDebugStream->print("GeoidalSeparation="); NMEA0183HandlersDebugStream->println(pBD->GeoidalSeparation);
      NMEA0183HandlersDebugStream->print("DGPSAge="); NMEA0183HandlersDebugStream->println(pBD->DGPSAge);
      NMEA0183HandlersDebugStream->print("DGPSReferenceStationID="); NMEA0183HandlersDebugStream->println(pBD->DGPSReferenceStationID);
    }
  } else if (NMEA0183HandlersDebugStream!=0) { NMEA0183HandlersDebugStream->println("Failed to parse GGA"); }
}

#define PI_2 6.283185307179586476925286766559

// *****************************************************************************
void HandleHDT(const tNMEA0183Msg &NMEA0183Msg) {
  if (pBD==0) return;
  
  if (NMEA0183ParseHDT_nc(NMEA0183Msg,pBD->TrueHeading)) {
    if (pNMEA2000!=0) { 
      tN2kMsg N2kMsg;
      double MHeading=pBD->TrueHeading-pBD->Variation;
      while (MHeading<0) MHeading+=PI_2;
      while (MHeading>=PI_2) MHeading-=PI_2;
//      SetN2kMagneticHeading(N2kMsg,1,MHeading,0,pBD->Variation);
      SetN2kTrueHeading(N2kMsg,1,pBD->TrueHeading);
      pNMEA2000->SendMsg(N2kMsg);
    }
    if (NMEA0183HandlersDebugStream!=0) {
      NMEA0183HandlersDebugStream->print("True heading="); NMEA0183HandlersDebugStream->println(pBD->TrueHeading);
    }
  } else if (NMEA0183HandlersDebugStream!=0) { NMEA0183HandlersDebugStream->println("Failed to parse HDT"); }
}

// *****************************************************************************
void HandleVTG(const tNMEA0183Msg &NMEA0183Msg) {
 double MagneticCOG;
  if (pBD==0) return;
  
  if (NMEA0183ParseVTG_nc(NMEA0183Msg,pBD->COG,MagneticCOG,pBD->SOG)) {
      pBD->Variation=pBD->COG-MagneticCOG; // Save variation for Magnetic heading
    if (pNMEA2000!=0) { 
      tN2kMsg N2kMsg;
      SetN2kCOGSOGRapid(N2kMsg,1,N2khr_true,pBD->COG,pBD->SOG);
      pNMEA2000->SendMsg(N2kMsg);
//      SetN2kBoatSpeed(N2kMsg,1,SOG);
//      NMEA2000.SendMsg(N2kMsg);
    }
    if (NMEA0183HandlersDebugStream!=0) {
      NMEA0183HandlersDebugStream->print("True heading="); NMEA0183HandlersDebugStream->println(pBD->TrueHeading);
    }
  } else if (NMEA0183HandlersDebugStream!=0) { NMEA0183HandlersDebugStream->println("Failed to parse VTG"); }
}


