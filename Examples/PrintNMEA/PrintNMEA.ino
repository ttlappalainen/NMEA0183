
#include <StandardCplusplus.h>
#include <NMEA0183.h>
#include <NMEA0183Msg.h>
#include <NMEA0183Messages.h>

tNMEA0183Msg NMEA0183Msg;
tNMEA0183 NMEA0183;

void setup() {

  Serial.begin(9600);
  NMEA0183.Begin(&Serial3,3, 9600);
}

void loop() {

  tNMEA0183Msg NMEA0183Msg;
  while (NMEA0183.GetMessage(NMEA0183Msg)) {
	  Serial.print(NMEA0183Msg.Sender());
	  Serial.print(NMEA0183Msg.MessageCode());
  	for (int i=0; i < NMEA0183Msg.FieldCount(); i++) {
  		Serial.print(NMEA0183Msg.Field(i));
  	}
   Serial.print("\n");
  }
}
