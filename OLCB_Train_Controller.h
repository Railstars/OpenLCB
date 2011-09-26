#ifndef __OLCB_TRAIN_CONTROLLER_H__
#define __OLCB_TRAIN_CONTROLLER_H__

#include "OLCB_Datagram.h"
#include "OLCB_Datagram_Handler.h"
#include "OLCB_Link.h"
#include "float16.h"

#define DATAGRAM_MOTIVE             	    0x30
#define DATAGRAM_MOTIVE_SETSPEED			0x01
#define DATAGRAM_MOTIVE_GETSPEED			0x03
#define DATAGRAM_MOTIVE_REPORTSPEED			0x02

#define DATAGRAM_MOTIVE_SETFX				0x11
#define DATAGRAM_MOTIVE_GETFX				0x13
#define DATAGRAM_MOTIVE_REPORTFX			0x12

#define DATAGRAM_MOTIVE_ATTACH      	    0x21
#define DATAGRAM_MOTIVE_ATTACHED			0x22
#define DATAGRAM_MOTIVE_ATTACH_DENIED		0x33
#define DATAGRAM_MOTIVE_RELEASE				0x24
#define DATAGRAM_MOTIVE_RELEASED			0x25

#define MSTOKMH	3.6
#define MSTOMPH 2.23693629


class OLCB_Train_Controller
{
  public:
  
  	void Train_Controller_create(OLCB_Datagram_Handler *h);
	void Train_Controller_initialize(void);
	void Train_Controller_update(void);
	bool Train_Controller_processDatagram(OLCB_Datagram *datagram);
//   	void Train_Controller_datagramResult(bool accepted, uint16_t errorcode);

	void Train_Controller_attach(OLCB_NodeID *nid) {Train_Controller_train = nid;}
  	void Train_Controller_setSpeed_m_s(float new_speed) {Train_Controller_speed = new_speed; Train_Controller_needs_update = true;}   	
  private:
	  bool handleAttachedDatagram(OLCB_Datagram *datagram) {return true;}
	  bool handleReleaseedDatagram(OLCB_Datagram *datagram) {return true;}

  //configuration
	float Train_Controller_speedConv;
	
  //state infor
  	OLCB_Datagram_Handler *Train_Controller_DatagramHandler;
  	bool Train_Controller_needs_update;
  	float Train_Controller_speed; //in speedsteps; signed for direction
    uint32_t Train_Controller_FX; //bitfield of 32 FX
    OLCB_NodeID *Train_Controller_train;
};

#endif
