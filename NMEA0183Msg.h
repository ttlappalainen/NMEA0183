/* 
NMEA0183Msg.h

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

#ifndef _tNMEA0183Msg_H_
#define _tNMEA0183Msg_H_

#include <string.h>
#include <Arduino.h>  // Required by stream. Make similar as with NMEA2000 library

const double   NMEA0183DoubleNA=-1e9;
const uint8_t  NMEA0183UInt8NA=0xff;
const int8_t   NMEA0183Int8NA=0x7f;
const uint16_t NMEA0183UInt16NA=0xffff;
const int16_t  NMEA0183Int16NA=0x7fff;
const uint32_t NMEA0183UInt32NA=0xffffffff;
const int32_t  NMEA0183Int32NA=0x7fffffff;

#define MAX_NMEA0183_MSG_LEN 81  // Accroding to NMEA 3.01. Can not contain multi message as in AIS
#define MAX_NMEA0183_MSG_FIELDS 20

class tNMEA0183Msg
{
  protected:
    static const char *EmptyField;
    unsigned long _MessageTime;
    char Data[MAX_NMEA0183_MSG_LEN];
    uint8_t iAddData;
    char Prefix;
    uint8_t Fields[MAX_NMEA0183_MSG_FIELDS];
    uint8_t _FieldCount;
    uint8_t CheckSum;
  
  protected:
    void ForceNullTermination() { Data[MAX_NMEA0183_MSG_LEN-1]=0; } // Just force null termination for data
  public:
    uint8_t SourceID;  // This is used to separate messages e.g. from different ports. Receiver must set this.
  public:
    tNMEA0183Msg();
    // Set message from received null terminated buffer. Returns true if checksum is OK.
    bool SetMessage(const char *buf);
    // Clear message
    void Clear();
    // Print message fields
    //void PrintFields(Stream &port) const;
    // Send message in valid NMEA0183 format. This uses blocking write. See tNMEA0183::SendMessage
    void Send(Stream &port) const;
    // Return count of fields on message
    uint8_t FieldCount() const { return _FieldCount; }
    // Return field in null terminated string
    const char *Field(uint8_t index) const;
    char GetPrefix() const { return Prefix; }
    // Return sender code (like GP) in null terminated string
    const char *Sender() const { return Data; }
    // Return message code (like GGA) in null terminated string
    const char *MessageCode() const { return Data+3; }
    // Return checksum of message.
    uint8_t GetCheckSum() const { return CheckSum; }
    // Check is message code given
    bool IsMessageCode(const char* _code) const { return (strcmp(MessageCode(),_code)==0); }
    //
    unsigned long MessageTime() const { return _MessageTime; }
    // Return length of field
    unsigned int FieldLen(uint8_t index) const;
    
    // Init message building.
    bool Init(const char *_MessageCode, const char *_Sender="II", char _Prefix='$');
    //
    bool AddEmptyField();
    //
    bool AddStrField(const char *FieldData);
    //
    bool AddDoubleField(double val, double multiplier=1, const char *Format="%.1f");
    //
    bool AddTimeField(double GPSTime, const char *Format="%09.2f");
    //
    bool AddDaysField(unsigned long DaysSince1970);
    //
    bool AddLatitudeField(double Latitude, const char *Format="%.3f");
    //
    bool AddLongitudeField(double Longitude, const char *Format="%.3f");
    
    static double GPSTimeToNMEA0183Time(double GPSTime);
    static double DoubleToddmm(double val);
    static unsigned long DaysToNMEA0183Date(unsigned long val);
};

#endif