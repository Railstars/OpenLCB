//#include "OLCB_CAN_Alias_Helper.h"
#include "OLCB_CAN_Link.h"

void OLCB_CAN_Alias_Helper::initialize(OLCB_CAN_Link *link)
{
	_link = link;
}

void OLCB_CAN_Alias_Helper::handleRID(OLCB_CAN_Buffer *RIDMsg)
{
	//check to see if RID matches anything currently being run through the buffers. if so, it needs to go back to the beginning.
	//Note: Not sure what to do in the case that CID1 is full!!
		//Currently, we just silently drop the NodeID, which is no good at all
		
	//check RID queue:
	//use reAllocateAlias()
	//check CID4 queue;
	//check CID3 queue;
	//check CID2 queue;
	//check CID1 queue;
}

void OLCB_CAN_Alias_Helper::handleCID(OLCB_CAN_Buffer *CIDMsg)
{
	//TODO
}

void OLCB_CAN_Alias_Helper::update(void)
{
//	Serial.println("=Alias_Helper::Update");
	//check RID queue.
	if(!_RID.isEmpty())
	{
//		Serial.println(" Checking RID");
		private_nodeID_t *iter;
		while(!_RID.isEmpty() && (millis() - _RID.peek()->time_stamp) > RID_TIME_WAIT)
		{
			iter = _RID.pop();
			if(!_link->sendRID(iter->node))
			{
//				Serial.println("Failure to send RID!?");
				//can't send a RID just now, outgoing CAN queue is busy
				_RID.push(iter);
				return; //just quit for now.
			}
//			Serial.println("RID SENT!");
			//TODO send AMD and InitializationComplete
		}
	}

	//check CID4 queue.
	if(!_CID4.isEmpty())
	{
//		Serial.println(" Checking CID4");
		private_nodeID_t *iter;
		while(!_CID4.isEmpty())
		{
			iter = _CID4.pop();
			if(!_link->sendCID(iter->node, 4) || !_RID.push(iter))
			{
				//can't send a RID just now, either the outgoing CAN queue is busy, or the RID queue is full
				_CID4.push(iter);
				return; //just quit for now.
			}
		}
	}
	
	//check CID3 queue.
	if(!_CID3.isEmpty())
	{
//		Serial.println(" Checking CID3");
		private_nodeID_t *iter;
		while(!_CID3.isEmpty())
		{
			iter = _CID3.pop();
			if(!_link->sendCID(iter->node, 3) || !_CID4.push(iter))
			{
				//can't send a CID3 just now, either the outgoing CAN queue is busy, or the CID4 queue is full
				_CID3.push(iter);
				return; //just quit for now.
			}
		}
	}
	
	//check CID2 queue.
	if(!_CID2.isEmpty())
	{
//		Serial.println(" Checking CID2");
		private_nodeID_t *iter;
		while(!_CID2.isEmpty())
		{
			iter = _CID2.pop();
			if(!_link->sendCID(iter->node, 2) || !_CID3.push(iter))
			{
				//can't send a CID2 just now, either the outgoing CAN queue is busy, or the CID3 queue is full
				_CID2.push(iter);
				return; //just quit for now.
			}
		}
	}

	//check CID1 queue.
	if(!_CID1.isEmpty())
	{
//		Serial.println(" Checking CID1");
		private_nodeID_t *iter;
		while(!_CID1.isEmpty())
		{
//			Serial.println(" Grabbing next node ID");
			iter = _CID1.pop();
			if(!_link->sendCID(iter->node, 1))
			{
//				Serial.println(" Failure to send CID1!");
				_CID1.push(iter);
				return; //just quit for now.
			}
//			Serial.println(" Success sending CID1");
			if(!_CID2.push(iter))
			{
//				Serial.println(" Failure to push onto CID2");
				//can't send a CID2 just now, either the outgoing CAN queue is busy, or the CID3 queue is full
				_CID1.push(iter);
				return; //just quit for now.
			}
//			Serial.println(" Success");
		}
	}
}

bool OLCB_CAN_Alias_Helper::allocateAlias(OLCB_NodeID* nodeID)
{
//	Serial.println("=allocateAlias");
	if(_CID1.isFull())
	{
//		Serial.println("  but _CID1 is full!");
		//return false;
	}
	uint32_t lfsr1 = (((uint32_t)nodeID->val[0]) << 16) | (((uint32_t)nodeID->val[1]) << 8) | ((uint32_t)nodeID->val[2]);
    uint32_t lfsr2 = (((uint32_t)nodeID->val[3]) << 16) | (((uint32_t)nodeID->val[4]) << 8) | ((uint32_t)nodeID->val[5]);
	nodeID->alias = (lfsr1 ^ lfsr2 ^ (lfsr1>>12) ^ (lfsr2>>12) )&0xFFF;
//	nodeID->print();
	
	return (_CID1.push(nodeID, lfsr1, lfsr2));
}

bool OLCB_CAN_Alias_Helper::reAllocateAlias(private_nodeID_t* nodeID)
{
	if(_CID1.isFull())
		return false;
	uint32_t temp1 = ((nodeID->lfsr1<<9) | ((nodeID->lfsr2>>15)&0x1FF)) & 0xFFFFFF;
	uint32_t temp2 = (nodeID->lfsr2<<9) & 0xFFFFFF;
   
	// add
	nodeID->lfsr2 = nodeID->lfsr2 + temp2 + 0x7A4BA9l;
	nodeID->lfsr1 = nodeID->lfsr1 + temp1 + 0x1B0CA3l;
   
	// carry
	nodeID->lfsr1 = (nodeID->lfsr1 & 0xFFFFFF) | ((nodeID->lfsr2&0xFF000000) >> 24);
	nodeID->lfsr2 = nodeID->lfsr2 & 0xFFFFFF;

	nodeID->node->alias = (nodeID->lfsr1 ^ nodeID->lfsr2 ^ (nodeID->lfsr1>>12) ^ (nodeID->lfsr2>>12) )&0xFFF;
	
	return (_CID1.push(nodeID));
}


bool OLCB_CAN_Alias_Helper::releaseAlias(OLCB_NodeID* nodeID)
{
	//TODO: Emit AMR message.
	if(_link->sendAMR(nodeID))
	{
		nodeID->alias = 0;
		return true;
	}
	return false;
}
