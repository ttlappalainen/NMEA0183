/* 
NMEA0183.cpp

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

#include <NMEA0183.h>

tNMEA0183::tNMEA0183()
: MsgCheckSumStartPos(-1), MsgInStarted(false), MsgInPos(0), 
  MsgHandler(0), SourceId(0),port(0) {
}

//*****************************************************************************
void tNMEA0183::Begin(HardwareSerial *_port, uint8_t _SourceID, unsigned long _baud) {
  SourceID=_SourceID;
  port=_port;
  port->begin(_baud);
}

//*****************************************************************************
void tNMEA0183::ParseMessages() {
  tNMEA0183Msg NMEA0183Msg;
  
    while (GetMessage(NMEA0183Msg)) {
      if (MsgHandler!=0) MsgHandler(NMEA0183Msg);
    }
}

//*****************************************************************************
bool tNMEA0183::GetMessage(tNMEA0183Msg &NMEA0183Msg) {
  bool result=false;

  while (port->available() > 0 && !result) {
    int NewByte=port->read();
//        Serial.println((char)NewByte);
      if (NewByte=='$' || NewByte=='!') { // Message start
        MsgInStarted=true;
        MsgInPos=0;
        MsgInBuf[MsgInPos]=NewByte;
        MsgInPos++;
      } else if (MsgInStarted) {
        MsgInBuf[MsgInPos]=NewByte;
        if (NewByte=='*') MsgCheckSumStartPos=MsgInPos;
        MsgInPos++;
        if (MsgCheckSumStartPos>0 and MsgCheckSumStartPos+3==MsgInPos) { // We have full checksum and so full message
            MsgInBuf[MsgInPos]=0; // add null termination
          if (NMEA0183Msg.SetMessage(MsgInBuf)) {
//        Serial.println(MsgBuf);
            NMEA0183Msg.SourceID=SourceID;
            result=true;
          }
          MsgInStarted=false;
          MsgInPos=0;
          MsgCheckSumStartPos=-1;  
        }
        if (MsgInPos>=MAX_NMEA0183_MSG_BUF_LEN) { // Too may chars in message. Start from beginning
          MsgInStarted=false;
          MsgInPos=0;
          MsgCheckSumStartPos=-1;  
        }
      }
  }
  
  return result;
}

