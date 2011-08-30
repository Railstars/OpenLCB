/***************************************************************************************
CommandStationX2011
A demonstration of a very basic OpenLCB DCC command station.
Copyright (C)2011 D.E. Goodman-Wilson

This file is part of ThrottleX2011.

    CommandStationX2011 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CommandStationX2011.  If not, see <http://www.gnu.org/licenses/>.
    
*******************************************

Hardware Requirements:
  Some kind of CAN interface.
  A second Arduino with CAN interface running the ThrottleX2011 demo.

Software Requirements:
  "OpenLCB" library
  "CAN" library (from OpenLCB repository)
  "CmdrArduino" library (from github)

*******************************************

Implements the MOTIVE protocol in the following way.

A VerifyID request for an address in the DCC range, when passed to LocomotiveFactory, creates a new vnode
to service that NID. It then emits a VerifiedID.

Only one throttle may attach to a train at a time. No handing off is handled: The throttle
must release a train before another can attach.

Speeds are assumed to be percentages of full throttle.

Functions are assumed to be either off (0 value) or on (non-0 value).

*******************************************

KNOWN BUGS:

-Little really ever times out. Which can cause problems.
-Released trains are immediately discarded, bringing them to a screeching halt;
  would be better if the state were maintained so trains could be easily handed off
  from one throttle to another.
-Memory issues confine us to 5 throttles on an ATmega328 and 20 on an AT90CAN128.
  That's too few!

***************************************************************************************/


//#include <MemoryFree.h>

#define DATAGRAM_LENGTH 5 //5 bytes at most. really.
#define OLCB_DEBUG

#include <OpenLCB.h>

#include <DCCPacket.h>
#include <DCCPacketQueue.h>
#include <DCCPacketScheduler.h>

#include <can.h>

#include "Locomotive.h"
#include "LocomotiveFactory.h"


DCCPacketScheduler packetScheduler;
LocomotiveFactory factory;

OLCB_NodeID nid(5,2,1,2,0,0);
OLCB_CAN_Link link(&nid);

void setup()
{
  //  Serial.begin(115200);
  //  Serial.println("Hello world!");
//  delay(50);
  link.initialize();
  factory.setLink((OLCB_Link*)&link);
//  //  Serial.println(freeMemory(),DEC);
//  //  Serial.println(sizeof(OLCB_Datagram), DEC);
  packetScheduler.setup();
}

void loop()
{
  packetScheduler.update();
  link.update();
//  //  Serial.println(freeMemory(),DEC);
}
