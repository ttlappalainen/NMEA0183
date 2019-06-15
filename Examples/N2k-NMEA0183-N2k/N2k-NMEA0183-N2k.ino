/*
 NMEA0183/NMEA2000 library. NMEA0183 -> NMEA2000 -> NMEA0183
   Reads GPS messages from NMEA0183_In (Serial3) and forwards them to the N2k bus
   and to NMEA0183_out (SerialUSB)
   Also forwards all NMEA2000 bus messages to the PC (Serial)
   Forwards also wind data from N2k bus to the NMEA0183_out

 To use this example you need both NMEA2000 and NMEA0183 library installed.

 The example works with default settings on Arduino DUE, since it uses
 board second USB SerialUSB. That can be changed by definitions on the 
 beginning of code. Note that on DUE SerialUSB blocks for some reason, when
 you close the port on PC, so the whole program stops. It continues, when
 you open port again. If you do not need NMEA2000 messages forwarded to PC,
 define Serial for NMEA0183_Out_Stream and comment line:
 #define N2kForward_Stream Serial

 Example reads NMEA0183 messages from one serial port. It is possible
 to add more serial ports for having NMEA0183 combiner functionality.

 The NMEA0183 messages, which will be handled has been defined on NMEA0183Handlers.cpp
 on NMEA0183Handlers variable initialization. So this does not automatically
 handle all NMEA0183 messages. If there is no handler for some message you need,
 you have to write handler for it and add it to the NMEA0183Handlers variable
 initialization. If you write new handlers, please after testing send them to me,
 so I can add them for others use.

 Example also reads NMEA2000 messages and forwards them to defined N2kForward_Stream
 - Serial as default.

 For NMEA2000 message handling and forwarding to NMEA0183_Out there is only sample 
 N2kWindDataHandler defined on N2kWindDataHandler.h. For each new handler you need 
 to create handler class for it and call NMEA2000.AttachMsgHandler(&<handler class>); 
 as for N2kWindDataHandler at end of setup()
*/

#include <Arduino.h>
#include <Time.h>
#include <N2kMsg.h>
#include <NMEA2000.h>
#include <N2kMessages.h>
#include <NMEA0183.h>
#include <NMEA0183Msg.h>
#include <NMEA0183Messages.h>
#include "NMEA0183Handlers.h"
#include "N2kWindDataHandler.h"
#include "BoatData.h"

#include <NMEA2000_CAN.h>  // This will automatically choose right CAN library and create suitable NMEA2000 object

tBoatData BoatData;

#define NMEA0183_In_Stream_Speed 19200
#define NMEA0183_In_Stream Serial3
#define NMEA0183_Out_Stream_Speed 115200
#define NMEA0183_Out_Stream SerialUSB
#define N2kForward_Stream_Speed 115200
#define N2kForward_Stream Serial

tNMEA0183 NMEA0183_In;
tNMEA0183 NMEA0183_Out;

// Set the information for other bus devices, which messages we support
const unsigned long TransmitMessages[] PROGMEM={126992L,127250L,127258L,129026L,129029L,0};
const unsigned long ReceiveMessages[] PROGMEM={130306L,0};

// NMEA 2000 handlers
tN2kWindDataHandler N2kWindDataHandler;

// *****************************************************************************
// Empty stream input buffer. Ports may get stuck, if they get data in and it will
// not be read out.
void FlushStreamInput(Stream &stream) {
  while ( stream.available() ) stream.read();
}

// *****************************************************************************
void setup() {
  // Setup NMEA2000 system
  #ifdef N2kForward_Stream
  N2kForward_Stream.begin(N2kForward_Stream_Speed);
  #endif
  NMEA0183_In_Stream.begin(NMEA0183_In_Stream_Speed);
  NMEA0183_Out_Stream.begin(NMEA0183_Out_Stream_Speed);
  delay(1000); // Give some time for serial to initialize

  #ifdef N2kForward_Stream
  NMEA2000.SetForwardStream(&N2kForward_Stream);
  #endif
  NMEA2000.SetProductInformation("00000008", // Manufacturer's Model serial code
                                 107, // Manufacturer's product code
                                 "N2k->NMEA0183->N2k->PC",  // Manufacturer's Model ID
                                 "1.0.0.1 (2018-03-23)",  // Manufacturer's Software version code
                                 "1.0.0.0 (2018-03-23)" // Manufacturer's Model version
                                 );
  // Det device information
  NMEA2000.SetDeviceInformation(8, // Unique number. Use e.g. Serial number.
                                130, // Device function=PC Gateway. See codes on http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20%26%20function%20codes%20v%202.00.pdf
                                25, // Device class=Inter/Intranetwork Device. See codes on http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20%26%20function%20codes%20v%202.00.pdf
                                2046 // Just choosen free from code list on http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf                               
                               );

  // NMEA2000.SetForwardType(tNMEA2000::fwdt_Text); // Show in clear text. Leave uncommented for default Actisense format.
  NMEA2000.SetMode(tNMEA2000::N2km_ListenAndNode,25);
  //NMEA2000.EnableForward(false);

  NMEA2000.ExtendTransmitMessages(TransmitMessages);
  NMEA2000.ExtendReceiveMessages(ReceiveMessages);

  NMEA2000.Open();

  // Setup NMEA0183 ports and handlers
  InitNMEA0183Handlers(&NMEA2000, &BoatData);
  NMEA0183_In.SetMsgHandler(HandleNMEA0183Msg);

  NMEA0183_In.SetMessageStream(&NMEA0183_In_Stream);
  NMEA0183_In.Open();
  NMEA0183_Out.SetMessageStream(&NMEA0183_Out_Stream);
  NMEA0183_Out.Open();
  
  NMEA2000.AttachMsgHandler(&N2kWindDataHandler);
}

// *****************************************************************************
void loop() {
  NMEA2000.ParseMessages();
  NMEA0183_In.ParseMessages();
  NMEA0183_Out.ParseMessages();
  // We need to clear output streams input data to avoid them to get stuck.
  FlushStreamInput(NMEA0183_Out_Stream);
  #ifdef N2kForward_Stream
  FlushStreamInput(N2kForward_Stream);
  #endif
  
  SendSystemTime();
}

#define TimeUpdatePeriod 1000

// *****************************************************************************
void SendSystemTime() {
  static unsigned long TimeUpdated=millis();
  tN2kMsg N2kMsg;

  if ( (TimeUpdated+TimeUpdatePeriod<millis()) && BoatData.DaysSince1970>0 ) {
    SetN2kSystemTime(N2kMsg, 0, BoatData.DaysSince1970, BoatData.GPSTime);
    TimeUpdated=millis();
    NMEA2000.SendMsg(N2kMsg);
  }
}


