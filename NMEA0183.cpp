/*
NMEA0183.cpp

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

#ifndef ARDUINO
#include <cstdio>
#endif
#include "NMEA0183.h"

//*****************************************************************************
tNMEA0183::tNMEA0183(tNMEA0183Stream *stream, uint8_t _SourceID)
: port(0), MsgCheckSumStartPos(SIZE_MAX),
  MsgInPos(0), MsgInStarted(false),
  MsgOutWritePos(0), MsgOutReadPos(0), MsgOutBuf(0), MsgOutBufSize(3*MAX_NMEA0183_MSG_BUF_LEN),
  MsgHandler(0)
{
  SetMessageStream(stream,_SourceID);
}

//*****************************************************************************
void tNMEA0183::SetMessageStream(tNMEA0183Stream *stream, uint8_t _SourceID) {
  SourceID=_SourceID;
  port=stream;
}

//*****************************************************************************
bool tNMEA0183::Open() {
  if ( !IsOpen() ) {
    if ( MsgOutBuf==0 ) MsgOutBuf=new char[MsgOutBufSize];
    MsgInPos=0; MsgInStarted=false;
    MsgOutWritePos=0; MsgOutReadPos=0;

    return IsOpen();
  }

  return true;
}

#ifdef ARDUINO
//*****************************************************************************
void tNMEA0183::Begin(HardwareSerial *_port, uint8_t _SourceID, unsigned long _baud) {
  _port->begin(_baud);
  SetMessageStream(_port,_SourceID);

  Open();
}
#endif

//*****************************************************************************
void tNMEA0183::SetSendBufferSize(size_t size) {
  if ( MsgOutBuf==0 ) {
    MsgOutBufSize=size;
  }
}

//*****************************************************************************
void tNMEA0183::ParseMessages() {
  tNMEA0183Msg NMEA0183Msg;

    if ( !Open() ) return;

    while (GetMessage(NMEA0183Msg)) {
      if (MsgHandler!=0) MsgHandler(NMEA0183Msg);
    }
    kick();
}

//*****************************************************************************
bool tNMEA0183::GetMessage(tNMEA0183Msg &NMEA0183Msg) {
  if ( !IsOpen() ) return false;

  bool result=false;

  while (port->available() > 0 && !result) {
    int NewByte=port->read();
      if (NewByte=='$' || NewByte=='!') { // Message start
        MsgInStarted=true;
        MsgInPos=0;
        MsgInBuf[MsgInPos]=NewByte;
        MsgInPos++;
      } else if (MsgInStarted) {
        MsgInBuf[MsgInPos]=NewByte;
        if (NewByte=='*') MsgCheckSumStartPos=MsgInPos;
        MsgInPos++;
        if (MsgCheckSumStartPos!=SIZE_MAX and MsgCheckSumStartPos+3==MsgInPos) { // We have full checksum and so full message
            MsgInBuf[MsgInPos]=0; // add null termination
          if (NMEA0183Msg.SetMessage(MsgInBuf)) {
            NMEA0183Msg.SourceID=SourceID;
            result=true;
          }
          MsgInStarted=false;
          MsgInPos=0;
          MsgCheckSumStartPos=SIZE_MAX;
        }
        if (MsgInPos>=MAX_NMEA0183_MSG_BUF_LEN) { // Too may chars in message. Start from beginning
          MsgInStarted=false;
          MsgInPos=0;
          MsgCheckSumStartPos=SIZE_MAX;
        }
      }
  }

  return result;
}

//*****************************************************************************
bool tNMEA0183::SendMessage(const tNMEA0183Msg &NMEA0183Msg) {
  if ( !Open() ) return false;

  char buf[7]={NMEA0183Msg.GetPrefix(),0};

  SendBuf(buf);
  SendBuf(NMEA0183Msg.Sender());
  SendBuf(NMEA0183Msg.MessageCode());
  for (int i=0; i<NMEA0183Msg.FieldCount(); i++) {
    SendBuf(",");
    SendBuf(NMEA0183Msg.Field(i));
  }
  sprintf(buf,"*%02X\r\n",NMEA0183Msg.GetCheckSum());
  return SendBuf(buf);
}

//*****************************************************************************
// availableForWrite does not exists on all implementations.
bool tNMEA0183::CanSendByte() {
  #if defined(ARDUINO_ARCH_ESP32)
  return true;
  #else
  return port->availableForWrite() > 0;
  #endif
}

//*****************************************************************************
void tNMEA0183::kick() {
  if ( !Open() ) return;

  while ( MsgOutWritePos!=MsgOutReadPos && CanSendByte() ) {
    port->write(MsgOutBuf[MsgOutReadPos]);
    MsgOutReadPos=(MsgOutReadPos + 1) % MsgOutBufSize;
  }
}

//*****************************************************************************
bool tNMEA0183::SendBuf(const char *buf) {
  kick();

  if ( buf==0 ) return true;

  size_t iBuf=0;

  if ( MsgOutWritePos==MsgOutReadPos ) { // try to send immediately
    for (; CanSendByte() > 0 && buf[iBuf]!=0; iBuf++ ) {
      port->write(buf[iBuf]);
    }
  }

  if ( buf[iBuf]==0 ) {
    return true;
  }

  // Could not send immediately, so buffer message
  if ( strlen(buf)-iBuf >= MsgOutBufFreeSize() ) return false; // No room for message

  size_t wp=MsgOutWritePos;

  for (size_t temp = (MsgOutWritePos + 1) % MsgOutBufSize;
       buf[iBuf]!=0 && temp!=MsgOutReadPos;
       temp = (MsgOutWritePos + 1) % MsgOutBufSize ) {
    MsgOutBuf[MsgOutWritePos]=buf[iBuf];
    MsgOutWritePos=temp;
  }

  if ( buf[iBuf]!=0 ) {
    MsgOutWritePos=wp;  // Cancel sending
    return false;
  }

  return true;
}

//*****************************************************************************
bool tNMEA0183::SendMessage(const char *buf) {
  if ( !Open() ) return false;
  // Add check that there is crlf at end.
  return SendBuf(buf);
}
