#ifndef __OLCB_VIRTUAL_NODE_H__
#define __OLCB_VIRTUAL_NODE_H__

#if defined(__AVR__)
#include <stdlib.h>
#endif
#include <string.h>

#include "OLCB_Link.h"
#include "OLCB_NodeID.h"
#include "OLCB_Buffer.h"
#include "OLCB_Handler.h"
class OLCB_Link;



class OLCB_Virtual_Node : public OLCB_Handler
{
 public:

//  virtual void initialize()
//  {
//  	_link->addVNode(this);
//  }

  virtual void update(void) {}
  
  //must be overridden in derived class as such!
  /*setNID() { OLCB_Virtual_Node::setNID(), OLCB_Event_Handler::setNID(), etc}
  */
//  void setNID(OLCB_NodeID *newNID);
//  void setLink(OLCB_Link *newLink);


  virtual bool verifyNID(OLCB_NodeID *nid)
  {
      if( isPermitted() && ((*nid) == (*NID)) )
      {
          return true;
      }
      return false;
  }
  
  bool isPermitted(void)
  {
  	if(NID)
	  	return NID->initialized;
	else
		return false;
  }
  
  OLCB_Virtual_Node *next;  
 protected:

//  OLCB_NodeID *NID;
//  OLCB_Link *_link;
};

#endif
