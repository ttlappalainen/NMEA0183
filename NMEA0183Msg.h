/*
NMEA0183Msg.h

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

#ifndef _tNMEA0183Msg_H_
#define _tNMEA0183Msg_H_

#include <string.h>
#include <time.h>
#include "NMEA0183Stream.h"

const double   NMEA0183DoubleNA=-1e9;
const uint8_t  NMEA0183UInt8NA=0xff;
const int8_t   NMEA0183Int8NA=0x7f;
const uint16_t NMEA0183UInt16NA=0xffff;
const int16_t  NMEA0183Int16NA=0x7fff;
const uint32_t NMEA0183UInt32NA=0xffffffff;
const int32_t  NMEA0183Int32NA=0x7fffffff;

inline bool NMEA0183IsNA(double v) { return v==NMEA0183DoubleNA; }
inline bool NMEA0183IsNA(uint8_t v) { return v==NMEA0183UInt8NA; }
inline bool NMEA0183IsNA(int8_t v) { return v==NMEA0183Int8NA; }
inline bool NMEA0183IsNA(uint16_t v) { return v==NMEA0183UInt16NA; }
inline bool NMEA0183IsNA(int16_t v) { return v==NMEA0183Int16NA; }
inline bool NMEA0183IsNA(uint32_t v) { return v==NMEA0183UInt32NA; }
inline bool NMEA0183IsNA(int32_t v) { return v==NMEA0183Int32NA; }

#define MAX_NMEA0183_MSG_LEN 81  // According to NMEA 3.01. Can not contain multi message as in AIS
#define MAX_NMEA0183_MSG_FIELDS 20

#ifndef _Time_h
typedef tm tmElements_t;
#endif

//------------------------------------------------------------------------------
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


// Helper functions on converting TimeLib.h to time.h
  protected:
    static unsigned long TimeTDaysTo1970Offset; // Offset for time_t to 1.1.1970. Seem to vary betweel libraries.
    static unsigned long CalcTimeTDaysTo1970Offset();
    bool AddToBuf(const char *data, char * &buf, size_t &BufSize) const;

  public:
    #ifdef _Time_h
    static inline void SetYear(tmElements_t &TimeElements, int val) { TimeElements.Year=val-1970; } //
    static inline void SetMonth(tmElements_t &TimeElements, int val) { TimeElements.Month=val>0?val-1:val; }
    static inline void SetDay(tmElements_t &TimeElements, int val) { TimeElements.Day=val; }
    static inline void SetHour(tmElements_t &TimeElements, int val) { TimeElements.Hour=val; }
    static inline void SetMin(tmElements_t &TimeElements, int val) { TimeElements.Minute=val; }
    static inline void SetSec(tmElements_t &TimeElements, int val) { TimeElements.Second=val; }
    static inline int GetYear(const tmElements_t &TimeElements) { return TimeElements.Year+1970; }
    static inline int GetMonth(const tmElements_t &TimeElements) { return TimeElements.Month+1; }
    static inline int GetDay(const tmElements_t &TimeElements) { return TimeElements.Day; }
    static inline time_t makeTime(tmElements_t &TimeElements) { return ::makeTime(TimeElements); }
    static inline void breakTime(time_t time, tmElements_t &TimeElements) { ::breakTime(time,TimeElements); }
    #else
    static inline void SetYear(tmElements_t &TimeElements, int val) { TimeElements.tm_year=val-1900; } //
    static inline void SetMonth(tmElements_t &TimeElements, int val) { TimeElements.tm_mon=val>0?val-1:val; }
    static inline void SetDay(tmElements_t &TimeElements, int val) { TimeElements.tm_mday=val; }
    static inline void SetHour(tmElements_t &TimeElements, int val) { TimeElements.tm_hour=val; }
    static inline void SetMin(tmElements_t &TimeElements, int val) { TimeElements.tm_min=val; }
    static inline void SetSec(tmElements_t &TimeElements, int val) { TimeElements.tm_sec=val; }
    static inline int GetYear(const tmElements_t &TimeElements) { return TimeElements.tm_year+1900; }
    static inline int GetMonth(const tmElements_t &TimeElements) { return TimeElements.tm_mon+1; }
    static inline int GetDay(const tmElements_t &TimeElements) { return TimeElements.tm_mday; }
    static inline time_t makeTime(tmElements_t &TimeElements) { return mktime(&TimeElements); }
    static inline void breakTime(time_t time, tmElements_t &TimeElements) { TimeElements=*localtime(&time); }
    static time_t daysToTime_t(unsigned long val);
    #endif
    static unsigned long elapsedDaysSince1970(time_t dt);

  protected:
    void ForceNullTermination() { Data[MAX_NMEA0183_MSG_LEN-1]=0; } // Just force null termination for data

  public:
    uint8_t SourceID;  // This is used to separate messages e.g. from different ports. Receiver must set this.
    static const char *DefDoubleFormat;

  public:
    tNMEA0183Msg();
    // Set message from received null terminated buffer. Returns true if checksum is OK.
    bool SetMessage(const char *buf);
    // Get message as complete NMEA0183 format string to buffer.
    bool GetMessage(char *MsgData, size_t BufSize) const;
    // Clear message
    void Clear();
    // Print message fields
    //void PrintFields(Stream &port) const;
    // Send message in valid NMEA0183 format. This uses blocking write. See tNMEA0183::SendMessage
    void Send(tNMEA0183Stream &port) const;
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

    // Add field with no data. Causes ,, in final message.
    bool AddEmptyField();

    // Add string field. E.g. AddStrField("K") causes ,K, on final message.
    bool AddStrField(const char *FieldData);

    //
    bool AddUInt32Field(uint32_t val);

    // Add double field. val must be in SI units (as in NMEA2000). Provide multiplier for conversion and
    // Format (default %.1f), if necessary. If you also provide Unit, it will be added as own field.
    // Note also that function tests is value valid and adds empty field, if it is not.
    // Examples:
    // NMEA0183Msg.AddDoubleField(120.123,radToDeg,tNMEA0183Msg::DefDoubleFormat,"M"); -> ,120.1,M
    // NMEA0183Msg.AddDoubleField(23.123); -> ,23.1
    bool AddDoubleField(double val, double multiplier=1, const char *Format=DefDoubleFormat, const char *Unit=0);

    // Add time field. GPSTime is just seconds since midnight.
    bool AddTimeField(double GPSTime, const char *Format="%09.2f");

    // Add Days field.
    bool AddDaysField(unsigned long DaysSince1970);

    // Add Latitude field. Also E/W will be added. Latitude is in degrees. Negative value is W. E.g.
    // AddLatitudeField(-5.2345); -> ,5.235,W
    bool AddLatitudeField(double Latitude, const char *Format="%.3f");

    // Add Longitude field. Also N/S will be added. Longitude is in degrees. Negative value is S. E.g.
    // AddLongitudeField(-5.2345); -> ,514.070,S
    bool AddLongitudeField(double Longitude, const char *Format="%.3f");

    // Helper function to convert GPSTime to NMEA0183 time (hhmmss.sss). E.g. 42000.55 -> 114000.55
    static double GPSTimeToNMEA0183Time(double GPSTime);

    // Helper function to convert double angle to ddmm.zzz format. E.g. 5.2345 -> 514.070
    static double DoubleToddmm(double val);

    // Helper function to convert days since 1970 to ddmmyyyy.
    static unsigned long DaysToNMEA0183Date(unsigned long val);
};

#endif
