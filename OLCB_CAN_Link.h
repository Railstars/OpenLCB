#ifndef __OLCB_CAN_LINK_H__
#define __OLCB_CAN_LINK_H__

#include <stdint.h>
#include "can.h"
#include "Arduino.h"
#include "OLCB_Link.h"
#include "OLCB_CAN_Buffer.h"
#include "OLCB_NodeID.h"
#include "OLCB_Event.h"
#include "OLCB_Datagram.h"
#include "OLCB_Stream.h"
#include "OLCB_Alias_Cache.h"

// state machine definitions NOT USED apparently!?
#define STATE_INITIAL 0
#define STATE_WAIT_CONFIRM 10
#define STATE_ALIAS_ASSIGNED 20
#define STATE_INITIALIZED 30

#define NODE_ID_STATE_INACTIVE  0
#define NODE_ID_STATE_CID1      1
#define NODE_ID_STATE_CID2      2
#define NODE_ID_STATE_CID3      3
#define NODE_ID_STATE_CID4      4
#define NODE_ID_STATE_RID       5

/**********************/
//YOU MUST SET THIS TO EQUAL THE NUMBER OF NIDS YOU WISH TO USE!
//This definition is pretty bad, but universally useable
#if defined(__AVR_ATMEGA328__) || defined(__AVR_ATMEGA168__)
#define CAN_ALIAS_BUFFER_SIZE   5
#else
#define CAN_ALIAS_BUFFER_SIZE	20
#endif

#define RID_TIME_WAIT           500 //ms. Up for debate how long this should be!

class OLCB_CAN_Link;

#define ALIAS_EMPTY_STATE 0	//idle, doing nothing, no alias
#define ALIAS_HOLDING_STATE 1	//alias allocated, not assigned to a NodeID
#define ALIAS_CID1_STATE 2
#define ALIAS_CID2_STATE 3
#define ALIAS_CID3_STATE 4
#define ALIAS_CID4_STATE 5
#define ALIAS_RID_STATE 6
#define ALIAS_AMD_STATE 7
#define ALIAS_RELEASING_STATE 8
#define ALIAS_RESENDRID_STATE 15
#define ALIAS_SENDVERIFIEDNID_STATE 16
#define ALIAS_READY_STATE 20	//assigned to a NodeID

struct private_nodeID_t
{
    OLCB_NodeID *node;
    uint8_t state;
    uint16_t alias; //yes, an alias.
    uint32_t time_stamp;
    uint32_t lfsr1, lfsr2;
};

/* This class is a helper class that manages the allocation of aliases */

class OLCB_CAN_Alias_Helper
{
  public:
    OLCB_CAN_Alias_Helper() : index(0)
    {
    	for(uint8_t i = 0; i < CAN_ALIAS_BUFFER_SIZE; ++i)
    	{
    		_nodes[i].alias = 0;
    		_nodes[i].node = 0;
    		_nodes[i].state = ALIAS_EMPTY_STATE;
    	}
    }
    void initialize(OLCB_CAN_Link *link);
    void checkMessage(OLCB_CAN_Buffer *msg);
    void update(void);
    void preAllocateAliases(void);
    void allocateAlias(OLCB_NodeID* nodeID);
    void reAllocateAlias(private_nodeID_t* nodeID);
    bool releaseAlias(OLCB_NodeID* nodeID);
    void idleAlias(OLCB_NodeID* nodeID);
  private:
    //methods
    //fields
    uint8_t index;
    OLCB_CAN_Link *_link;
    uint32_t _helper_value; //for constructing aliases with a NID to seed the LSFR.
    private_nodeID_t _nodes[CAN_ALIAS_BUFFER_SIZE];
};
/**********************/


class OLCB_CAN_Link : public OLCB_Link
{
 public:
  OLCB_CAN_Link() : internalMessage(false)
  {
      return;
  }
  
  bool initialize(void);
  
//  bool negotiateAlias(OLCB_NodeID *nid);
  void negotiateAliasesInQueue(void);
  
  bool handleTransportLevel(void);
  virtual void update(void);
  
  uint8_t sendDatagramFragment(OLCB_Datagram *datagram, uint8_t start);
  bool ackDatagram(OLCB_NodeID *source, OLCB_NodeID *dest);
  bool nakDatagram(OLCB_NodeID *source, OLCB_NodeID *dest, int reason);
  
  //TODO This one needs implementation!
  bool sendStream(OLCB_Stream *stream) {return false;}
  //Not sure that this is how streams should work at all!
  
  bool sendVerifiedNID(OLCB_NodeID *nid);
    /*Methods for handling nida caching TODO THESE NEED UPDATING!!*/
  bool sendVerifyNID(OLCB_NodeID *src, OLCB_NodeID *request);

  bool sendIdent(void);
  
  bool sendPCER(OLCB_Event *event);
  bool sendConsumerIdentified(OLCB_Event *event);
  bool sendLearnEvent(OLCB_Event *event);
  bool sendProducerIdentified(OLCB_Event *event);


  void addVNode(OLCB_Virtual_Node *vnode);
  void removeVNode(OLCB_Virtual_Node *vnode);
  
  //friend class OLCB_CAN_Alias_Helper;
 //protected:
  
 //private:
  //This method would not only send the current txbuffer over CAN, but would distribute the message locally among vnodes as well.
  bool sendMessage(void);
  void deliverMessage(void);


  //OLCB_CAN_Buffer txBuffer, rxBuffer;
  OLCB_Buffer txBuffer, rxBuffer;
  OLCB_CAN_Alias_Helper _aliasHelper;
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
    
  bool sendAMR(OLCB_NodeID *nid);
  bool sendAMD(OLCB_NodeID *nid);
  
  bool internalMessage;
};

#endif //__OLCB_LINK_H__
