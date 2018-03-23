/* 
N2kWindDataHandler.cpp

Copyright (c) 2018 Timo Lappalainen, Kave Oy, www.kave.fi

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

NMEA2000 wind data handler class.

*/

#include <NMEA0183.h>
#include <N2kMessages.h>
#include <NMEA0183Messages.h>
#include "N2kWindDataHandler.h"

const double radToDeg=180.0/M_PI;

extern tNMEA0183 NMEA0183_Out;

// *****************************************************************************
tN2kWindDataHandler::tN2kWindDataHandler(tNMEA2000 *_pNMEA2000) :
   tNMEA2000::tMsgHandler(130306L,_pNMEA2000) {
}

// *****************************************************************************
void tN2kWindDataHandler::HandleMsg(const tN2kMsg &N2kMsg) {
  unsigned char SID;
  double WindSpeed;
  double WindAngle;
  tN2kWindReference WindReference;
  tNMEA0183WindReference NMEA0183Reference=NMEA0183Wind_True;
  
  if ( ParseN2kWindSpeed(N2kMsg, SID, WindSpeed, WindAngle, WindReference) ) {
    tNMEA0183Msg NMEA0183Msg;
    if ( WindReference==N2kWind_Apparent ) NMEA0183Reference=NMEA0183Wind_Apparent;
    NMEA0183SetMWV(NMEA0183Msg, WindAngle*radToDeg, NMEA0183Reference , WindSpeed);
    
    NMEA0183_Out.SendMessage(NMEA0183Msg);
  }

}
