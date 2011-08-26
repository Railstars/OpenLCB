#include "OLCB_Virtual_Node.h"

void OLCB_Virtual_Node::setNID(OLCB_NodeID *newNID)
{
  //Only allocate memory for a new NID the first time this gets called!
  if(!NID || (NID == _link->getNodeID())) //don't want to over-write the link's ID!
  {
    NID = (OLCB_NodeID*)malloc(sizeof(OLCB_NodeID));
  }
  memcpy(NID,newNID, sizeof(OLCB_NodeID));
}

//this really should only be called once.
void OLCB_Virtual_Node::setLink(OLCB_Link *newLink)
{
  _link = newLink;
  if(!NID) //if no NID has been allocated, just grab the link's
    NID = _link->getNodeID();
  _link->addHandler(this);
}
