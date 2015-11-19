/* 
NMEA0183Messages.h

2015 Copyright (c) Kave Oy, www.kave.fi  All right reserved.

Author: Timo Lappalainen

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-
  1301  USA
*/

#ifndef _tNMEA0183_MESSAGES_H_
#define _tNMEA0183_MESSAGES_H_
#include <TimeLib.h>
#include <NMEA0183Msg.h>

bool NMEA0183ParseGGA_nc(const tNMEA0183Msg &NMEA0183Msg, double &GPSTime, double &Latitude, double &Longitude,
                      int &GPSQualityIndicator, int &SatelliteCount, double &HDOP, double &Altitude, double &GeoidalSeparation,
                      double &DGPSAge, int &DGPSReferenceStationID);

inline bool NMEA0183ParseGGA(const tNMEA0183Msg &NMEA0183Msg, double &GPSTime, double &Latitude, double &Longitude,
                      int &GPSQualityIndicator, int &SatelliteCount, double &HDOP, double &Altitude, double &GeoidalSeparation,
                      double &DGPSAge, int &DGPSReferenceStationID) {
  return (NMEA0183Msg.IsMessageCode("GGA")
            ?NMEA0183ParseGGA_nc(NMEA0183Msg,GPSTime,Latitude,Longitude,GPSQualityIndicator,SatelliteCount,HDOP,Altitude,GeoidalSeparation,DGPSAge,DGPSReferenceStationID)
            :false);
}

// RMC
bool NMEA0183ParseRMC_nc(const tNMEA0183Msg &NMEA0183Msg, double &GPSTime, double &Latitude, double &Longitude,
                      double &TrueCOG, double &SOG, unsigned long &DaysSince1970, double &Variation, time_t *DateTime=0);

inline bool NMEA0183ParseRMC(const tNMEA0183Msg &NMEA0183Msg, double &GPSTime, double &Latitude, double &Longitude,
                      double &TrueCOG, double &SOG, unsigned long &DaysSince1970, double &Variation, time_t *DateTime=0) {
  return (NMEA0183Msg.IsMessageCode("RMC")
            ?NMEA0183ParseRMC_nc(NMEA0183Msg, GPSTime, Latitude, Longitude, TrueCOG, SOG, DaysSince1970, Variation)
            :false);
}
                      
// COG will be returned be in radians
// SOG will be returned in m/s
bool NMEA0183ParseVTG_nc(const tNMEA0183Msg &NMEA0183Msg, double &TrueCOG, double &MagneticCOG, double &SOG);

inline bool NMEA0183ParseVTG(const tNMEA0183Msg &NMEA0183Msg, double &TrueCOG, double &MagneticCOG, double &SOG) {
  return (NMEA0183Msg.IsMessageCode("VTG")
            ?NMEA0183ParseVTG_nc(NMEA0183Msg,TrueCOG,MagneticCOG,SOG)
            :false);
}

// Heading will be returned be in radians
bool NMEA0183ParseHDT_nc(const tNMEA0183Msg &NMEA0183Msg,double &TrueHeading); 

inline bool NMEA0183ParseHDT(const tNMEA0183Msg &NMEA0183Msg, double &TrueHeading) {
  return (NMEA0183Msg.IsMessageCode("HDT")
            ?NMEA0183ParseHDT_nc(NMEA0183Msg,TrueHeading)
            :false);
}

#endif
