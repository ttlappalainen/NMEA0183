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

tNMEA0183::tNMEA0183() {
    MsgWritePos=0;
    MsgCheckSumStartPos=-1;
    MsgStarted=false;
    MsgHandler=0;
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
        MsgStarted=true;
        MsgWritePos=0;
        MsgBuf[MsgWritePos]=NewByte;
        MsgWritePos++;
      } else if (MsgStarted) {
        MsgBuf[MsgWritePos]=NewByte;
        if (NewByte=='*') MsgCheckSumStartPos=MsgWritePos;
        MsgWritePos++;
        if (MsgCheckSumStartPos>0 and MsgCheckSumStartPos+3==MsgWritePos) { // We have full checksum and so full message
            MsgBuf[MsgWritePos]=0; // add null termination
//        Serial.println(MsgBuf);
          if (NMEA0183Msg.SetMessage(MsgBuf)) {
            NMEA0183Msg.SourceID=SourceID;
            result=true;
          }
          MsgStarted=false;
          MsgWritePos=0;
          MsgCheckSumStartPos=-1;  
        }
        if (MsgWritePos>=MAX_NMEA0183_MSG_BUF_LEN) { // Too may chars in message. Start from beginning
          MsgStarted=false;
          MsgWritePos=0;
          MsgCheckSumStartPos=-1;  
        }
      }
  }
  
  return result;
}

