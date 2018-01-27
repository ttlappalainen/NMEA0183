/* 
NMEA0183Messages.h

2015-2017 Copyright (c) Kave Oy, www.kave.fi  All right reserved.

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

#define NMEA0183MaxWpNameLength 20

//A NMEA sentence can at max be 80 characters long including the $GPRTE,2,1,c,0 part, the checksum and carriage return.
//Which leaves 62 characters for WP's, in case of single character waypoints a RTE message could at max host 31 waypoints.
#define NMEA0183_MAX_WP_IN_RTE 31

//$GPRTE,2,1,c,0,W3IWI,DRIVWY,32CEDR,32-29,32BKLD,32-I95,32-US1,BW-32,BW-198*69
struct tRTE {

	//total number of sentences needed for full data
	unsigned int nrOfsentences;
	unsigned int currSentence;
	//'c' = complete route list, 'w' = first listed waypoint is start of current leg
	char type;
	unsigned int routeID;
	char wp[NMEA0183_MAX_WP_IN_RTE][NMEA0183MaxWpNameLength + 1];
	unsigned int nrOfwp;
};

struct tGGA {
	
	double GPSTime;
	double latitude;
	double longitude;
	int GPSQualityIndicator;
	int satelliteCount;
	double HDOP;
	double altitude;
	double geoidalSeparation;
	double DGPSAge;
	int DGPSReferenceStationID;
};

struct tGLL {
	
	double GPSTime;
	double latitude;
	double longitude;
	//'A' = OK, 'V' = Void (warning)
	char status;
};


struct tRMB {

	//'A' = OK, 'V' = Void (warning)
	char status;
	double xte;
	double latitude;
	double longitude;
	double dtw;
	double btw;
	double vmg;
	//'A' = arrived, 'V' = not arrived
	char arrivalAlarm;
  char originID[NMEA0183MaxWpNameLength];
  char destID[NMEA0183MaxWpNameLength];
};

struct tRMC {
	
	//'A' = OK, 'V' = Void (warning)
	char status;
	double GPSTime; // Secs since midnight
	double latitude;
	double longitude;
	double trueCOG;
	double SOG;
	unsigned long daysSince1970;
	double variation;
};


struct tWPL {

	//total number of sentences needed for full data
	double latitude;
	double longitude;
  char name[NMEA0183MaxWpNameLength];
};

struct tBOD {
	//True bearing from origin to dest
	double trueBearing;
	//Magnetic bearing from origin to dest
	double magBearing;
	//Origin waypoint ID
  char originID[NMEA0183MaxWpNameLength];
	//Destination waypoint ID
  char destID[NMEA0183MaxWpNameLength];
};

void NMEA0183AddChecksum(char* msg);

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

inline bool NMEA0183ParseGGA(const tNMEA0183Msg &NMEA0183Msg, tGGA &gga) {
	
	return NMEA0183ParseGGA(NMEA0183Msg,gga.GPSTime,gga.latitude,gga.longitude,gga.GPSQualityIndicator,
										gga.satelliteCount,gga.HDOP,gga.altitude,gga.geoidalSeparation,gga.DGPSAge,gga.DGPSReferenceStationID);
}

bool NMEA0183ParseGLL_nc(const tNMEA0183Msg &NMEA0183Msg, tGLL &gll);

inline bool NMEA0183ParseGLL(const tNMEA0183Msg &NMEA0183Msg, tGLL &gll) {
  return (NMEA0183Msg.IsMessageCode("GLL")
            ?NMEA0183ParseGLL_nc(NMEA0183Msg,gll)
            :false);
}

bool NMEA0183ParseRMB_nc(const tNMEA0183Msg &NMEA0183Msg, tRMB &rmb);

inline bool NMEA0183ParseRMB(const tNMEA0183Msg &NMEA0183Msg, tRMB &rmb) {
    return (NMEA0183Msg.IsMessageCode("RMB") ?
        NMEA0183ParseRMB_nc(NMEA0183Msg, rmb) : false);
}

//*****************************************************************************
// RMC
bool NMEA0183ParseRMC_nc(const tNMEA0183Msg &NMEA0183Msg, double &GPSTime, double &Latitude, double &Longitude,
                      double &TrueCOG, double &SOG, unsigned long &DaysSince1970, double &Variation, time_t *DateTime=0);

inline bool NMEA0183ParseRMC(const tNMEA0183Msg &NMEA0183Msg, double &GPSTime, double &Latitude, double &Longitude,
                      double &TrueCOG, double &SOG, unsigned long &DaysSince1970, double &Variation, time_t *DateTime=0) {
  (void)DateTime;
  return (NMEA0183Msg.IsMessageCode("RMC")
            ?NMEA0183ParseRMC_nc(NMEA0183Msg, GPSTime, Latitude, Longitude, TrueCOG, SOG, DaysSince1970, Variation, DateTime)
            :false);
}

inline bool NMEA0183ParseRMC(const tNMEA0183Msg &NMEA0183Msg, tRMC &rmc, time_t *DateTime=0) {
	
	return NMEA0183ParseRMC(NMEA0183Msg, rmc.GPSTime, rmc.latitude, rmc.longitude, rmc.trueCOG, rmc.SOG, rmc.daysSince1970, rmc.variation, DateTime);
}
 
bool NMEA0183SetRMC(tNMEA0183Msg &NMEA0183Msg, double GPSTime, double Latitude, double Longitude,
                      double TrueCOG, double SOG, unsigned long DaysSince1970, double Variation, const char *Src="GP");
 
//*****************************************************************************
// COG will be returned be in radians
// SOG will be returned in m/s
bool NMEA0183ParseVTG_nc(const tNMEA0183Msg &NMEA0183Msg, double &TrueCOG, double &MagneticCOG, double &SOG);

inline bool NMEA0183ParseVTG(const tNMEA0183Msg &NMEA0183Msg, double &TrueCOG, double &MagneticCOG, double &SOG) {
  return (NMEA0183Msg.IsMessageCode("VTG")
            ?NMEA0183ParseVTG_nc(NMEA0183Msg,TrueCOG,MagneticCOG,SOG)
            :false);
}

bool NMEA0183SetVTG(tNMEA0183Msg &NMEA0183Msg, double TrueCOG, double MagneticCOG, double SOG, const char *Src="GP");

// This is obsolet. Use NMEA0183SetVTG
bool NMEA0183BuildVTG(char* msg, const char Src[], double TrueCOG, double MagneticCOG, double SOG);

//*****************************************************************************
// Rate of turn will be returned be in radians
bool NMEA0183ParseROT_nc(const tNMEA0183Msg &NMEA0183Msg,double &RateOfTurn); 

inline bool NMEA0183ParseROT(const tNMEA0183Msg &NMEA0183Msg, double &RateOfTurn) {
  return (NMEA0183Msg.IsMessageCode("ROT")
            ?NMEA0183ParseROT_nc(NMEA0183Msg,RateOfTurn)
            :false);
}

bool NMEA0183SetROT(tNMEA0183Msg &NMEA0183Msg, double RateOfTurn, const char *Src="GP");

//*****************************************************************************
// Heading will be returned be in radians
bool NMEA0183ParseHDT_nc(const tNMEA0183Msg &NMEA0183Msg,double &TrueHeading); 

inline bool NMEA0183ParseHDT(const tNMEA0183Msg &NMEA0183Msg, double &TrueHeading) {
  return (NMEA0183Msg.IsMessageCode("HDT")
            ?NMEA0183ParseHDT_nc(NMEA0183Msg,TrueHeading)
            :false);
}

bool NMEA0183SetHDT(tNMEA0183Msg &NMEA0183Msg, double Heading, const char *Src="GP");

//*****************************************************************************
// Heading will be returned be in radians
bool NMEA0183ParseHDM_nc(const tNMEA0183Msg &NMEA0183Msg,double &MagneticHeading); 

inline bool NMEA0183ParseHDM(const tNMEA0183Msg &NMEA0183Msg, double &MagneticHeading) {
  return (NMEA0183Msg.IsMessageCode("HDM")
            ?NMEA0183ParseHDT_nc(NMEA0183Msg,MagneticHeading)
            :false);
}

bool NMEA0183SetHDM(tNMEA0183Msg &NMEA0183Msg, double Heading, const char *Src="GP");

//*****************************************************************************
bool NMEA0183SetHDG(tNMEA0183Msg &NMEA0183Msg, double Heading, double Deviation, double Variation, const char *Src="GP");

//*****************************************************************************
// VDM is basically a bitstream
bool NMEA0183ParseVDM_nc(const tNMEA0183Msg &NMEA0183Msg,
			uint8_t &pkgCnt, uint8_t &pkgNmb,
			unsigned int &seqMessageId, char &channel,
			unsigned int &length, char *bitstream,
			unsigned int &fillBits);


inline bool NMEA0183ParseVDM(const tNMEA0183Msg &NMEA0183Msg, uint8_t &pkgCnt, uint8_t &pkgNmb,
						unsigned int &seqMessageId, char &channel,
						unsigned int &length, char* bitstream, unsigned int &fillBits) {
  return (NMEA0183Msg.IsMessageCode("VDM") ?
		NMEA0183ParseVDM_nc(NMEA0183Msg, pkgCnt, pkgNmb, seqMessageId, channel, length, bitstream, fillBits) : false);
}

//$GPRTE,2,1,c,0,W3IWI,DRIVWY,32CEDR,32-29,32BKLD,32-I95,32-US1,BW-32,BW-198*69
bool NMEA0183ParseRTE_nc(const tNMEA0183Msg &NMEA0183Msg, tRTE &rte);

inline bool NMEA0183ParseRTE(const tNMEA0183Msg &NMEA0183Msg, tRTE &rte) {
	return (NMEA0183Msg.IsMessageCode("RTE") ?
					NMEA0183ParseRTE_nc(NMEA0183Msg,rte) : false);
}

//$GPWPL,5208.700,N,00438.600,E,MOLENB*4D
bool NMEA0183ParseWPL_nc(const tNMEA0183Msg &NMEA0183Msg, tWPL &wpl);

inline bool NMEA0183ParseWPL(const tNMEA0183Msg &NMEA0183Msg, tWPL &wpl) {
	return (NMEA0183Msg.IsMessageCode("WPL") ?
					NMEA0183ParseWPL_nc(NMEA0183Msg,wpl) : false);
}

bool NMEA0183ParseBOD_nc(const tNMEA0183Msg &NMEA0183Msg, tBOD &bod);

inline bool NMEA0183ParseBOD(const tNMEA0183Msg &NMEA0183Msg, tBOD &bod) {
	return (NMEA0183Msg.IsMessageCode("BOD") ?
					NMEA0183ParseBOD_nc(NMEA0183Msg,bod) : false);
}

#endif
