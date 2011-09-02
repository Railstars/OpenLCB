#ifndef __OLCB_VIRTUAL_NODE_H__
#define __OLCB_VIRTUAL_NODE_H__

#if defined(__AVR__)
#include <stdlib.h>
#endif
#include <string.h>
#include "OLCB_NodeID.h"
#include "OLCB_Buffer.h"
#include "OLCB_Link.h"

class OLCB_Link;

class OLCB_Virtual_Node
{
 public:
  OLCB_Virtual_Node() : NID(0), _link(0), next(0), _initialized(false)
    {return;}

  virtual void init(void) {return;}

  virtual void update(void);
  virtual bool handleFrame(OLCB_Buffer *buffer) {return false;}  
  virtual bool verifyNID(OLCB_NodeID *nid)
  {
      if((*nid) == (*NID))
      {
          return true;
      }
      return false;
  }
  virtual void setNID(OLCB_NodeID *newNID);
  virtual void setLink(OLCB_Link *newLink);
  
  OLCB_Virtual_Node *next;  
  OLCB_NodeID *NID;
  
 protected:
  OLCB_Link *_link;
  uint16_t _sendEvent;
  bool _initialized;
};

//class OLCB_VirtualNode : protected OLCP_Handler
//{
//}

#endif
