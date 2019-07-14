/*
NMEA0183.h

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

#ifndef _tNMEA0183_H_
#define _tNMEA0183_H_

#include <stdint.h>
#include "NMEA0183Stream.h"
#include "NMEA0183Msg.h"

#define MAX_NMEA0183_MSG_BUF_LEN 81  // According to NMEA 3.01. Can not contain multi message as in AIS

class tNMEA0183
{
  protected:
    tNMEA0183Stream *port;
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
    bool CanSendByte();
  public:
    tNMEA0183(tNMEA0183Stream *stream=0, uint8_t _SourceID=0);
    void SetMessageStream(tNMEA0183Stream *stream, uint8_t _SourceID=0);
    bool Open();
    #ifdef ARDUINO
    // Begin is obsolete. Use Open(...)
    void Begin(HardwareSerial *_port, uint8_t _SourceID=0, unsigned long _baud=4800);
    #endif
    // Set size for send message buffer. Call this before Open().
    void SetSendBufferSize(size_t size);
    // Set call back function, which will be called for new messages on ParseMessages.
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
