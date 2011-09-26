#include "OLCB_DCC_Train.h"
#include "float16.h"

void OLCB_DCC_Train::DCC_Train_create(DCCPacketScheduler *controller)
{
	DCC_Controller = controller;
}

void OLCB_DCC_Train::DCC_Train_initialize() //OLCB_Datagram *txBuf, OLCB_Datagram *rxBuf, OLCB_Link *link)
{
//	DCC_Train_txDatagramBuffer = txBuf;
//	DCC_Traindatagram = rxBuf;
//	DCC_Train_link = link;
	DCC_Train_timer = millis();
	DCC_Train_speed_steps = SPEED_STEPS_28;
	DCC_Train_dcc_address = 0;
	DCC_Train_speed = 0;
	DCC_Train_FX = 0;
	uint8_t i;
	for(i = 0; i < NUM_SIMULTANEOUS_CONTROLLERS; ++i)
	{
		DCC_Train_controllers[i] = NULL;
	}
	for(i = 0; i < 28; ++i)
	{
		DCC_Train_speed_curve[i] = map(i, 0, 27, 0, 255);
	}
}

void OLCB_DCC_Train::DCC_Train_update(void)
{
	//see if we need to send out any periodic updates
	uint32_t time = millis();
	if( (time - DCC_Train_timer) >= 60000 ) //one minute
	{
		DCC_Train_timer = time;
		DCC_Controller->setSpeed(DCC_Train_dcc_address, DCC_Train_speed, DCC_Train_speed_steps);
	}
}

bool OLCB_DCC_Train::DCC_Train_isAttached(OLCB_NodeID *node)
{
	for(uint8_t i = 0; i < NUM_SIMULTANEOUS_CONTROLLERS; ++i)
	{
		if( DCC_Train_controllers[i] && (*(DCC_Train_controllers[i]) == *node) )
			return true;
	}
	return false;
}

bool OLCB_DCC_Train::DCC_Train_processDatagram(OLCB_Datagram *datagram)
{		
	//from this point on, it is for us!
	//make sure that it is a train control datagram, and handle that accordingly
	if(datagram->length && (datagram->data[0] == DATAGRAM_MOTIVE) )//is this a datagram for loco control?
	{
		switch(datagram->data[1])
		{
		case DATAGRAM_MOTIVE_ATTACH:
			return handleAttachDatagram(datagram);
		case DATAGRAM_MOTIVE_RELEASE:
			return handleReleaseDatagram(datagram);
		case DATAGRAM_MOTIVE_SETSPEED:
			return handleSetSpeedDatagram(datagram);
		case DATAGRAM_MOTIVE_GETSPEED:
			return handleGetSpeedDatagram(datagram);
		case DATAGRAM_MOTIVE_SETFX:
			return handleSetFXDatagram(datagram);
		case DATAGRAM_MOTIVE_GETFX:
			return handleGetFXDatagram(datagram);
		}
	}
	return false;
}

uint8_t OLCB_DCC_Train::DCC_Train_metersPerSecondToDCCSpeed(float mps)
{
	//notice that the input here should be strictly positive!!
	
	//We need to know the top speed of the locomotive;
	//we assume that the physical speed of the locomotive is a linear function of the voltage applied to the motor. Note that this is not generally true, but is close enough for our purposes. For now. TODO
	
	//I hate the float division, but it is necessary :( :( :(
	
	return mps * 0xFF / DCC_Train_full_voltage_speed;
}

uint8_t OLCB_DCC_Train::DCC_Train_DCCSpeedToNotch(uint8_t dccspeed)
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
	for(uint8_t i = DCC_Train_speed_steps; i >= 0; --i)
	{
		if(DCC_Train_speed_curve[i] <= dccspeed)
		{
			notch = i;
			break;
		}
	}
	
	return notch;
}

bool OLCB_DCC_Train::handleSetSpeedDatagram(OLCB_Datagram *datagram)
{
	if(DCC_Train_isAttached(&(datagram->source)))
	{
		//incoming speed is a signed float16
		//we store it as a signed 8-bit int, with -1/1 = stop (and 0 = estop), and -127/127 = max speed
		//notice that it is not enough to get the raw integral value of the float16, but we must scale it, because the DCC speed steps != absolute speed, but throttle notches. So we have to account for what motor speed each notch represents, and choose the appropriate notch..yuck!
		//finally, unhandled here, users can set a custom scale value. TODO
		_float16_shape_type f_val;
		f_val.words.msw = datagram->data[1];
		f_val.words.lsw = datagram->data[2];
		float new_speed;
		new_speed = float16_to_float32(f_val);
		int8_t dir = 1; //forward
		if(new_speed < 0)
		{
			dir = -1; //reverse
			new_speed *= -1; //make it positive
		}
		uint8_t speed = dir * DCC_Train_DCCSpeedToNotch(DCC_Train_metersPerSecondToDCCSpeed(new_speed));
		
		if(DCC_Controller->setSpeed(DCC_Train_dcc_address, speed, DCC_Train_speed_steps))
		{
			DCC_Train_speed = speed;
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
