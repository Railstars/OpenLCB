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

#include "Throttle.h"

void Throttle::init(void)
{
  _speed = 0;
  _direction = FORWARD;
  for(int i = 0; i < NUM_FUNCS; ++i)
    _functions[i] = 0;
  _address = 0; //no address
  _attached = false;
  _set_function = false;
}

void Throttle::update(void)
{
  //If address != 0 and attached = false, that means we are attempting to attach
  //to a loco. Give it another go.
  if(_new_address && !_attached && (_state==IDLE))
  {
    Serial.print("Attempting to attach to address ");
    Serial.println(_new_address,DEC);
    _dg.data[0] = DATAGRAM_MOTIVE;
    _dg.data[1] = DATAGRAM_MOTIVE_ATTACH; //attach
    _dg.length = 2;
    Serial.println("Making state ATTACHING in update()");
    _state = ATTACHING;
    sendDatagram(&_dg);
  }
  
  else if((_new_speed != _speed) || (_new_direction != _direction)) //if the speed has changed
  {
    _dg.data[0] = DATAGRAM_MOTIVE;
    _dg.data[1] = DATAGRAM_MOTIVE_SETSPEED; //set speed
    if(_new_direction == FORWARD) //forward
      _dg.data[2] = OLCB_FORWARD;
    else
      _dg.data[2] = OLCB_REVERSE;
    _dg.data[3] = _new_speed; //in percent throttle
    _dg.length = 4;
    _state = SETTING_SPEED;
    sendDatagram(&_dg);
  }
  
  else if(_set_function) //if a function has changed
  {
    _dg.data[0] = DATAGRAM_MOTIVE;
    _dg.data[1] = DATAGRAM_MOTIVE_SETFUNCTION; //set function
    _dg.data[2] = _new_function_ID+1;
    _dg.data[3] = _new_function_val; //in percent throttle
    _dg.length = 4;
    _state = SETTING_FUNCTION;
    _set_function = false;
    sendDatagram(&_dg);
  }
  
  OLCB_Datagram_Handler::update();
}


void Throttle::datagramResult(bool accepted, uint16_t errorcode)
{
/*   Serial.print("The datagram was ");
   if(!accepted)
     Serial.print("not ");
   Serial.println("accepted.");
   if(!accepted)
   {
     Serial.print("   The reason: ");
     Serial.println(errorcode,HEX);
  }
  Serial.print("And the state was:");
  switch(_state)
  {
      case SETTING_SPEED:
        Serial.println("SETTING_SPEED");
        break;
      case SETTING_FUNCTION:
        Serial.println("SETTING_FUNCTION");
         break;
      case ATTACHING:
        Serial.println("ATTACHING");
         break;
      case RELEASING:
        Serial.println("RELEASING (should never happen!?)");
        break;
       case IDLE:
         Serial.println("IDLE");
         break;
  }*/
    
  if(_state == SETTING_SPEED)
  {
    if(accepted)
    {
      _speed = _new_speed;
      _direction = _new_direction;
//      Serial.print("Speed is now set to ");
//      Serial.println(_new_speed, DEC);
//      Serial.print(" and direction to ");
//      if(_new_direction == FORWARD)
//        Serial.println("FORWARD");
//      else
//        Serial.println("REVERSE");
    }
    else
    {
      _new_speed = _speed; //reset new_speed request
      _new_direction = _direction;
    }
    _state = IDLE;
  }
  
  else if(_state == SETTING_FUNCTION)
  {
    if(accepted)
    {
      _functions[_new_function_ID] = _new_function_val;
//      Serial.print("Function ");
//      Serial.print(_new_function_ID, DEC);
//      Serial.print(" is now set to ");
//      Serial.print(_new_function_val, DEC);
    }
    _state = IDLE;
  }
  
  else if (_state == ATTACHING)
  {
    if(!accepted)
    {
      //reset
      _state = IDLE;
      _attached = false;
      _address = 0; 
      _new_address = 0;
    }
  }
  
  else if (_state == RELEASING)
  {
    if(accepted)
    {
      //reset
      _state = IDLE;
      _attached = false;
      _address = 0;
    }
    else
    {
      //keep trying!
      release();
    }
  }
}

bool Throttle::processDatagram(void)
{
  if((_rxDatagramBuffer->data[0] == DATAGRAM_MOTIVE) && (_rxDatagramBuffer->data[1] == DATAGRAM_MOTIVE_ATTACHED))
  {
//    Serial.println("Received Datagram ATTACHED");
//    Serial.print(" from ");
//    Serial.println(_rxDatagramBuffer->source.alias, DEC);
//    Serial.println((_rxDatagramBuffer->source.val[4] << 8) & (_rxDatagramBuffer->source.val[4]), DEC);
//    if(_new_address == (_rxDatagramBuffer->source.val[4] << 8) & (_rxDatagramBuffer->source.val[4])) //TODO WIll this cause problems? Ans: YES because source only contains an alias on CAN
//    {
      _state = IDLE;
      _attached = true;
      _address = _new_address;
      _speed = _new_speed = 0;
      _direction = _new_direction = FORWARD;
      for(int i = 0; i < NUM_FUNCS; ++i)
        _functions[i] = 0; //start with only headlights on
      _new_function_ID = 0;
      _new_function_val = true;
      _set_function = true; //get those headlights going!
//      Serial.println("ACKing");    
      return true;
//    }
  }
  else if((_rxDatagramBuffer->data[0] == DATAGRAM_MOTIVE) && (_rxDatagramBuffer->data[1] == DATAGRAM_MOTIVE_RELEASED) && (_state == RELEASING))
  {
//    Serial.println("Recevied Datagram RELEASED");
    _state = IDLE;
    _address = 0;
    //don't touch _new_address, as it might have something important in it!!!
    _attached = false;
    return true; //whatever, just ACK it.
  }
  
  else if((_rxDatagramBuffer->data[0] == DATAGRAM_MOTIVE) && (_rxDatagramBuffer->data[1] == DATAGRAM_MOTIVE_ATTACH_DENIED))
  {
    _state = IDLE;
    _address = _new_address = 0; 
    _attached = false;
    return true;
  }

//  Serial.println("NAKing datagram:");
//  Serial.print("len ");
//  Serial.println(_rxDatagramBuffer->length, DEC);
//  for(int i = 0; i < _rxDatagramBuffer->length; ++i)
//    Serial.println(_rxDatagramBuffer->data[i], HEX);
//  Serial.println("NAKing datagram");
  return false;
}

void Throttle::setSpeed(unsigned short speed, boolean direction)
{
  _new_speed = speed;
  _new_direction = direction;
}

void Throttle::setFunction(byte funcID, boolean on)
{
  _new_function_ID = funcID;
  _new_function_val = on;
  _set_function = true;
}

void Throttle::setAddress(unsigned int address)
{
  //first, if address!=0, release existing locomotive
  if(_address)
  {
    release();
  }
  else
  {
    _state = IDLE;
  }
//  Serial.print("setAddress to: ");
//  Serial.print(address, DEC);
  _new_address = address;
  _address = 0; //not yet!
    //now, set new address
//    _state = IDLE; //not really, but this will do.
  _dg.destination.set(6,1,0,0,(_new_address&0xFF00) >> 8,(_new_address&0x00FF)); //set the locomotive as the destination address for future comms...
}

void Throttle::release(void)
{
  _dg.data[0] = DATAGRAM_MOTIVE;
  _dg.data[1] = DATAGRAM_MOTIVE_RELEASE; //set function
  _dg.length = 2;
  _state = RELEASING; //This is to force the throttle to wait for a "Released" datagram;
  _attached = false;
  _new_address = 0; //to avoid thinking that we're going to attach to something when we're not!
  sendDatagram(&_dg);
}
