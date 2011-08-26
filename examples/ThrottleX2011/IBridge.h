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

#ifndef __IBRIDGE_H__
#define __IBRIDGE_H__

#include <avr/pgmspace.h>
#include <WProgram.h>

//Define the hardware operation function
void IBridge_GPIO_Config(void);

void IBridge_init(void);

unsigned char IBridge_Read_Key(void);

#endif
