#ifndef __OLCB_CAN_LINK_H__
#define __OLCB_CAN_LINK_H__

#include <stdint.h>
#include <wprogram.h>
#include "OLCB_Link.h"
#include "OLCB_CAN_Buffer.h"
#include "OLCB_NodeID.h"
#include "OLCB_Event.h"
#include "OLCB_Datagram.h"
#include "OLCB_Stream.h"
#include "OLCB_Alias_Cache.h"
#include "OLCB_CAN_Alias_Helper.h"

// state machine definitions NOT USED apparently!?
#define STATE_INITIAL 0
#define STATE_WAIT_CONFIRM 10
#define STATE_ALIAS_ASSIGNED 20
#define STATE_INITIALIZED 30

#define NODE_ID_STATE_INACTIVE	0
#define NODE_ID_STATE_CID1		1
#define NODE_ID_STATE_CID2		2
#define NODE_ID_STATE_CID3		3
#define NODE_ID_STATE_CID4		4
#define NODE_ID_STATE_RID		5

class OLCB_CAN_Link_Helper;

class OLCB_CAN_Link : public OLCB_Link
{
 public:
  OLCB_CAN_Link(OLCB_NodeID *NID) : OLCB_Link(NID)
  {
//    Serial.print("OLCB_CAN_Link: ");
//    Serial.println((uint16_t)id, HEX);
    //_nodeIDToBeVerified.set(0,0,0,0,0,0);
  }
  
  bool initialize(void);
  
//  bool negotiateAlias(OLCB_NodeID *nid);
  void negotiateAliasesInQueue(void);
  
  bool handleTransportLevel(void);
  virtual void update(void);
    
  bool sendEvent(OLCB_Event *event) {return false;}
  
  uint8_t sendDatagramFragment(OLCB_Datagram *datagram, uint8_t start);
  bool ackDatagram(OLCB_NodeID *source, OLCB_NodeID *dest);
  bool nakDatagram(OLCB_NodeID *source, OLCB_NodeID *dest, int reason);
  
  bool sendStream(OLCB_Stream *stream) {return false;}
  //Not sure that this is how streams should work at all!
  
  bool sendVerifiedNID(OLCB_NodeID *nid);
  
  bool addVNode(OLCB_NodeID *NID)
  {
	return _aliasHelper.allocateAlias(NID);//TODO This can sometimes fail!
  }
  
// protected:
  
// private:
  //OLCB_CAN_Buffer txBuffer, rxBuffer;
  OLCB_Buffer txBuffer, rxBuffer;
  OLCB_CAN_Alias_Helper _aliasHelper; //TODO I would strongly prefer this not to be a pointer! But I run into an issue with circular references otherwise. Get a "error: field '_aliasHelper' has incomplete type" error. What's going on!?
  OLCB_Alias_Cache _translationCache;
  
  //stuffs for VerifyNodeID/VerifiedNodeID; TODO!
  OLCB_NodeID _nodeIDToBeVerified;
  uint32_t _aliasCacheTimer;

  /**
   * Send the next CID message.  Return true it you can/did,
   * false if you didn't.
   */
  bool sendCID(OLCB_NodeID* nodeID, uint8_t i);
  
  /**
   * Send an RIM message.  Return true it you can/did,
   * false if you didn't.
   */
  bool sendRID(OLCB_NodeID* nodeID);
  
  /**
   * Send the InitializationComplete when everything is OK.
   *  Return true it you can/did,
   * false if you didn't.
   */
  bool sendInitializationComplete(OLCB_NodeID* nodeID);
  
  /*Methods for handling nida caching TODO THESE NEED UPDATING!!*/
  bool sendNIDVerifyRequest(OLCB_NodeID *nid);
  
  bool sendAMR(OLCB_NodeID *nid);
  
};

#endif //__OLCB_LINK_H__
