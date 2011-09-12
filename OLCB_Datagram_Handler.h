#ifndef __OLCB_DATAGRAM_HANDLER__
#define __OLCB_DATAGRAM_HANDLER__

#include <string.h>
#include "WProgram.h"
#include "OLCB_Link.h"
#include "OLCB_Virtual_Node.h"
#include "OLCB_Datagram.h"

#define DATAGRAM_ACK_TIMEOUT 5000
#define DATAGRAM_ERROR_ABORTED 0x1001
#define DATAGRAM_ERROR_ACK_TIMEOUT 0x1002

// A mix-in class for handling the datagram protocol.

class OLCB_Datagram_Handler : public OLCB_Virtual_Node
{
 public:
  OLCB_Datagram_Handler() : OLCB_Virtual_Node(), _rxDatagramBufferFree(true), _txDatagramBufferFree(true), _sentTime(0), _txFlag(false), _loc(0)
  {
#if defined(__arm__)
    _rxDatagramBuffer = new OLCB_Datagram;
    _txDatagramBuffer = new OLCB_Datagram;
#elif defined(__AVR__)
    _rxDatagramBuffer = (OLCB_Datagram*)malloc(sizeof(OLCB_Datagram));
    _txDatagramBuffer = (OLCB_Datagram*)malloc(sizeof(OLCB_Datagram));
#endif
  }
  
  void setLink(OLCB_Link *newLink);
  void setNID(OLCB_NodeID *newNID);
  
  virtual bool verifyNID(OLCB_NodeID *nid)
  {
    return OLCB_Virtual_Node::verifyNID(nid);
  }
  
  virtual bool handleFrame(OLCB_Buffer *frame);
  
  virtual void update(void);
  
  //This method, and maybe the next, must be fleshed out in a derived class to be of much use.
  virtual bool processDatagram(void) {return false;}
  virtual void datagramResult(bool accepted, uint16_t errorcode) {return;}
  
  bool sendDatagram(OLCB_Datagram *datagram);
  bool isDatagramSent(void);
  
 protected:
 
 //TODO condense all these bools into a bitfield
  uint32_t _sentTime;
//  bool _initialized;
  bool _txFlag;
  bool _rxDatagramBufferFree;
  bool _txDatagramBufferFree;
  uint8_t _loc;
  
  OLCB_Datagram *_rxDatagramBuffer;
  OLCB_Datagram *_txDatagramBuffer;
};

#endif
