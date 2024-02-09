/*
NMEA0183LinuxStream.cpp

2018-2024 Copyright (c) Timo Lappalainen   All rights reserved

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

Inherited tNMEA0183Stream object for Linux
*/

#ifndef _NMEA0183_LINUX_STREAM_H_
#define _NMEA0183_LINUX_STREAM_H_

#include "NMEA0183Stream.h"

#if defined(__linux__)||defined(__linux)||defined(linux)
//-----------------------------------------------------------------------------
class tNMEA0183LinuxStream : public tNMEA0183Stream {
protected:
  int port;
public:
    tNMEA0183LinuxStream(const char *_port=0);
    virtual ~tNMEA0183LinuxStream();
    int read();
    size_t write(const uint8_t* data, size_t size);
};
#endif

#endif /* _NMEA0183_LINUX_STREAM_H_ */

