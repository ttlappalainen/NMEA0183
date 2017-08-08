# NMEA0183
NMEA0183 library for Arduino

Object oriented NMEA0183 library for Arduino Boards

Library gives you a e.g. easy way to make NMEA0183->NMEA2000 interface like Actisense device.

Currently there are only few functions for parsing NMEA0183 messages. There are no
functions for sending or creating NMEA0183 messages, since the the original need for the
library was to parse message for forwarding them to the NMEA2000 bus.

To use this library you also need latest version of:
* Arduino Time library http://playground.arduino.cc/code/time
* Standard C++ library for Arduino https://github.com/maniacbug/StandardCplusplus

TODO: Rewrite the NMEA0183ToN2k example to use the new structs.
To compile example NMEA0183ToN2k you need also NMEA2000 library https://github.com/ttlappalainen/NMEA2000

