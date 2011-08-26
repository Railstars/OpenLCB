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

#ifndef __LOCOMOTIVE_FACTORY_H__
#define __LOCOMOTIVE_FACTORY_H__

#include <OLCB_Datagram_Handler.h>
//#include <MemoryFree.h>
#include "Locomotive.h"

#if defined(__AVR_AT90CAN128__) || defined(__AVR_ATMEGA1280__)
#define NUM_SLOTS 20
#else
#define NUM_SLOTS 5
#endif

class LocomotiveFactory : public OLCB_Datagram_Handler
{
  public:
  
  bool verifyNID(OLCB_NodeID *nid);
  
 // bool processDatagram(void);
  
  void update(void);
  
 private:
  Locomotive _locos[NUM_SLOTS];
};

#endif
