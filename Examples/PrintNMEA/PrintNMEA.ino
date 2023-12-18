
#include <NMEA0183.h>
#include <NMEA0183Msg.h>
#include <NMEA0183Messages.h>

// Uncomment below to use SoftwareSerial for NMEA0183 port instead of hw. This works e.g., with Arduino Uno
// #define USE_SOFTWARE_SERIAL 1
// If you use SoftwareSerial, define NMEA0183 port Rx/Tx pins below
#define NMEA0183_RX_PIN 0
#define NMEA0183_TX_PIN 1

#if defined(USE_SOFTWARE_SERIAL)
#include <SoftwareSerial.h>

SoftwareSerial NMEA0183Port(0, 1, false);
#else
HardwareSerial &NMEA0183Port=Serial1;
#endif

tNMEA0183Msg NMEA0183Msg;
tNMEA0183 NMEA0183;


void setup() {
  Serial.begin(9600);
  NMEA0183Port.begin(4800);
  NMEA0183.SetMessageStream(&NMEA0183Port,3);
  #if defined(USE_SOFTWARE_SERIAL)
  Serial.println("Open SoftwareSerial for NMEA0183 port");
  #else
  Serial.println("Open HardwareSerial for NMEA0183 port");
  #endif
  NMEA0183.Open();
}

void loop() {
  tNMEA0183Msg NMEA0183Msg;
  
  while (NMEA0183.GetMessage(NMEA0183Msg)) {
    Serial.print(NMEA0183Msg.Sender());
    Serial.print(NMEA0183Msg.MessageCode()); Serial.print(" ");
    for (int i=0; i < NMEA0183Msg.FieldCount(); i++) {
      Serial.print(NMEA0183Msg.Field(i));
      if ( i<NMEA0183Msg.FieldCount()-1 ) Serial.print(" ");
    }
   Serial.print("\n");
  }
}
