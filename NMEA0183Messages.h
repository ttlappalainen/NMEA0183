/*
NMEA0183Messages.h

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

#ifndef _tNMEA0183_MESSAGES_H_
#define _tNMEA0183_MESSAGES_H_
#include <stdio.h>
#include <time.h>
#include <NMEA0183Msg.h>

#ifndef Arduino
typedef uint8_t byte;
#endif

#define NMEA0183_MAX_WP_NAME_LENGTH 20
//The $GPRTE,2,1,c,0, ... *69 part takes up 18 characters. Need additional character for the null terminator of the last string.
#define NMEA0183_RTE_WPLENGTH MAX_NMEA0183_MSG_LEN-18+1

//*****************************************************************************
//Representation of a single NMEA0183 RTE message.
//In case of tRTE.nrOfsentences > 1 a sequence of RTE messages is needed for full data which are correlated by tRTE.routeID and ordered by tRTE.currSentence.
//The tRTE contains a array of waypoints in the range tRTE[0] ... tRTE[tRTE.nrOfwp - 1]
struct tRTE {

	//total number of sentences needed for full data
	unsigned int nrOfsentences;
	unsigned int currSentence;
	//'c' = complete route list, 'w' = first listed waypoint is start of current leg
	char type;
	unsigned int routeID;
	//Internal list of null terminator separated waypoints. Use the [] operator to read.
	char _wp[NMEA0183_RTE_WPLENGTH];
	unsigned int nrOfwp;

	const char* operator [](unsigned int i) const {
		if ( i > nrOfwp ) {
			return 0; //Index out of bounds.
		} else if (i == 0) {
			return _wp;
		} else {
			byte j = 0;
			for (byte k=0; k < NMEA0183_RTE_WPLENGTH; k++) {
				if (_wp[k] == 0 && ++j == i) {
					return _wp + k + 1;
				}
			}
			return 0;
		}
	}
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
  char originID[NMEA0183_MAX_WP_NAME_LENGTH];
  char destID[NMEA0183_MAX_WP_NAME_LENGTH];
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

struct tWDC {
	double dtw;
  char destID[NMEA0183_MAX_WP_NAME_LENGTH];
};

struct tWDR {
	double dtw;
  char destID[NMEA0183_MAX_WP_NAME_LENGTH];
};

struct tWPL {

	//total number of sentences needed for full data
	double latitude;
	double longitude;
  char name[NMEA0183_MAX_WP_NAME_LENGTH];
};

struct tBOD {
	//True bearing from origin to dest
	double trueBearing;
	//Magnetic bearing from origin to dest
	double magBearing;
	//Origin waypoint ID
  char originID[NMEA0183_MAX_WP_NAME_LENGTH];
	//Destination waypoint ID
  char destID[NMEA0183_MAX_WP_NAME_LENGTH];
};

struct tZTG {
	double GPSTime;
	double ETA;
  char destID[NMEA0183_MAX_WP_NAME_LENGTH];
};

enum tNMEA0183WindReference {
                            NMEA0183Wind_True=0,
                            // Apparent Wind (relative to the vessel centerline)
                            NMEA0183Wind_Apparent=1
                          };

const char NMEA0183RDO_NoDirectionOrder='\0';
const char NMEA0183RDO_MoveToStarboard='R';
const char NMEA0183RDO_MoveToPort='L';

void NMEA0183AddChecksum(char* msg);

//*****************************************************************************
bool NMEA0183SetDBK(tNMEA0183Msg &NMEA0183Msg, double Depth, const char *Src="II");

//*****************************************************************************
bool NMEA0183SetDBS(tNMEA0183Msg &NMEA0183Msg, double Depth, const char *Src="II");

//*****************************************************************************
bool NMEA0183SetDBT(tNMEA0183Msg &NMEA0183Msg, double Depth, const char *Src="II");

//*****************************************************************************
// Set message to DBK/DBS/DBT automatically according to Offset
bool NMEA0183SetDBx(tNMEA0183Msg &NMEA0183Msg, double DepthBelowTransducer, double Offset, const char *Src="II");


//*****************************************************************************
bool NMEA0183SetDPT(tNMEA0183Msg &NMEA0183Msg, double DepthBelowTransducer, double Offset, const char *Src="II");


//*****************************************************************************
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

//*****************************************************************************
bool NMEA0183SetGGA(tNMEA0183Msg &NMEA0183Msg, double GPSTime, double Latitude, double Longitude,
          	uint32_t GPSQualityIndicator, uint32_t SatelliteCount, double HDOP, double Altitude, double GeoidalSeparation,
	          double DGPSAge, uint32_t DGPSReferenceStationID, const char *Src="GP");


//*****************************************************************************
bool NMEA0183ParseGLL_nc(const tNMEA0183Msg &NMEA0183Msg, tGLL &gll);

inline bool NMEA0183ParseGLL(const tNMEA0183Msg &NMEA0183Msg, tGLL &gll) {
  return (NMEA0183Msg.IsMessageCode("GLL")
            ?NMEA0183ParseGLL_nc(NMEA0183Msg,gll)
            :false);
}

//*****************************************************************************
bool NMEA0183SetGLL(tNMEA0183Msg &NMEA0183Msg, double GPSTime, double Latitude, double Longitude, const char *Src="GP");

//*****************************************************************************
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
// TrueHeading,MagneticHeading will be returned be in radians
// SOW will be returned in m/s
bool NMEA0183ParseVHW_nc(const tNMEA0183Msg &NMEA0183Msg, double &TrueHeading, double &MagneticHeading, double &SOW);

inline bool NMEA0183ParseVHW(const tNMEA0183Msg &NMEA0183Msg, double &TrueHeading, double &MagneticHeading, double &SOW) {
  return (NMEA0183Msg.IsMessageCode("VHW")
            ?NMEA0183ParseVHW_nc(NMEA0183Msg,TrueHeading,MagneticHeading,SOW)
            :false);
}

bool NMEA0183SetVHW(tNMEA0183Msg &NMEA0183Msg, double TrueHeading, double MagneticHeading, double SOW, const char *Src="VW");

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

//*****************************************************************************
//Parse a single NMEA0183 RTE message into a tRTE struct.
//Depending on the size of the route a GPS will send a single RTE message or send multiple RTE messages via NMEA0183.
//This method only handles a single RTE message. Handling a sequence of RTE messages is outside of the scope of this lib.
//This should be handled in the calling lib. An example lib which handles a sequence of RTE messages can be found here: https://github.com/tonswieb/NMEAGateway
//$GPRTE,2,1,c,0,W3IWI,DRIVWY,32CEDR,32-29,32BKLD,32-I95,32-US1,BW-32,BW-198*69
bool NMEA0183ParseRTE_nc(const tNMEA0183Msg &NMEA0183Msg, tRTE &rte);

inline bool NMEA0183ParseRTE(const tNMEA0183Msg &NMEA0183Msg, tRTE &rte) {
	return (NMEA0183Msg.IsMessageCode("RTE") ?
					NMEA0183ParseRTE_nc(NMEA0183Msg,rte) : false);
}

//*****************************************************************************
//$GPWPL,5208.700,N,00438.600,E,MOLENB*4D
bool NMEA0183ParseWPL_nc(const tNMEA0183Msg &NMEA0183Msg, tWPL &wpl);

inline bool NMEA0183ParseWPL(const tNMEA0183Msg &NMEA0183Msg, tWPL &wpl) {
	return (NMEA0183Msg.IsMessageCode("WPL") ?
					NMEA0183ParseWPL_nc(NMEA0183Msg,wpl) : false);
}

//*****************************************************************************
bool NMEA0183ParseBOD_nc(const tNMEA0183Msg &NMEA0183Msg, tBOD &bod);

inline bool NMEA0183ParseBOD(const tNMEA0183Msg &NMEA0183Msg, tBOD &bod) {
	return (NMEA0183Msg.IsMessageCode("BOD") ?
					NMEA0183ParseBOD_nc(NMEA0183Msg,bod) : false);
}

//*****************************************************************************
// MWV - Wind Speed and Angle
bool NMEA0183ParseMWV_nc(const tNMEA0183Msg &NMEA0183Msg,double &WindAngle, tNMEA0183WindReference &Reference, double &WindSpeed);

inline bool NMEA0183ParseMWV(const tNMEA0183Msg &NMEA0183Msg,double &WindAngle, tNMEA0183WindReference &Reference, double &WindSpeed) {
  return (NMEA0183Msg.IsMessageCode("MWV")
            ?NMEA0183ParseMWV_nc(NMEA0183Msg,WindAngle,Reference,WindSpeed)
            :false);
}

bool NMEA0183SetMWV(tNMEA0183Msg &NMEA0183Msg, double WindAngle, tNMEA0183WindReference Reference, double WindSpeed, const char *Src="II");

//*****************************************************************************
// AAM - Waypoint Arrival Alarm
bool NMEA0183ParseAAM_nc(const tNMEA0183Msg &NMEA0183Msg, char &ArrivalCircleEntered, char &PerpendicularCrossed, double &arrivalRadius, char destID[NMEA0183_MAX_WP_NAME_LENGTH]);

inline bool NMEA0183ParseAAM(const tNMEA0183Msg &NMEA0183Msg, char &ArrivalCircleEntered, char &PerpendicularCrossed, double &arrivalRadius, char destID[NMEA0183_MAX_WP_NAME_LENGTH]) {
  return (NMEA0183Msg.IsMessageCode("AAM")
            ?NMEA0183ParseAAM_nc(NMEA0183Msg, ArrivalCircleEntered, PerpendicularCrossed, arrivalRadius, destID)
            :false);
}

bool NMEA0183SetAAM(tNMEA0183Msg &NMEA0183Msg, char ArrivalCircleEntered, char PerpendicularCrossed, double arrivalRadius, char destID[NMEA0183_MAX_WP_NAME_LENGTH], const char *Src="GP");

//*****************************************************************************
// APA - Autopilot Sentence "A"             Deprecated
bool NMEA0183ParseAPA_nc(const tNMEA0183Msg &NMEA0183Msg, double &xte, char &RudderDirectionOrder, char &ArrivalCircleEntered, char &PerpendicularCrossed, double &bearingFromOrigin, char &bearingFromOriginFlag, char destID[NMEA0183_MAX_WP_NAME_LENGTH]);

inline bool NMEA0183ParseAPA(const tNMEA0183Msg &NMEA0183Msg, double &xte, char &RudderDirectionOrder, char &ArrivalCircleEntered, char &PerpendicularCrossed, double &bearingFromOrigin, char &bearingFromOriginFlag, char destID[NMEA0183_MAX_WP_NAME_LENGTH]) {
  return (NMEA0183Msg.IsMessageCode("APA")
            ?NMEA0183ParseAPA_nc(NMEA0183Msg, xte, RudderDirectionOrder, ArrivalCircleEntered, PerpendicularCrossed, bearingFromOrigin, bearingFromOriginFlag, destID)
            :false);
}

bool NMEA0183SetAPA(tNMEA0183Msg &NMEA0183Msg, double xte, char RudderDirectionOrder, char ArrivalCircleEntered, char PerpendicularCrossed, double bearingFromOrigin, char bearingFromOriginFlag, char destID[NMEA0183_MAX_WP_NAME_LENGTH], const char *Src="GP");

//*****************************************************************************
// APB - Autopilot Sentence "B"             Deprecated
bool NMEA0183ParseAPB_nc(const tNMEA0183Msg &NMEA0183Msg, double &xte, char &RudderDirectionOrder, char &ArrivalCircleEntered, char &PerpendicularCrossed, double &bearingFromOrigin, char &bearingFromOriginFlag, char destID[NMEA0183_MAX_WP_NAME_LENGTH], double &bearingToDestination, char &bearingToDestinationFlag, double &headingToDestination, char &headingToDestinationFlag);

inline bool NMEA0183ParseAPB(const tNMEA0183Msg &NMEA0183Msg, double &xte, char &RudderDirectionOrder, char &ArrivalCircleEntered, char &PerpendicularCrossed, double &bearingFromOrigin, char &bearingFromOriginFlag, char destID[NMEA0183_MAX_WP_NAME_LENGTH], double &bearingToDestination, char &bearingToDestinationFlag, double &headingToDestination, char &headingToDestinationFlag) {
  return (NMEA0183Msg.IsMessageCode("APB")
            ?NMEA0183ParseAPB_nc(NMEA0183Msg, xte, RudderDirectionOrder, ArrivalCircleEntered, PerpendicularCrossed, bearingFromOrigin, bearingFromOriginFlag, destID, bearingToDestination, bearingToDestinationFlag, headingToDestination, headingToDestinationFlag)
            :false);
}

bool NMEA0183SetAPB(tNMEA0183Msg &NMEA0183Msg, double xte, char RudderDirectionOrder, char ArrivalCircleEntered, char PerpendicularCrossed, double bearingFromOrigin, char bearingFromOriginFlag, char destID[NMEA0183_MAX_WP_NAME_LENGTH], double bearingToDestination, char bearingToDestinationFlag, double headingToDestination, char headingToDestinationFlag, const char *Src="GP");

//*****************************************************************************
// BWC - Bearing & Distance to Waypoint using a Great Circle route.
bool NMEA0183ParseBWC_nc(const tNMEA0183Msg &NMEA0183Msg, double &GPSTime, double &latitude, double &longitude, double &trueBearing, double &magBearing, double &dtw, char destID[NMEA0183_MAX_WP_NAME_LENGTH]);

inline bool NMEA0183ParseBWC(const tNMEA0183Msg &NMEA0183Msg, double &GPSTime, double &latitude, double &longitude, double &trueBearing, double &magBearing, double &dtw, char destID[NMEA0183_MAX_WP_NAME_LENGTH]) {
  return (NMEA0183Msg.IsMessageCode("BWC")
            ?NMEA0183ParseBWC_nc(NMEA0183Msg, GPSTime, latitude, longitude, trueBearing, magBearing, dtw, destID)
            :false);
}

bool NMEA0183SetBWC(tNMEA0183Msg &NMEA0183Msg, double GPSTime, double latitude, double longitude, double trueBearing, double magBearing, double dtw, char destID[NMEA0183_MAX_WP_NAME_LENGTH], const char *Src="GP");

//*****************************************************************************
// BWR - Bearing & Distance to Waypoint using a Rhumb Line route.
bool NMEA0183ParseBWR_nc(const tNMEA0183Msg &NMEA0183Msg, double &GPSTime, double &latitude, double &longitude, double &trueBearing, double &magBearing, double &dtw, char destID[NMEA0183_MAX_WP_NAME_LENGTH]);

inline bool NMEA0183ParseBWR(const tNMEA0183Msg &NMEA0183Msg, double &GPSTime, double &latitude, double &longitude, double &trueBearing, double &magBearing, double &dtw, char destID[NMEA0183_MAX_WP_NAME_LENGTH]) {
  return (NMEA0183Msg.IsMessageCode("BWR")
            ?NMEA0183ParseBWR_nc(NMEA0183Msg, GPSTime, latitude, longitude, trueBearing, magBearing, dtw, destID)
            :false);
}

bool NMEA0183SetBWR(tNMEA0183Msg &NMEA0183Msg, double GPSTime, double latitude, double longitude, double trueBearing, double magBearing, double dtw, char destID[NMEA0183_MAX_WP_NAME_LENGTH], const char *Src="GP");

//*****************************************************************************
bool NMEA0183ParseDBK_nc(const tNMEA0183Msg &NMEA0183Msg, double &Depth);

inline bool NMEA0183ParseDBK(const tNMEA0183Msg &NMEA0183Msg, double &Depth) {
  return (NMEA0183Msg.IsMessageCode("DBK")
            ?NMEA0183ParseDBK_nc(NMEA0183Msg,Depth)
            :false);
}

//*****************************************************************************
bool NMEA0183ParseDBT_nc(const tNMEA0183Msg &NMEA0183Msg, double &Depth);

inline bool NMEA0183ParseDBT(const tNMEA0183Msg &NMEA0183Msg, double &Depth) {
  return (NMEA0183Msg.IsMessageCode("DBT")
            ?NMEA0183ParseDBT_nc(NMEA0183Msg,Depth)
            :false);
}

//*****************************************************************************
// Heading will be returned be in radians
bool NMEA0183ParseHDG_nc(const tNMEA0183Msg &NMEA0183Msg, double &MagneticHeading, double &Deviation, double &Variation);

inline bool NMEA0183ParseHDG(const tNMEA0183Msg &NMEA0183Msg, double &MagneticHeading, double &Deviation, double &Variation) {
  return (NMEA0183Msg.IsMessageCode("HDG")
            ?NMEA0183ParseHDG_nc(NMEA0183Msg,MagneticHeading,Deviation,Variation)
            :false);
}

//*****************************************************************************
// HSC - Heading Steering Command
bool NMEA0183ParseHSC_nc(const tNMEA0183Msg &NMEA0183Msg, double &TrueHeading, double &MagneticHeading);

inline bool NMEA0183ParseHSC(const tNMEA0183Msg &NMEA0183Msg, double &TrueHeading, double &MagneticHeading) {
  return (NMEA0183Msg.IsMessageCode("HSC")
            ?NMEA0183ParseHSC_nc(NMEA0183Msg, TrueHeading, MagneticHeading)
            :false);
}

bool NMEA0183SetHSC(tNMEA0183Msg &NMEA0183Msg, double TrueHeading, double MagneticHeading, const char *Src="GP");

//*****************************************************************************
// MTW - Water Temperature
bool NMEA0183SetMTW(tNMEA0183Msg &NMEA0183Msg, double WaterTemperature, const char *Src="II");

bool NMEA0183ParseMTW_nc(const tNMEA0183Msg &NMEA0183Msg, double &WaterTemperature);

inline bool NMEA0183ParseMTW(const tNMEA0183Msg &NMEA0183Msg, double &WaterTemperature) {
  return (NMEA0183Msg.IsMessageCode("MTW")
            ?NMEA0183ParseMTW_nc(NMEA0183Msg,WaterTemperature)
            :false);
}

//*****************************************************************************
bool NMEA0183SetRMB(tNMEA0183Msg &NMEA0183Msg, tRMB RMB, const char *Src="GP");

//*****************************************************************************
// RSA - Rudder Sensor Angle
bool NMEA0183ParseRSA_nc(const tNMEA0183Msg &NMEA0183Msg, double &RudderSensor1, double &RudderSensor2);

inline bool NMEA0183ParseRSA(const tNMEA0183Msg &NMEA0183Msg, double &RudderSensor1, double &RudderSensor2) {
  return (NMEA0183Msg.IsMessageCode("RSA")
            ?NMEA0183ParseRSA_nc(NMEA0183Msg,RudderSensor1,RudderSensor2)
            :false);
}

bool NMEA0183SetRSA(tNMEA0183Msg &NMEA0183Msg, double RudderSensor1, double RudderSensor2, const char *Src="II");

//*****************************************************************************
// VLW - Water LOG
bool NMEA0183SetVLW(tNMEA0183Msg &NMEA0183Msg, double LOGtotal, double LOGtrip, const char *Src="II");

bool NMEA0183ParseVLW_nc(const tNMEA0183Msg &NMEA0183Msg, double &LOGtotal, double &LOGtrip);

inline bool NMEA0183ParseVLW(const tNMEA0183Msg &NMEA0183Msg, double &LOGtotal, double &LOGtrip) {
  return (NMEA0183Msg.IsMessageCode("VLW")
            ?NMEA0183ParseVLW_nc(NMEA0183Msg,LOGtotal,LOGtrip)
            :false);
}

//*****************************************************************************
// VWR - Apparent Wind Speed and Angle    Deprecated
bool NMEA0183ParseVWR_nc(const tNMEA0183Msg &NMEA0183Msg, double &WindAngle, double &WindSpeed);

inline bool NMEA0183ParseVWR(const tNMEA0183Msg &NMEA0183Msg, double &WindAngle, double &WindSpeed) {
  return (NMEA0183Msg.IsMessageCode("VWR")
            ?NMEA0183ParseVWR_nc(NMEA0183Msg,WindAngle,WindSpeed)
            :false);
}

bool NMEA0183SetVWR(tNMEA0183Msg &NMEA0183Msg, double WindAngle, double WindSpeed, const char *Src="II");

//*****************************************************************************
// WDC - Distance to Waypoint - Great Circle     Deprecated
bool NMEA0183ParseWDC_nc(const tNMEA0183Msg &NMEA0183Msg, double &dtw, char destID[NMEA0183_MAX_WP_NAME_LENGTH]);

inline bool NMEA0183ParseWDC(const tNMEA0183Msg &NMEA0183Msg, double &dtw, char destID[NMEA0183_MAX_WP_NAME_LENGTH]) {
  return (NMEA0183Msg.IsMessageCode("WDC")
            ?NMEA0183ParseWDC_nc(NMEA0183Msg, dtw, destID)
            :false);
}

inline bool NMEA0183ParseWDC(const tNMEA0183Msg &NMEA0183Msg, tWDC &wdc) {

	return NMEA0183ParseWDC(NMEA0183Msg, wdc.dtw, wdc.destID);
}

bool NMEA0183SetWDC(tNMEA0183Msg &NMEA0183Msg, double dtw, char destID[NMEA0183_MAX_WP_NAME_LENGTH], const char *Src="GP");

//*****************************************************************************
// WDR - Distance to Waypoint - Rhumb Line       Deprecated
bool NMEA0183ParseWDR_nc(const tNMEA0183Msg &NMEA0183Msg, double &dtw, char destID[NMEA0183_MAX_WP_NAME_LENGTH]);

inline bool NMEA0183ParseWDR(const tNMEA0183Msg &NMEA0183Msg, double &dtw, char destID[NMEA0183_MAX_WP_NAME_LENGTH]) {
  return (NMEA0183Msg.IsMessageCode("WDR")
            ?NMEA0183ParseWDR_nc(NMEA0183Msg, dtw, destID)
            :false);
}

inline bool NMEA0183ParseWDR(const tNMEA0183Msg &NMEA0183Msg, tWDR &wdr) {

	return NMEA0183ParseWDR(NMEA0183Msg, wdr.dtw, wdr.destID);
}

bool NMEA0183SetWDR(tNMEA0183Msg &NMEA0183Msg, double dtw, char destID[NMEA0183_MAX_WP_NAME_LENGTH], const char *Src="GP");

//*****************************************************************************
bool NMEA0183SetWPL(tNMEA0183Msg &NMEA0183Msg, double latitude, double longitude, char name[NMEA0183_MAX_WP_NAME_LENGTH], const char *Src="GP");

//*****************************************************************************
// XTE - Cross-Track Error, Measured
bool NMEA0183ParseXTE_nc(const tNMEA0183Msg &NMEA0183Msg, double &xte, char &RudderDirectionOrder);

inline bool NMEA0183ParseXTE(const tNMEA0183Msg &NMEA0183Msg, double &xte, char &RudderDirectionOrder) {
  return (NMEA0183Msg.IsMessageCode("XTE")
            ?NMEA0183ParseXTE_nc(NMEA0183Msg, xte, RudderDirectionOrder)
            :false);
}

bool NMEA0183SetXTE(tNMEA0183Msg &NMEA0183Msg, double xte, char RudderDirectionOrder, const char *Src="GP");

//*****************************************************************************
// ZTG - UTC & Time to Destination Waypoint
bool NMEA0183ParseZTG_nc(const tNMEA0183Msg &NMEA0183Msg, double &GPSTime, double &ETA, char destID[NMEA0183_MAX_WP_NAME_LENGTH]);

inline bool NMEA0183ParseZTG(const tNMEA0183Msg &NMEA0183Msg, double &GPSTime, double &ETA, char destID[NMEA0183_MAX_WP_NAME_LENGTH]) {
  return (NMEA0183Msg.IsMessageCode("ZTG")
            ?NMEA0183ParseZTG_nc(NMEA0183Msg, GPSTime, ETA, destID)
            :false);
}

inline bool NMEA0183ParseZTG(const tNMEA0183Msg &NMEA0183Msg, tZTG &ztg) {

	return NMEA0183ParseZTG(NMEA0183Msg, ztg.GPSTime, ztg.ETA, ztg.destID);
}

bool NMEA0183SetZTG(tNMEA0183Msg &NMEA0183Msg, double GPSTime, double ETA, char destID[NMEA0183_MAX_WP_NAME_LENGTH], const char *Src="GP");

#endif
