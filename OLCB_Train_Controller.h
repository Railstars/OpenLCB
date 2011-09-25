#ifndef __OLCB_TRAIN_CONTROLLER_H__
#define __OLCB_TRAIN_CONTROLLER_H__

#include "OLCB_Datagram.h"
#include "OLCB_Link.h"
#include "float16.h"

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

/*
class OLCB_Train_Controller
{
  public:
//  	void Train_Controller_create(void);
	void Train_Controller_initialize(void);
	void Train_Controller_update(void);
	bool Train_Controller_processDatagram(OLCB_Datagram *datagram);
//   	void Train_Controller_datagramResult(bool accepted, uint16_t errorcode);

	bool Train_Controller_isAttached(OLCB_NodeID *node);
   	
  private:
	  uint8_t Train_Controller_DCCSpeedToNotch(uint8_t dccspeed);
	  uint8_t Train_Controller_metersPerSecondToDCCSpeed(float mps);
	  
	  bool handleAttachDatagram(OLCB_Datagram *datagram) {return false;}
	  bool handleReleaseDatagram(OLCB_Datagram *datagram) {return false;}
	  bool handleSetSpeedDatagram(OLCB_Datagram *datagram);
	  bool handleGetSpeedDatagram(OLCB_Datagram *datagram) {return false;}
	  bool handleSetFXDatagram(OLCB_Datagram *datagram) {return false;}
	  bool handleGetFXDatagram(OLCB_Datagram *datagram) {return false;}
  
  
  //helpers
    uint32_t Train_Controller_timer;
    DCCPacketScheduler *DCC_Controller;
//    OLCB_Datagram *Train_Controller_txDatagramBuffer;
//    OLCB_Datagram *Train_Controller_rxDatagramBuffer;
//    OLCB_Link *Train_Controller_link;

  //configuration
    uint8_t Train_Controller_speed_steps;
    uint8_t Train_Controller_dcc_address;
    uint8_t Train_Controller_speed_curve[128];
    float Train_Controller_full_voltage_speed; //yuck! good thing we don't need it very much.

  //state infor
  	int8_t Train_Controller_speed; //in speedsteps; signed for direction
    uint32_t Train_Controller_FX; //bitfield of 32 FX
    OLCB_NodeID *Train_Controller_controllers[NUM_SIMULTANEOUS_CONTROLLERS];
};
*/

#endif
