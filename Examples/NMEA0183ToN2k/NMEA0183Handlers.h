/* 
NMEA0183Handlers.h

2015 Copyright (c) Kave Oy, www.kave.fi  All right reserved.

Author: Timo Lappalainen

  This library is free software; you can redistribute it and/or
  modify it as you like.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/
 
#ifndef _NMEA0183Handlers_H_
#define _NMEA0183Handlers_H_
#include <Arduino.h>
#include <Time.h>
#include <NMEA0183.h>
#include <NMEA0183Msg.h>
#include <NMEA2000.h>
#include "BoatData.h"

void InitNMEA0183Handlers(tNMEA2000 *_NMEA2000, tBoatData *_BoatData);
void DebugNMEA0183Handlers(Stream* _stream);

void HandleNMEA0183Msg(const tNMEA0183Msg &NMEA0183Msg);

#endif


