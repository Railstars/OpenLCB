#ifndef __OLCB_CAN_ALIAS_HELPER_H__
#define __OLCB_CAN_ALIAS_HELPER_H__

#define CAN_ALIAS_BUFFER_SIZE	10
#define	RID_TIME_WAIT			500 //ms. Up for debate how long this should be!

#include <stdint.h>
#include "wprogram.h"

#include "OLCB_CAN_Buffer.h"
#include "OLCB_NodeID.h"

class OLCB_CAN_Link;


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
    	Serial.println("=push(private)");
    	if(!isFull())
    	{
    		Serial.print(" writing to location ");
    		Serial.println(write, DEC);
    		buffer[write].node = private_node->node;
    		buffer[write].time_stamp = millis();
    		buffer[write].lfsr1 = private_node->lfsr1;
    		buffer[write].lfsr2 = private_node->lfsr2;
    		write = (write+1)%CAN_ALIAS_BUFFER_SIZE; //which might be the read head!
    		++size;
    		return true;
    	}
    	Serial.println("  buffer is full!");
    	return false;
    }
    bool push(OLCB_NodeID *node, uint32_t lfsr1, uint32_t lfsr2)
    {
    	Serial.println("=push(node)");
    	if(!isFull())
    	{
    		Serial.print(" writing to location ");
    		Serial.println(write, DEC);
    		buffer[write].node = node;
    		buffer[write].time_stamp = millis();
    		buffer[write].lfsr1 = lfsr1;
    		buffer[write].lfsr2 = lfsr2;
    		write = (write+1)%CAN_ALIAS_BUFFER_SIZE; //which might be the read head!
    		++size;
    		return true;
    	}
    	Serial.println("  buffer is full!");
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
    	return NULL;
    }
    private_nodeID_t *peek(void)
    {
    	if(!isEmpty())
    		return &(buffer[read]);
    	return NULL;
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

#endif //__OLCB_CAN_ALIAS_HELPER_H__
