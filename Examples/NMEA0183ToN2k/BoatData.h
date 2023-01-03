#ifndef _BoatData_H_
#define _BoatData_H_

struct tBoatData {
  unsigned long DaysSince1970;   // Days since 1970-01-01
  
  double TrueHeading,SOG,COG,Variation,
         GPSTime,// Secs since midnight,
         Latitude, Longitude, Altitude, HDOP, GeoidalSeparation, DGPSAge,
         WaterTemperature, WaterDepth, Offset,
         WindDirectionT, WindDirectionM, WindSpeedK, WindSpeedM,
         WindAngle ;
  int GPSQualityIndicator, SatelliteCount, DGPSReferenceStationID;
  bool MOBActivated;
  char Status;

public:
  tBoatData() {
    TrueHeading=0;
    SOG=0;
    COG=0; 
    Variation=7.0;
    GPSTime=0;
    Latitude = 0;
    Longitude = 0;
    Altitude=0;
    HDOP=100000;
    DGPSAge=100000;
    WaterTemperature = 0;
    DaysSince1970=0; 
    MOBActivated=false; 
    SatelliteCount=0; 
    DGPSReferenceStationID=0;
  };
};

#endif // _BoatData_H_
