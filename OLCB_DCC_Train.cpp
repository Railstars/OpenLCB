#include "OLCB_DCC_Train.h"

void OLCB_DCC_Train::initialize(void)
{
	_timer = millis();
	_speedsteps = SPEED_STEPS_28;
	_dcc_address = 0;
	_speed = 0;
	_FX = 0;
	uint8_t i;
	for(i = 0; i < NUM_SIMULTANEOUS_CONTROLLERS; ++i)
	{
		_controllers[i] = NULL;
	}
	for(i = 0; i < 28; ++i)
	{
		_speed_curve[i] = map(i, 0, 27, 0, 255);
	}
}

void OLCB_DCC_Train::update(void)
{
	if(isPermitted())
	{
		//see if we need to send out any periodic updates
		uint32_t time = millis();
		if( (time - _timer) >= 60000 ) //one minute
		{
			_timer = time;
			DCC_Controller->setSpeed(_dcc_address, _speed, _speedsteps);
		}
	}
}

bool OLCB_DCC_Train::isAttached(OLCB_NodeID *node)
{
	for(uint8_t i = 0; i < NUM_SIMULTANEOUS_CONTROLLERS; ++i)
	{
		if( _controllers[i] && (*(_controllers[i]) == *NID)) )
			return true;
	}
	return false;
}

bool OLCB_DCC_Train::processDatagram(void)
{
	//check to see if it is for us first:
	if(_rxDatagramBuffer.destination != *NID)
		return false;
		
	//from this point on, it is for us!
	//make sure that it is a train control datagram, and handle that accordingly
	if(_rxDatagramBuffer->length && _rxDatagramBuffer->data[0] == DATAGRAM_MOTIVE) //is this a datagram for loco control?
	{
		switch(_rxDatagramBuffer->data[1])
		{
		case DATAGRAM_MOTIVE_ATTACH:
			return attachDatagram();
		case DATAGRAM_MOTIVE_RELEASE:
			return releaseDatagram();
		case DATAGRAM_MOTIVE_SETSPEED:
			return setSpeedDatagram();
		case DATAGRAM_MOTIVE_GETSPEED:
			return getSpeedDatagram();
		case DATAGRAM_MOTIVE_SETFUNCTION:
			return setFunctionDatagram();
		case DATAGRAM_MOTIVE_GETFUNCTION:
			return getFunctionDatagram();
		}
	}
	return false;
}

//borrowed from
// http://www.ogre3d.org/docs/api/html/OgreBitwise_8h_source.html
static float float16_to_float32(_float16_shape_type f_val)
{
	uint16_t exponent = f_val.number.exponent;
	uint16_t sign = f_val.number.sign;
	uint32_t mantissa = f_val.number.mantissa;
	
	union { float f; uint32 i; } v;
	
	if(exponent == 0)
	{
		if(mantissa == 0) //zero!
		{
			v.i = sign << 31;
			return v.f;
		}
		else	//sub-normal number, renormalize it (!?)
		{
			while (!(mantissa & 0x00000400))
			{
				mantissa <<= 1;
				exponent -=  1;
			}
			exponent += 1;
			mantissa &= ~0x00000400;
		}
	}
	
	else if(exponent == 0x1F) //NaN! treat as zero for our purposes
	{
		if (mantissa == 0) // Inf
		{
			v.i = (sign << 31) | 0x7f800000;
			return v.f;
		}
		else // NaN
		{
			v.i = (sign << 31) | 0x7f800000 | (mantissa << 13);
			return v.f;
		}
	}
	
	//else a normal number; fall through from sub-normal case.
	exponent = exponent + (127 - 15);
    mantissa = mantissa << 13;
    
    v.i = (sign << 31) | (exponent << 23) | mantissa;
    return v.f;
}

uint8_t OLCB_DCC_Train::metersPerSecondToDCCSpeed(float mps)
{
	//notice that the input here should be strictly positive!!
	
	//We need to know the top speed of the locomotive;
	//we assume that the physical speed of the locomotive is a linear function of the voltage applied to the motor. Note that this is not generally true, but is close enough for our purposes. For now. TODO
	uint8_t dccspeed; //the speed in the range 0...255, with 0 = no voltage and 255 = max voltage.
	
	//I hate the float division, but it is necessary :( :( :(
	
	return mps * 0xFF / _full_voltage_speed;
}

uint8_t OLCB_DCC_Train::DCCSpeedToNotch(uint8_t dccspeed)
{
	//find the entry in the speed table that most closely matches the desired speed
	//how to alias? Here, just find the speed that is closest without exceeding the indicated speed.
	//at 14 steps, we have: 0 = estop, 1 = stop, 2 = first move, ..., 14 = top speed.
	//at 28 steps, 0 = estop, 1 = stop, ..., 14 = top speed
	//at 128 steps, 0 = estop, 1 = stop, ..., 127 = top speed (so, not really 128 steps at all!)
	
	//the speed curve is laid out as such (example is 14 speed steps):
	/*
	_speed_curve[0] = 0 // stop always = 0!
	_speed_curve[1] = min // first movement speed; set in CV TODO
	...
	_speed_curve[8] = mid // middle movement speed; set in CV TODO
	...
	_speed_curve[14] = max // top speed, set by CV TODO
	*/
	
	uint8_t notch = 1; //default: stop
	for(uint8_t i = _speed_steps; i >= 0; --i)
	{
		if(_speed_curve[i] <= dccspeed)
		{
			notch = i;
			break;
		}
	}
	
	return notch;
}

bool OLCB_DCC_Train::setSpeedDatagram(void)
{
	if(isAttached(&(_rxDatagramBuffer.source)))
	{
		//incoming speed is a signed float16
		//we store it as a signed 8-bit int, with -1/1 = stop (and 0 = estop), and -127/127 = max speed
		//notice that it is not enough to get the raw integral value of the float16, but we must scale it, because the DCC speed steps != absolute speed, but throttle notches. So we have to account for what motor speed each notch represents, and choose the appropriate notch..yuck!
		//finally, unhandled here, users can set a custom scale value. TODO
		_float16_shape_type f_val;
		f_val.words.msw = _rxDatagramBuffer.data[1];
		f_val.words.lsw = _rxDatagramBuffer.data[2];
		float new_speed = float16_to_float32(f_val);
		int8_t dir = 1; //forward
		if(new_speed < 0)
		{
			dir = -1; //reverse
			new_speed *= -1; //make it positive
		}
		uint8_t speed = dir * DCCSpeedToNotch(metersPerSecondToDCCSpeed(new_speed));
		
		if(DCC_Controller->setSpeed(_dcc_address, speed, _speed_steps))
		{
			_speed = speed;
			return true;
		}
		else
		{
			//need to send a message that the datagram should be repeated!
			return true;
		}
	}
	
	return false;
}
