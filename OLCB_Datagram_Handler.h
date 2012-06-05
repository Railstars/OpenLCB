#ifndef __OLCB_DATAGRAM_HANDLER__
#define __OLCB_DATAGRAM_HANDLER__

#include <string.h>
#include "Arduino.h"
#include "OLCB_Link.h"
#include "OLCB_Virtual_Node.h"
#include "OLCB_Datagram.h"

#define DATAGRAM_ERROR_OK 0xFFFF //assume this won't get used as an error code. HACK!

#define DATAGRAM_ACK_TIMEOUT 5000
#define DATAGRAM_ERROR_ABORTED 0x1001
#define DATAGRAM_ERROR_ACK_TIMEOUT 0x1002

// An abstract class for handling the datagram protocol.

class OLCB_Datagram_Handler : public OLCB_Virtual_Node
{
 public:
  OLCB_Datagram_Handler() : _rxDatagramBufferFree(true), _txDatagramBufferFree(true), _sentTime(0), _txFlag(false), _loc(0)
  {
#if defined(__arm__)
    _rxDatagramBuffer = new OLCB_Datagram;
    _txDatagramBuffer = new OLCB_Datagram;
#elif defined(__AVR__)
    _rxDatagramBuffer = (OLCB_Datagram*)malloc(sizeof(OLCB_Datagram));
    _txDatagramBuffer = (OLCB_Datagram*)malloc(sizeof(OLCB_Datagram));
#endif
  }
    
  bool handleMessage(OLCB_Buffer *frame);
  
  virtual void update(void);
  
  //This method, and maybe the next, must be fleshed out in a derived class to be of much use.
  virtual uint16_t processDatagram(void) {return false;}
  virtual void datagramResult(bool accepted, uint16_t errorcode) {return;}
  
  bool sendDatagram(OLCB_Datagram *datagram);
  bool isDatagramSent(void);
  
  //clears any pending communications between this node and nodeid
  void clearBuffer(OLCB_NodeID *nodeid);

 protected:
 
 //TODO condense all these bools into a bitfield
//  bool _initialized;
  bool _rxDatagramBufferFree;
  bool _txDatagramBufferFree;
  uint32_t _sentTime;
  bool _txFlag;
  bool _ackFlag;
  uint16_t _ackReason;
  uint8_t _loc;
  
  //TODO MAKE THESE NOT POINTERS
  OLCB_Datagram *_rxDatagramBuffer;
  OLCB_Datagram *_txDatagramBuffer;
};

#endif
