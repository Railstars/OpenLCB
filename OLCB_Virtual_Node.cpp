#include "OLCB_Virtual_Node.h"

void OLCB_Virtual_Node::update(void)
{
    if(!_initialized)
        _initialized = _link->addVNode(NID);
}


void OLCB_Virtual_Node::setNID(OLCB_NodeID *newNID)
{
  //Only allocate memory for a new NID the first time this gets called!
  if(!NID || (NID == _link->getNodeID())) //don't want to over-write the link's ID!
  {
#if defined(__arm__)
    NID = new OLCB_NodeID;
#elif defined(__AVR__)
    NID = (OLCB_NodeID*)malloc(sizeof(OLCB_NodeID));
#endif
  }
  memcpy(NID,newNID, sizeof(OLCB_NodeID));
  if(_link)
  {
      _initialized = _link->addVNode(NID);
  }
}

//this really should only be called once.
void OLCB_Virtual_Node::setLink(OLCB_Link *newLink)
{
  _link = newLink;
  if(!NID) //if no NID has been allocated, just grab the link's
    NID = _link->getNodeID();
  _link->addHandler(this);
}
