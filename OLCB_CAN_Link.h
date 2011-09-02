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

class OLCB_CAN_Link;

struct private_nodeID_t
{
    OLCB_NodeID *node;
    uint32_t time_stamp;
    uint32_t lfsr1, lfsr2;
};

class OLCB_Circular_Buffer
{
  public:
    OLCB_Circular_Buffer() : write(0), read(0), size(0)
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
    OLCB_Circular_Buffer _CID1;
    OLCB_Circular_Buffer _CID2;
    OLCB_Circular_Buffer _CID3;
    OLCB_Circular_Buffer _CID4;
    OLCB_Circular_Buffer _RID;
};
/**********************/

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
    
  bool sendEvent(OLCB_Event *event) {return false;}
  
  uint8_t sendDatagramFragment(OLCB_Datagram *datagram, uint8_t start);
  bool ackDatagram(OLCB_NodeID *source, OLCB_NodeID *dest);
  bool nakDatagram(OLCB_NodeID *source, OLCB_NodeID *dest, int reason);
  
  bool sendStream(OLCB_Stream *stream) {return false;}
  //Not sure that this is how streams should work at all!
  
  bool sendVerifiedNID(OLCB_NodeID *nid);
  
  //TODO!!
  bool sendConsumerIdentified(OLCB_Event *event) {return false;}
  bool sendLearnEvent(OLCB_Event *event) {return false;}
  bool sendProducerIdentified(OLCB_Event *event) {return false;}


  bool addVNode(OLCB_NodeID *NID);
  
  //friend class OLCB_CAN_Alias_Helper;
 //protected:
  
// private:
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
  
};

#endif //__OLCB_LINK_H__
