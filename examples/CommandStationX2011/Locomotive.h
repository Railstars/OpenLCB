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
    
***************************************************************************************/

#ifndef __LOCOMOTIVE_H__
#define __LOCOMOTIVE_H__

#include <DCCPacketScheduler.h>

#include <OLCB_Datagram_Handler.h>
#include <OLCB_NodeID.h>
#include <OLCB_CAN_Link.h>

#define LOCOMOTIVE_INITIAL 0
#define LOCOMOTIVE_ATTACHING 1
#define LOCOMOTIVE_ATTACHED 2
#define LOCOMOTIVE_RELEASING 3

//datagram byte 0 (command identifier)le
#define DATAGRAM_MOTIVE                 0x30
//datagram byte 1 (sub-command identifier, lower nibble only; upper nibble reserverd)
#define DATAGRAM_MOTIVE_ATTACH          0x01
#define DATAGRAM_MOTIVE_ATTACHED        0x02
#define DATAGRAM_MOTIVE_ATTACH_DENIED   0x03
#define DATAGRAM_MOTIVE_RELEASE         0x04
#define DATAGRAM_MOTIVE_RELEASED        0x05
#define DATAGRAM_MOTIVE_SETSPEED        0x06
#define DATAGRAM_MOTIVE_GETSPEED        0x07
#define DATAGRAM_MOTIVE_SETFUNCTION     0x08
#define DATAGRAM_MOTIVE_GETFUNCTION     0x09
#define DATAGRAM_MOTIVE_SUBCOMMAND_MASK 0x0F
#define DATAGRAM_MOTIVE_SUBCOMMAND_FLAGS_MASK 0xF0


//This is a total hack to avoid eating up too much memory by storing a pointer in each instance. Tried using
// a static class member, but avr-gcc seems to puke on this kind of usage?
extern DCCPacketScheduler packetScheduler;

class Locomotive : public OLCB_Datagram_Handler
{
 friend class LocomotiveFactory;
 public:
  Locomotive() :available(true), state(LOCOMOTIVE_INITIAL), speed(0), direction(1), verified(true) {}

  void reset(void);
  void update(void);
  bool processDatagram(void);
  void datagramResult(bool accepted, uint16_t errorcode);
  
  //bool verifyNID(OLCB_NodeID *nid);
  
  //used to determine if this locomotive can be safely deleted.
  bool isAvailable(void) { return available; }
 
 private:
  //some methods for handling various possible incoming datagrams
  bool attachDatagram(void);
  bool releaseDatagram(void);
  bool setSpeedDatagram(void);
  bool setFunctionDatagram(void);
  
  uint16_t getDCCAddress(void); 
 
  uint8_t speed; //in percent full throttle
  int8_t direction;
  uint16_t functions; //stores function states
  OLCB_NodeID throttle;
  bool available;
  uint8_t state;
  uint16_t timer;
  bool verified;
  bool sendAttached;
};

//TODO: No method for removing a Locomotive (to undo a setLink() and setNID() operation)


#endif
