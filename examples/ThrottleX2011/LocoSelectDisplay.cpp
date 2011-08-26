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

#include "Globals.h"
#include "LocoSelectDisplay.h"

void LocoSelectDisplay::DisplayMenu(void)
{
    global_lcd.fillrect(0, 39, 84, 9, 1);
    for(int i = 0; i < 3; ++i)
    {
      if(_locos[i].hasAddress()) //if it's been assigned
      {
        global_lcd.drawstringinverse(i*30,5,"     ");
        global_lcd.drawstringinverse(i*30,5,String(_locos[i].getAddress(),DEC));
      }
      else if(_locos[i].isAttaching())
      {
        global_lcd.drawstringinverse(i*30,5,".... ");
      }
      else
      {
        global_lcd.drawstringinverse(i*30,5,"---- ");
      }
      global_lcd.drawline(i*30+26, 48, i*30+26, 40, 0);
    }
}

void LocoSelectDisplay::ProcessMenuKey(unsigned short key)
{
  //Three possibilities: If address = 0, the user didn't enter an address. Use the address already assigned to the menu key
  //If address > 0 the user did enter an address. Assign that address to the selected menu key.
  //if address < 0 the user wants to release the specified loco only.
  unsigned short i = 0;
  if(key == 16)
    i = 0;
  else if (key == 12)
    i = 1;
  else if (key == 8)
    i = 2;
  else //error!
    return;
  if((_address > 0) && (_address < 10000)) //an address was entered
  {
//    Serial.print("Got a new address: ");
//    Serial.println(_address,DEC);
    _locos[i].setAddress(_address);
    _address = 0;
    global_throttle = &_locos[i];
    //go to main screen
    global_lcd.clear();
    global_state = DISP_MAIN;
  }
  else if(_address == 0)//no address was entered
  {
//    Serial.println("no address entered!");
    if(!_locos[i].hasAddress() && !_locos[i].isAttaching()) //nothing is in this slot
    {
      return;
    }
    _address = 0;
    global_throttle = &_locos[i];
    //go to main screen
    global_lcd.clear();
    global_state = DISP_MAIN;
  }
  else if(_address < 0) //release request
  {
    if(!_locos[i].hasAddress())
    {
       return;
    }
    _locos[i].release();
    _address = 0;
  }
  
  delay(50);
}

void LocoSelectDisplay::Display(void)
{
  if(_address >= 0)
  {
     global_lcd.drawstring(6,1,"Address: ");
     String add(_address, DEC);
     global_lcd.drawstring(12,2,add);
     global_lcd.drawstring(12+(6*add.length()), 2, "    ");
  }
  else //releasing
  {
      global_lcd.drawstring(6,1,"Release?");
      global_lcd.drawstring(6,2,"         ");
  }
}

void LocoSelectDisplay::ProcessKey(unsigned short key)
{
  unsigned short val = 99;  
  switch(key)
  {
    case 3: //backspace!
      _address = (unsigned short)(_address / 10); //back it up! Does this do integer division correctly?
      //Serial.println(_address);
      break;
    case 4: //release loco
      _address = -99; //flag to release loco only.
      //Serial.clear();
      return;
    case 2:
      break;

    case 1: //0
      val = 0;
      break;
    case 0xD:
      val = 1;
      break;
    case 9:
      val = 2;
      break;
    case 5:
      val = 3;
      break;
    case 0xE:
      val = 4;
      break;
    case 0xA:
      val = 5;
      break;
    case 6:
      val = 6;
      break;
    case 0xF:
      val = 7;
      break;
    case 0xB:
      val = 8;
      break;
    case 7:
      val = 9;
      break;
  }

  //Figure out what to do with it.
  if((val != 99) && (_address*10 < 10000)) //if a numeric key was pressed
  {
    if(_address == -99)
    {
       _address = val;
    }
    else
    {
      _address *= 10;
      _address += val;
    }
    //Serial.println(_address);
  }
}
