#ifndef __OLCB_DCC_TRAIN_H__
#define __OLCB_DCC_TRAIN_H__

#include "OLCB_Virtual_Node.h"
#include "OLCB_Datagram_Handler.h"
#include "OLCB_Datagram.h"
#include "DCCPacketScheduler.h"

#define DATAGRAM_MOTIVE             	    0x30
#define DATAGRAM_MOTIVE_SETSPEED			0x01
#define DATAGRAM_MOTIVE_GETSPEED			0x02
#define DATAGRAM_MOTIVE_REPORTSPEED			0x03

#define DATAGRAM_MOTIVE_SETFX				0x11
#define DATAGRAM_MOTIVE_GETFX				0x12
#define DATAGRAM_MOTIVE_REPORTFX			0x13

#define DATAGRAM_MOTIVE_ATTACH      	    0x21
#define DATAGRAM_MOTIVE_ATTACHED			0x22
#define DATAGRAM_MOTIVE_ATTACH_DENIED		0x33
#define DATAGRAM_MOTIVE_RELEASE				0x24
#define DATAGRAM_MOTIVE_RELEASED			0x25

#define SPEED_STEPS_14	14
#define SPEED_STEPS_28	28
#define SPEED_STEPS_128	127


#define NUM_SIMULTANEOUS_CONTROLLERS 2

/* data type for storing float16s */

typedef union
{
	struct
	{
		uint16_t sign : 1;
		uint16_t exponent : 5;
		uint16_t mantissa: 10;
	} number;
	struct
	{
		uint8_t msw;
		uint8_t lsw;
	} words;
} _float16_shape_type;

/***
Super states:
	inhibited/permitted
	
States in permitted:
	TODO
***/


static float float16_to_float32(_float16_shape_type f_val);


class OLCB_DCC_Train : public OLCB_Virtual_Node, public OLCB_Datagram_Handler
{
  public:
	void initialize(void);
	void update(void);
	bool processDatagram(void);
   	void datagramResult(bool accepted, uint16_t errorcode);
   	
  private:
    uint32_t _timer;
    uint8_t _speed_steps;
    uint8_t _speed_curve[128];
    uint8_t _dcc_address;
  	int8_t _speed; //in speedsteps; signed for direction
    OLCB_NodeID *_controllers[NUM_SIMULTANEOUS_CONTROLLERS];
    uint32_t _FX;
};


#endif
