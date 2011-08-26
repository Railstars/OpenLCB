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
#include "MainDisplay.h"

void MainDisplay::DisplayMenu(void)
{
  global_lcd.fillrect(0, 39, 84, 9, 1);
  global_lcd.drawstringinverse(0, 5, "LOCO ");
  global_lcd.drawline(26, 48, 26, 40, 0);
  //global_lcd.drawstringinverse(30, 5, "     ");
  //global_lcd.drawstringinverse(60, 5, "     ");
}

void MainDisplay::ProcessMenuKey(unsigned short key)
{
  if(key == 16) //go to loco select
  {
    global_state = DISP_LOCO_SELECT;
    global_lcd.clear();
    delay(50);
  }
}

void MainDisplay::Display(void)
{  
  // Print the loco
//  Serial.setCursor(0, 0);
  
  if(!global_throttle->isAttached()) //if the throttle has not attached to a loco, then it is either attaching, or has failed to attach.
  {
    global_lcd.clear();
    if(global_throttle->isAttaching())
    {
      global_lcd.drawstring(0, 1, "Attaching...");
    }
    else
    {
      global_lcd.drawstring(0, 1, "Attach Failed");
    }
    return;
  }

  // Print the address
  global_lcd.drawstring(0, 0, "Train: ");
  global_lcd.drawstring(40, 0, String(global_throttle->getAddress(), DEC));
  
  // Print the speed
  //Serial.setCursor(0, 1);
  global_lcd.drawstring(0,1,"Speed: ");
  unsigned short speed = global_throttle->getSpeed();
  unsigned char dir = '>';
  if(global_throttle->getDirection())
    dir = '<';
  if(speed == 100)
  {
    global_lcd.drawstring(30,1," ");
    global_lcd.drawchar(36,1,dir);
    global_lcd.drawstring(42,1,String(speed, DEC));
  }
  else if(speed >= 10)
  {
    global_lcd.drawstring(36,1,"  ");
    global_lcd.drawchar(42,1,dir);
    global_lcd.drawstring(48,1,String(speed, DEC));
  }
  else
  {
    global_lcd.drawstring(36,1,"   ");
    global_lcd.drawchar(48,1,dir);
    global_lcd.drawstring(54,1,String(speed, DEC));
  }
  global_lcd.drawstring(60,1,"%  ");
  
  global_lcd.fillrect(70, 0, 10, 38, 0);
  global_lcd.drawrect(70, 0, 10, 38, 1);
  global_lcd.fillrect(72, 37-map(speed,0,100,0,37), 6, map(speed,0,100,0,37), 1);

  // update the function display
  for(unsigned char i = 0; i < 5; ++i)
  {
     if(global_throttle->getFunction(i))
       global_lcd.drawchar((i+2)*8, 3, i+48);
     else
       global_lcd.drawchar((i+2)*8, 3, ' ');
  }
  for(unsigned char i = 5; i < 10; ++i)
  {
      if(global_throttle->getFunction(i))
       global_lcd.drawchar((i-3)*8, 4, i+48);
     else
       global_lcd.drawchar((i-3)*8, 4, ' ');
   }

}

void MainDisplay::ProcessKey(unsigned short key)
{
  // keypad layout:
  // G C 8 4
  // F B 7 3
  // E A 6 2
  // D 9 5 1
  unsigned short func = 0;
  unsigned short speed = 0;
  boolean direction = true;
//  Serial.print("Keypress! ");
//  Serial.println(key,HEX);
  switch(key)
  {
    //speed keys
  case 2: //decrease speed
//    Serial.println("speed down");
    speed = global_throttle->getSpeed();
    direction = global_throttle->getDirection();
    if(speed > 0) global_throttle->setSpeed(speed - 1, direction);
    return;
  case 3: //increase speed
//    Serial.println("speed up");
    speed = global_throttle->getSpeed();
    direction = global_throttle->getDirection();
    if(speed < 100) global_throttle->setSpeed(speed + 1, direction);
    return;
  case 4: //direction
//    Serial.println("reverse direction");
    speed = global_throttle->getSpeed();
    direction = global_throttle->getDirection();
    global_throttle->setSpeed(speed, !direction);
    return;

    //function keys:
    //
    // G C 8 4
    // F B 7 3
    // E A 6 2
    // D 9 5 1
    //
    // M M M D
    // 7 8 9 +
    // 4 5 6 -
    // 1 2 3 0

  case 1:
    func = 0;
    break;
  case 0xD:
    func = 1;
    break;
  case 9:
    func = 2;
    break;
  case 5:
    func = 3;
    break;
  case 0xE:
    func = 4;
    break;
  case 0xA:
    func = 5;
    break;
  case 6:
    func = 6;
    break;
  case 0xF:
    func = 7;
    break;
  case 0xB:
    func = 8;
    break;
  case 7:
    func = 9;
    break;
  default:  //error!
    return;
  }
  
  boolean func_val = global_throttle->getFunction(func);
//  if(!func_val)
//    Serial.println("function on");
//  else
//    Serial.println("function off");
  global_throttle->setFunction(func, !func_val);
}

void MainDisplay::ProcessRepeatKey(unsigned short key)
{
    // keypad layout:
  // G C 8 4
  // F B 7 3
  // E A 6 2
  // D 9 5 1
  unsigned short func = 0;
  unsigned short speed = 0;
  boolean direction = true;
//  Serial.print("Keypress! ");
//  Serial.println(key,HEX);
  switch(key)
  {
    //speed keys
  case 2: //decrease speed
//    Serial.println("speed down");
    speed = global_throttle->getSpeed();
    direction = global_throttle->getDirection();
    if(speed > 0) global_throttle->setSpeed(speed - 1, direction);
    return;
  case 3: //increase speed
//    Serial.println("speed up");
    speed = global_throttle->getSpeed();
    direction = global_throttle->getDirection();
    if(speed < 100) global_throttle->setSpeed(speed + 1, direction);
    return;
  }
}

