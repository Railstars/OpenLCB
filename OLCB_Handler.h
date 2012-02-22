#ifndef __OLCB_HANDLER_H__
#define __OLCB_HANDLER_H__

#include "OLCB_Link.h"
#include "OLCB_NodeID.h"
#include "OLCB_Buffer.h"

class OLCB_Link;

class OLCB_Handler
{
  public:
	
	void create(OLCB_Link *link, OLCB_NodeID *nid)
	{
		_link = link;
		NID = nid;
	}
	
	
  	virtual bool handleMessage(OLCB_Buffer *buffer)
  	{
		return false;
	}
	
  	OLCB_Link *_link;
	OLCB_NodeID *NID;
};

#endif
