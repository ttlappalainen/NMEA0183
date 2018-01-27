/* 
NMEA0183.h

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

#ifndef _tNMEA0183_H_
#define _tNMEA0183_H_

#include <Arduino.h>  // Required by stream. Make similar as with NMEA2000 library
#include <stdint.h>
#include "NMEA0183Msg.h"

#define MAX_NMEA0183_MSG_BUF_LEN 81  // According to NMEA 3.01. Can not contain multi message as in AIS

class tNMEA0183
{
  protected:
    Stream *port;
    size_t MsgCheckSumStartPos;
    char MsgInBuf[MAX_NMEA0183_MSG_BUF_LEN];
    size_t MsgInPos;
    bool MsgInStarted;
    size_t MsgOutWritePos;
    size_t MsgOutReadPos;
    char *MsgOutBuf;
    size_t MsgOutBufSize;
    uint8_t SourceID;  // User defined ID for this message handler

    // Handler callback
    void (*MsgHandler)(const tNMEA0183Msg &NMEA0183Msg);

    size_t MsgOutBufFreeSize() {
      return (MsgOutReadPos<MsgOutWritePos?MsgOutBufSize-(MsgOutWritePos-MsgOutReadPos):MsgOutBufSize+MsgOutReadPos-MsgOutWritePos); 
    }
    bool IsOpen() const { return ( port!=0 && MsgOutBuf!=0 ); }
    bool SendBuf(const char *buf);
  public:
    tNMEA0183(Stream *stream=0, uint8_t _SourceID=0);
    void SetMessageStream(Stream *stream, uint8_t _SourceID=0);
    bool Open();
    // Begin is obsolete. Use Open(...)
    void Begin(HardwareSerial *_port, uint8_t _SourceID=0, unsigned long _baud=4800);
    // Set size for send message buffer. Call this before Open().
    void SetSendBufferSize(size_t size);
    void SetMsgHandler(void (*_MsgHandler)(const tNMEA0183Msg &NMEA0183Msg)) {MsgHandler=_MsgHandler;}
    // Call this in loop to read incoming messages or empty buffered sent messages.
    // For new messages message handler will be called.
    void ParseMessages();
    // You can also read incoming messages with GetMessage. Function
    // returns true, when there is valid message.
    bool GetMessage(tNMEA0183Msg &NMEA0183Msg);
    // Function will send message immediately of buffer it. Call ParseMessages()
    // in loop so that buffered messages will be sent.
    bool SendMessage(const tNMEA0183Msg &NMEA0183Msg);
    
    // These are obsolete. Use SendMessage
    bool SendMessage(const char *buf);
    void kick();
};

#endif
