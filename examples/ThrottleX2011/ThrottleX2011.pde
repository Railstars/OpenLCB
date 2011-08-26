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
    
*******************************************

Hardware Requirements:
  An iTeadStudio iBridge shield
    (http://iteadstudio.com/store/index.php?main_page=product_info&products_id=308)
  A Nokia 5110 breakout (not the Adafruit one, the pinout is different than the ones listed below)
    (http://iteadstudio.com/store/index.php?main_page=product_info&cPath=3&products_id=155&zenid=efscc20sp8decajjrr1r3pu8d0)
    (http://www.sparkfun.com/products/10168)
  Some kind of CAN interface.
  A second Arduino with CAN interface running the CommandStationX2011 demo.

Software Requirements:
  "OpenLCB" library
  "CAN" library (from OpenLCB repository)
    
The software has two modes: Train Select and Train Drive. The hardware begins in Train Select mode.

----------------

Train Select mode keypad layout:

Throttle1  Throttle2  Throttle3    Release
    7          8          9       Backspace
    4          5          6         n/a
    1          2          3          0

In Train Select mode, you enter the address of a train to control, then hit the key corresponding
to the throttle to assign the train to. To release a train, hit the Release key, then choose
the throttle to release.
To switch to an alread-attached throttle, just hit the corresponding throttle key.

----------------

Train Drive mode keypad layout:

TrainSelectMode  n/a   n/a  ToggleDirection
      F7         F8    F9     Throttle+
      F4         F5    F6     Throttle-
      F1         F2    F3       F0

Trains default at: direction=forward, speed=0, All functions off, save F0 which is on.
F0 -- F9 toggle functions on and off.
Throttle+/- increase/decrease train speed.
ToggleDirection changes the direction of the train (warning: hitting this button at high speed might damage your train!)
TrainSelectMode takes you back to the train selection screen.

*******************************************

KNOWN BUGS:

-Don't ever try to release a train while not connected to the Command Station. This will throw the throttle
 out of sync, and cause problems, nevermind that the command station will never get the RELEASE request.

***************************************************************************************/

#include <OLCB_AliasCache.h>
#include <OLCB_Buffer.h>
#include <OLCB_CAN_Buffer.h>
#include <OLCB_CAN_Link.h>
#include <OLCB_Datagram.h>
#include <OLCB_Datagram_Handler.h>
#include <OLCB_Event.h>
#include <OLCB_EventID.h>
#include <OLCB_Handler.h>
#include <OLCB_Link.h>
#include <OLCB_NodeID.h>
#include <OLCB_Stream.h>

#include <can.h>

#include "IBridge.h"
#include "Throttle.h"
#include "PCD8544.h"

// The dimensions of the LCD (in pixels)...
static const byte LCD_WIDTH = 84;
static const byte LCD_HEIGHT = 48;

PCD8544 global_lcd = PCD8544(8, 9, 10, 12, 11);
OLCB_NodeID nid(5,2,1,2,0,1);
OLCB_CAN_Link link(&nid);

Throttle *global_throttle;
unsigned short global_state;

#include "Globals.h"
#include "LocoSelectDisplay.h"
#include "MainDisplay.h"

MainDisplay mainDisplay;
LocoSelectDisplay locoSelectDisplay;
Interface *interface;

void handleDisplay(void)
{
  interface->Display();
  interface->DisplayMenu();
  global_lcd.display();
}

void handleKey(unsigned char key)
{
  if(key == 8 || key == 12 || key == 16)
  {
    interface->ProcessMenuKey(key);
    return;
  }
  
  interface->ProcessKey(key);
}

void handleRepeatKey(unsigned char key)
{
  interface->ProcessRepeatKey(key);
}

/***********************************************************************/
unsigned char key, old_key;

void setup() {
  global_lcd.init();

  // turn all the pixels on (a handy test)
  //global_lcd.command(PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYALLON);
  //delay(500);
  // back to normal
  global_lcd.command(PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL);

  // show splashscreen
  global_lcd.display();
  
  //draw a Railstars logo
//  global_lcd.setCursor(0, 0);
//  global_lcd.drawProgmemBitmap(railstars_logo, RAILSTARS_WIDTH, RAILSTARS_HEIGHT);

//  Serial.begin(115200);
//  Serial.println("RAILSTARS THROTTLEX");

  IBridge_GPIO_Config();
  link.initialize();
  
  locoSelectDisplay.init((OLCB_Link*)&link);
  
  while(!locoSelectDisplay.ready())
    link.update();

  global_state = DISP_LOCO_SELECT;
  interface = &locoSelectDisplay;

  key = old_key = 0;

  global_lcd.clear();
  handleDisplay();
}

void loop() { 
  link.update();

  handleDisplay();

  key = IBridge_Read_Key();
  if(key)
  {
    if(key != old_key)
    {
      handleKey(key);
    }
    else
    {
      handleRepeatKey(key);
    }
  }
    

  //update the state
  if(global_state == DISP_MAIN)
    interface = &mainDisplay;
  else if(global_state == DISP_LOCO_SELECT)
    interface = &locoSelectDisplay;

  if(key != old_key)
  {
    delay(20);
  }
  
  old_key = key;
}


/* EOF - TempSensor.pde */

