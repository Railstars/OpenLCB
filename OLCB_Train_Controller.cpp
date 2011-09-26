#include "OLCB_Train_Controller.h"

void OLCB_Train_Controller::Train_Controller_create(OLCB_Datagram_Handler *h)
{
	Train_Controller_DatagramHandler = h;
}

void OLCB_Train_Controller::Train_Controller_initialize(void)
{
  //configuration
	Train_Controller_speedConv = MSTOMPH;
	
  //state infor
  	Train_Controller_needs_update = false;
  	Train_Controller_speed = 0; //in speedsteps; signed for direction
    Train_Controller_FX - 0; //bitfield of 32 FX
    Train_Controller_train = NULL;
}

void OLCB_Train_Controller::Train_Controller_update(void)
{
	if(Train_Controller_needs_update)
	{
		//send speed and FX
		OLCB_Datagram d;
		d.destination.copy(Train_Controller_train);
		d.length = 4;
		d.data[0] = DATAGRAM_MOTIVE;
		d.data[1] = DATAGRAM_MOTIVE_SETSPEED;
		_float16_shape_type f16_val = float32_to_float16(Train_Controller_speed);
		d.data[2] = f16_val.words.msw;
		d.data[3] = f16_val.words.lsw;
		Train_Controller_needs_update = !Train_Controller_DatagramHandler->sendDatagram(&d); //OK to send pointer, sendDatagram copies the contents;
	}
}

bool OLCB_Train_Controller::Train_Controller_processDatagram(OLCB_Datagram *datagram)
{
	//we don't handle any datagrams, we just send them
	return false;
}
