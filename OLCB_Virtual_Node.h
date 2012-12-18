#ifndef __OLCB_VIRTUAL_NODE_H__
#define __OLCB_VIRTUAL_NODE_H__

#if defined(__AVR__)
#include <stdlib.h>
#endif
#include <string.h>

#include "OLCB_Link.h"
#include "OLCB_NodeID.h"
#include "OLCB_Buffer.h"
class OLCB_Link;



class OLCB_Virtual_Node
{
    public:

virtual void update(void) {}

bool isPermitted(void)
{
    if(NID)
    {
        return NID->initialized;
    }
    else
    {
        return false;
    }
}

void create(OLCB_Link *link, OLCB_NodeID *nid)
{
    _link = link;
    NID = nid;
}


virtual bool handleMessage(OLCB_Buffer *buffer)
{
    return false;
}

//clears any pending communications between this node and nodeid
virtual void clearBuffer(OLCB_NodeID *nodeid)
{
    //Serial.println("nothing");
    return;
}
OLCB_Virtual_Node *next;
OLCB_Link *_link;
OLCB_NodeID *NID;
};

#endif
