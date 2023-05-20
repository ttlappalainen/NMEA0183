/*
NMEA0183Messages.cpp

The MIT License

Copyright (c) 2015-2022 Timo Lappalainen, Kave Oy, www.kave.fi

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
// \brief Tests for functions in NMEA0183Messages.h.

#define CATCH_CONFIG_MAIN
#include <functional> // std::function

#include <string.h>
#include <catch2/catch.hpp>
#include <NMEA0183Messages.h>

// Plan is simple:
// 1. Convert the string nmea0183 to a src.
// 2. Invoke parse_and_set to parse src and set the dst.
// 3. Convert dst to string again and compare with the original.
void test_nmea0183_message(const char* nmea0183, std::function<void(tNMEA0183Msg&, tNMEA0183Msg&)> parse_and_set)
{
  char result[100];
  tNMEA0183Msg src, dst;

  CHECK(src.SetMessage(nmea0183));
  parse_and_set(src, dst);
  CHECK(dst.GetMessage(result, sizeof(result)));
  CHECK_THAT(result, Catch::Matchers::Equals(nmea0183));
}

TEST_CASE("DPT before NMEA0183 v3.0")
{
  test_nmea0183_message("$IIDPT,10.5,0.9*7D", [](tNMEA0183Msg& src, tNMEA0183Msg& dst) {
    double depth, offset;
    CHECK(NMEA0183ParseDPT(src, depth, offset));
    CHECK(NMEA0183SetDPT(dst, depth, offset));
  });
}

TEST_CASE("DPT after NMEA0183 v3.0")
{
  test_nmea0183_message("$IIDPT,10.5,0.9,100*60", [](tNMEA0183Msg& src, tNMEA0183Msg& dst) {
    double depth, offset, range;
    CHECK(NMEA0183ParseDPT(src, depth, offset, range));
    CHECK(NMEA0183SetDPT(dst, depth, offset, range));
  });
}

TEST_CASE("ZDA")
{
  test_nmea0183_message("$GPZDA,160012.71,11,03,2004,-1,00*7D", [](tNMEA0183Msg& src, tNMEA0183Msg& dst) {
    double GPSTime;
    int GPSDay, GPSMonth, GPSYear, LZD, LZMD;
    CHECK(NMEA0183ParseZDA(src, GPSTime, GPSDay, GPSMonth, GPSYear, LZD, LZMD));
    CHECK(NMEA0183SetZDA(dst, GPSTime, GPSDay, GPSMonth, GPSYear, LZD, LZMD));
  });
}
