#ifndef __OLCB_CAN_LINK_H__
#define __OLCB_CAN_LINK_H__

#include <stdint.h>
#include "can.h"
#include "WProgram.h"
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
#define CAN_ALIAS_BUFFER_SIZE   10
#define RID_TIME_WAIT           500 //ms. Up for debate how long this should be!

#define CAN_BUFFER_SIZE			10 //The value of this constant really depends on how many vnodes there are! It need not ever be larger than the number of vnodes, but it shouldn't be too much smaller, either. If every vnode will always be sending out a packet each loop() cycle, then it must be equal to the number of vnodes.

class OLCB_CAN_Link;

struct private_nodeID_t
{
    OLCB_NodeID *node;
    uint32_t time_stamp;
    uint32_t lfsr1, lfsr2;
};

class OLCB_Alias_Buffer
{
  public:
    OLCB_Alias_Buffer() : write(0), read(0), size(0)
    {
    }
    bool isEmpty(void) { return size?false:true; }
    bool isFull(void) { return (size==CAN_ALIAS_BUFFER_SIZE)?true:false; }
    bool push(private_nodeID_t *private_node)
    {
//      Serial.println("=push(private)");
        if(!isFull())
        {
//          Serial.print(" writing to location ");
//          Serial.println(write, DEC);
            buffer[write].node = private_node->node;
            buffer[write].time_stamp = millis();
            buffer[write].lfsr1 = private_node->lfsr1;
            buffer[write].lfsr2 = private_node->lfsr2;
            write = (write+1)%CAN_ALIAS_BUFFER_SIZE; //which might be the read head!
            ++size;
            return true;
        }
//      Serial.println("  buffer is full!");
        return false;
    }
    bool push(OLCB_NodeID *node, uint32_t lfsr1, uint32_t lfsr2)
    {
//      Serial.println("=push(node)");
        if(!isFull())
        {
//          Serial.print(" writing to location ");
//          Serial.println(write, DEC);
            buffer[write].node = node;
            buffer[write].time_stamp = millis();
            buffer[write].lfsr1 = lfsr1;
            buffer[write].lfsr2 = lfsr2;
            write = (write+1)%CAN_ALIAS_BUFFER_SIZE; //which might be the read head!
            ++size;
            return true;
        }
//      Serial.println("  buffer is full!");
        return false;
    }

    private_nodeID_t *pop(void)
    {
        if(!isEmpty())
        {
            private_nodeID_t *retval = &(buffer[read]);
            read = (read+1)%CAN_ALIAS_BUFFER_SIZE;
            --size;
            return retval;
        }
        return 0; //TODO NULL;
    }
    private_nodeID_t *peek(void)
    {
        if(!isEmpty())
            return &(buffer[read]);
        return 0; //NULL;
    }
  private:
    private_nodeID_t buffer[CAN_ALIAS_BUFFER_SIZE] ;
    uint8_t write;
    uint8_t read;
    uint8_t size;
};

/* This class is a helper class that manages the allocation of aliases */

class OLCB_CAN_Alias_Helper
{
  public:
    OLCB_CAN_Alias_Helper()
    {
    }
    void initialize(OLCB_CAN_Link *link);
    void handleRID(OLCB_CAN_Buffer *RIDMsg);
    void handleCID(OLCB_CAN_Buffer *CIDMsg);
    void update(void);
    bool allocateAlias(OLCB_NodeID* nodeID);
    bool reAllocateAlias(private_nodeID_t* nodeID);
    bool releaseAlias(OLCB_NodeID* nodeID);
  private:
    //methods
    //fields
    OLCB_CAN_Link *_link;
    OLCB_Alias_Buffer _CID1;
    OLCB_Alias_Buffer _CID2;
    OLCB_Alias_Buffer _CID3;
    OLCB_Alias_Buffer _CID4;
    OLCB_Alias_Buffer _RID;
};
/**********************/

/*
This is a stupid thing to need, but I cannot think of any other way.
The problem is this: Outgoing CAN messages are often needed by other
virtual nodes on the same physical node. This buffer is used to hold
outgoing messages to be repeated back to the virtual nodes. It is a total
kludge, but I cannot think of any other way of doing it. Part of the
kludgyness comes from implementing YET ANOTHER circular buffer; if templates
turn out to be usable on AVR, I will shoot myself for doing this. In the
meantime, here's another circular buffer.
*/

class OLCB_CAN_Circular_Buffer
{
  public:
    OLCB_CAN_Circular_Buffer() : write(0), read(0), size(0)
    {
    }
    bool isEmpty(void) { return size?false:true; }
    bool isFull(void) { return (size==CAN_BUFFER_SIZE)?true:false; }
    
    bool push(OLCB_CAN_Buffer *new_node)
    {
        if(!isFull())
        {
			memcpy(&(buffer[write]), new_node, sizeof(OLCB_CAN_Buffer));
            write = (write+1)%CAN_BUFFER_SIZE; //which might be the read head!
            ++size;
            return true;
        }
        return false;
    }

    bool pop(OLCB_CAN_Buffer *new_node)
    {
    	bool retval = peek(new_node);
        if(retval)
        {
            read = (read+1)%CAN_ALIAS_BUFFER_SIZE;
            --size;
        }
        return retval;
    }
    
    bool peek(OLCB_CAN_Buffer *new_node)
    {
        if(!isEmpty())
        {
        	memcpy(new_node, &(buffer[read]), sizeof(OLCB_CAN_Buffer));
        	return true;
        }
        return false;
    }
    
  private:
    OLCB_CAN_Buffer buffer[CAN_BUFFER_SIZE] ;
    uint8_t write;
    uint8_t read;
    uint8_t size;
};

class OLCB_CAN_Link : public OLCB_Link
{
 public:
  OLCB_CAN_Link(OLCB_NodeID *NID) : OLCB_Link(NID)
  {
//    Serial.print("OLCB_CAN_Link: ");
//    Serial.println((uint16_t)id, HEX);
    //_nodeIDToBeVerified.set(0,0,0,0,0,0);
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
  
  bool sendEvent(OLCB_Event *event);
  bool sendConsumerIdentified(OLCB_Event *event);
  bool sendLearnEvent(OLCB_Event *event);
  bool sendProducerIdentified(OLCB_Event *event);


  bool addVNode(OLCB_NodeID *NID);
  
  //friend class OLCB_CAN_Alias_Helper;
 //protected:
  
 //private:
  //This method would not only send the current txbuffer over CAN, but would distribute the message locally among vnodes as well.
  bool sendMessage(OLCB_CAN_Buffer *to_send);

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
  
  /*Methods for handling nida caching TODO THESE NEED UPDATING!!*/
  bool sendNIDVerifyRequest(OLCB_NodeID *nid);
  
  bool sendAMR(OLCB_NodeID *nid);
  
  
  OLCB_CAN_Circular_Buffer repeat_buffer;
  
};

#endif //__OLCB_LINK_H__
