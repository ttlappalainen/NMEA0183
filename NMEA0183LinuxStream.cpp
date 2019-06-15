/*
NMEA0183LinuxStream.cpp

2018 Copyright (c) Timo Lappalainen   All rights reserved

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

#if defined(__linux__)||defined(__linux)||defined(linux)

//#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include "NMEA0183LinuxStream.h"


//*****************************************************************************
tNMEA0183LinuxStream::tNMEA0183LinuxStream(const char *_port) : port(-1) {
  if ( _port!=0 ) {
    port=open(_port, O_RDWR | O_NOCTTY | O_NDELAY);
  }

  if ( port!=-1 ) {
//    cout << _port << " Opened" << endl;
  } else {
//    if ( _port!=0 ) cout << "Failed to open port " << _port << endl;
  }
}

//*****************************************************************************
tNMEA0183LinuxStream::~tNMEA0183LinuxStream() {
  if ( port!=-1 ) {
    close(port);
  }
}


/********************************************************************
*	Other 'Bridge' functions and classes
*
*
*
**********************************************************************/
int tNMEA0183LinuxStream:: read() {
  if ( port!=-1 ) {
    return -1;
  } else {
    // Serial stream bridge -- Returns first byte if incoming data, or -1 on no available data.
    struct timeval tv = { 0L, 0L };
    fd_set fds;

    FD_ZERO(&fds);
    FD_SET(0, &fds);
    if (select(1, &fds, NULL, NULL, &tv) < 0)                                   // Check fd=0 (stdin) to see if anything is there (timeout=0)
        return -1;                                                              // Nothing is waiting for us.

   return (getc(stdin));                                                         // Something is there, go get one char of it.
  }
}



//*****************************************************************************
size_t tNMEA0183LinuxStream:: write(const uint8_t* data, size_t size) {                // Serial Stream bridge -- Write data to stream.
  if ( port!=-1 ) {
    return ::write(port,data,size);
  } else {
    size_t i;

    for (i=0; (i<size) && data[i];  i++)                                        // send chars to stdout for 'size' or until null is found.
        putc(data[i],stdout);

    return(i);
  }
}

#endif
