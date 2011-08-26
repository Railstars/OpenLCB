/***************************************************************************************
ThrottleX2011
A demonstration of a very basic OpenLCB throttle.
Copyright (C)2011 D.E. Goodman-Wilson

This file is part of ThrottleX2011.

    ThrottleX2011 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ThrottleX2011.  If not, see <http://www.gnu.org/licenses/>.
    
***************************************************************************************/

#ifndef __LOCOSELECTDISPLAY_H__
#define __LOCOSELECTDISPLAY_H__

#include "Throttle.h"
#include "Interface.h"
#include <OLCB_Link.h>

class LocoSelectDisplay : public Interface
{
 public:
  void init(OLCB_Link *link)
  {
    _address = 0;
    OLCB_NodeID nid(2,1,13,0,0,3);
    for(int i = 0; i < 3; ++i)
    {
      nid.set(2,1,13,0,0,3+i); //this is crap.
      _locos[i].setLink(link);
      _locos[i].setNID(&nid);
    }
  }
  
  boolean ready(void)
  {
    for(int i = 0; i < 3; ++i)
    {
//      Serial.print(_locos[i].NID->alias, DEC);
//      Serial.print(" ");
      if(!_locos[i].NID->alias)
      {
//        Serial.println();
        return false;
      }
    }
//    Serial.println();
    return true;
  }
  
  void Display(void);
  void DisplayMenu(void);
  void ProcessKey(unsigned short key);
  void ProcessMenuKey(unsigned short key);
 private:
  int _address;
  Throttle _locos[3];
};

#endif
