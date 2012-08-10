#include "OLCB_CAN_Link.h"
#include "Arduino.h"
//Code taken from LinkControl.cpp


//////Initialization routine
// Based on check(), etc
bool OLCB_CAN_Link::initialize(void)
{
    if(!can_init(BITRATE_125_KBPS))
    {
        return false;
    }
 
#if defined (__AVR_AT90CAN128__) || defined (__AVR_AT90CAN64__) || defined (__AVR_AT90CAN32__)
	for(uint8_t i = 0; i < 15; ++i)
    {
    	CANPAGE = (i << 4);
    	CANIDM4 |= (1<<IDEMSK); //ignore standard frames
	}
#else
	//TODO figure out how to set up filters for MCP2515
#endif
    
    _aliasHelper.initialize(this);
    _translationCache.initialize(10);
    
    //Negotiate the alias for this Node's real ID.
    //_aliasHelper.allocateAlias(_nodeID);
    
  return true;
}


// send the next CID message.  "i" is the 0-3 ordinal number of the message, which
// becomes F-C in the CID itself. Returns true if successfully sent.
bool OLCB_CAN_Link::sendCID(OLCB_NodeID *nodeID, uint8_t i) {
  if (!can_check_free_buffer()) return false;  // couldn't send just now
  uint16_t fragment;
  switch (i) {
    case 1:  fragment = ( (nodeID->val[0]<<4)&0xFF0) | ( (nodeID->val[1] >> 4) &0xF);
             break;
    case 2:  fragment = ( (nodeID->val[1]<<8)&0xF00) | ( nodeID->val[2] &0xF);
             break;
    case 3:  fragment = ( (nodeID->val[3]<<4)&0xFF0) | ( (nodeID->val[4] >> 4) &0xF);
             break;
    default:
    case 4:  fragment = ( (nodeID->val[4]<<8)&0xF00) | ( nodeID->val[5] &0xF);
             break;
  }
  txBuffer.setCID(i,fragment,nodeID->alias);
  while(!sendMessage());  // wait for queue, but earlier check says will succeed
  return true;
}

bool OLCB_CAN_Link::sendRID(OLCB_NodeID* nodeID)
{
  if (!can_check_free_buffer()) return false;  // couldn't send just now
  txBuffer.setRID(nodeID->alias);
  memcpy(txBuffer.data, nodeID->val, 6); //This seems an important part of the message that is being left off!
  while(!sendMessage());  // wait for queue, but earlier check says will succeed
  return true;
}

bool OLCB_CAN_Link::sendInitializationComplete(OLCB_NodeID* nodeID)
{
  if (!can_check_free_buffer()) return false;  // couldn't send just now
  txBuffer.setInitializationComplete(nodeID);
  while(!sendMessage());    // wait for queue, but earlier check says will succeed
  return true;
}

bool OLCB_CAN_Link::sendRejectOptionalInteraction(OLCB_NodeID* source, OLCB_NodeID* dest, uint16_t MTI)
{
  if (!can_check_free_buffer()) return false;  // couldn't send just now
  txBuffer.setRejectOptionalInteraction(source, dest, MTI);
  while(!sendMessage());  // wait for queue, but earlier check says will succeed
  return true;
}

/////// Methods for sending and receiving OLCB packets over CAN

bool OLCB_CAN_Link::handleTransportLevel()
{
	//Serial.println("handleTransportLevel");
	OLCB_NodeID n(0,0,0,0,0,0);
    // see if this is a Verify request to us; first check type
    if (rxBuffer.isVerifyNID())
    {
    	//if this is a request for any of our vnodes to respond with a verifiednodeid, we should do so.
    	//if is global with empty payload, all vnodes respond, full stop.
    	//if is global with NodeID, and one vnode matches, respond.
    	//if is addressed, and alias matches, respond.
    	//if is addressed, and nodeIde matches, respond.
      //pass it off to the alias helper, since it maintains a definitive list of registered nodeIDs
      _aliasHelper.verifyNID(&rxBuffer);
      return true;
    }
    // Perhaps it is someone sending a Verified NID packet. We might have requested that, in which case we should cache it
    else if (rxBuffer.isVerifiedNID())
    {
      //Serial.println("!!!! Got VerifiedNID!");
      // We have a packet that contains a verified NID. We might have requested this. Let's check.
      rxBuffer.getSourceNID(&n); //get the alias from the message header
      rxBuffer.getNodeID(&n); //Get the actual NID from the message body
      if(n.alias == 0) return false;
      if(n.sameNID(&_nodeIDToBeVerified))
      {
      	//Serial.println("And it's the one we were waiting for!");
        //add it to the NID/NIDa translation cache
        _translationCache.add(&n);
        //remove from temp buffer
        _nodeIDToBeVerified.set(0,0,0,0,0,0);
      }
      return true;
    }
    else if(rxBuffer.isAME())
    {
      rxBuffer.getNodeID(&n);
      //Serial.println("Got an AME for:");
      //n.print();
      _aliasHelper.sendAMD(&n);
      return true;
    }
    else if(rxBuffer.isAMR()) //is someone releasing their alias? Remove it from the cache.
    {
      //Serial.println("Link: Got AMR");
      rxBuffer.getSourceNID(&n);
      //n.print();
      _translationCache.removeByAlias(rxBuffer.getSourceAlias());
      //and, kill any pending transmissions between that node and any of ours:
      OLCB_Virtual_Node *iter = _handlers;
      while(iter)
      {
        //Serial.println("calling clearBuffer");
      	iter->clearBuffer(&n);
        iter = iter->next;
      }
      return true;
    }
    else if(rxBuffer.isAMD()) //is someone claiming a new alias? Update the cache
    {
      //Serial.println("Link: Got AMD");
      rxBuffer.getNodeID(&n);
      if(!n.empty())
      {
	      n.alias = rxBuffer.getSourceAlias();
      	  //n.print();
      	  _translationCache.add(&n);
      	  //TODO clear buffers? probably not needed, node should transmit "buffer full retransmit" error, so OK.
      }
      else //AMD should not be empty; assume that the alias is now lost in limbo until we hear otherwise.
      {
	    _translationCache.removeByAlias(rxBuffer.getSourceAlias());
	    //and, kill any pending transmissions between that node and any of ours:
	    OLCB_Virtual_Node *iter = _handlers;
    	while(iter)
	    {
	      rxBuffer.getSourceNID(&n);
   	      //Serial.println("calling clearBuffer");
    	  iter->clearBuffer(&n);
          iter = iter->next;
      	}
      }
      return true;
    }
    return false;
}

void OLCB_CAN_Link::update(void)
{
  //check first for any self-generated messages, and second for messages
  // coming from the CAN bus
  if(rxBuffer.isInternal()) //we have a message waiting to be delivered
  {
  	deliverMessage();
	rxBuffer.setExternal(); //so we don't read it again
  }
  if(can_get_message(&rxBuffer))
  {
//  	if(rxBuffer.flags.extended) //ignore standard frames TODO THIS MUST BE UNCOMMENTED FOR MCP2515 USE!
//  	{
      //Serial.println("Got message on wire");
      //Serial.println(rxBuffer.id, HEX);
      rxBuffer.setExternal();
      deliverMessage();
      wasActive = true;
//    }
  }
  //update alias allocation
  _aliasHelper.update();
  OLCB_Link::update();
}

void OLCB_CAN_Link::deliverMessage(void)
{
	//first, send it to our aliasHelper to ensure against duplicate aliases.
	_aliasHelper.checkMessage(&rxBuffer);

    // See if this message is a CAN-level message that we should be handling.
    if(handleTransportLevel())
    {
		//bail early, the packet grabbed isn't for any of the attached handlers to deal with
	     return;
	}
    //otherwise, let's pass it on to our handlers
    
    //First, if there is a source for this message, see if we can pull the full NID from the cache!
    OLCB_NodeID n;
    rxBuffer.getSourceNID(&n); //get the alias
    if(_translationCache.getNIDByAlias(&n)) //attempt to fill it in with a NID from the cache
      rxBuffer.setSourceNID(&n); //overwrite the original with the full NID
    //now, see if it has a destination, and if it's in the cache too
    if(rxBuffer.getDestNID(&n))
    {
      //see if we can pull the actual NID from the cache
      if(_translationCache.getNIDByAlias(&n))
        rxBuffer.setDestNID(&n);
    }
    else
    {
    	//no destination specified
	    n.set(0,0,0,0,0,0);
	}
    
    //now, pass the message among the handlers;
    bool handled = false;
    OLCB_NodeID *dest_node = NULL;
    
    OLCB_Virtual_Node *iter = _handlers;
    while(iter)
    {
      //check to see if message is addressed to this node
      if(n.alias && (n.alias == iter->NID->alias))
      {
      	dest_node = iter->NID;
      }
      if(iter->handleMessage((OLCB_Buffer*)(&rxBuffer)))
      {
        handled = true;
      }
      iter = iter->next;
    }
    //TODO why do we only care about external messages here? Shouldn't vnodes be able to send rejections to other vnodes?
    if(rxBuffer.isExternal() && dest_node && !handled) //if it came from the outside, and was addressed to one of ours, and it went unhandled, then:
    {
    	//Serial.println("Message was external, addressed to us, and not handled");
    	rxBuffer.getSourceNID(&n);
    	sendRejectOptionalInteraction(dest_node, &n, rxBuffer.getMTI());
    }
}

uint8_t OLCB_CAN_Link::sendDatagramFragment(OLCB_Datagram *datagram, uint8_t start)
{
  //datagram is the datagram to transmit.
  //start is the index of the next byte to start from.
  //returns the number of bytes sent.
  //Serial.println("sendDGfragment");
  //datagram->destination.print();
  if(!datagram->destination.alias) //there is no alias here. We'll need to look it up.
  {
  	//Serial.println("no alias in dest, looking up");
    //try the cache
    uint16_t alias = _translationCache.getAliasByNID(&(datagram->destination));
    //Serial.println(alias, HEX);
    if(!alias) //not cached!
    {
      //need to ask
      //Serial.println("Link: Gonna have to ask with a VerifyNID");
      sendVerifyNID(&(datagram->source), &(datagram->destination)); //if it can't go through, it'll get called again. no need to loop.
      return 0; //wait to transmit until we know what the proper alias is.
    }
  }
  //datagram->destination.print();
  //now, figure out how many bytes remain, and whether this is the last fragment that needs to be sent.
  // Notice that the CAN link can send 8 bytes per frame.
  //set the source, and init the buffer.
  //several possible cases.
  if(start == 0) //first fragment
  {
  	if(datagram->length > 8) //this is just the first
  	{
  		//Serial.println("first fragment");
  	  txBuffer.setFirstDatagram(&(datagram->source), &(datagram->destination));
  	}
  	else //will fit into one message, so it is first and last.
  	{
  		//Serial.println("only fragment");
  	  txBuffer.setOnlyDatagram(&(datagram->source), &(datagram->destination));
  	}
  }
  else if((datagram->length - start) > 8) //not yet done!
  {
  	//Serial.println("middle fragment");
    txBuffer.setMiddleDatagram(&(datagram->source), &(datagram->destination));
  }
  else //last fragment!
  {
  	//Serial.println("last fragment");
    txBuffer.setLastDatagram(&(datagram->source), &(datagram->destination));
  }
  
  uint8_t len = min(datagram->length-start,8);
  txBuffer.length = len;
  for (uint8_t i = 0; i<txBuffer.length; i++)
    txBuffer.data[i] = datagram->data[i+start];

	//Serial.println("sending message");
  while(!sendMessage());
  //Serial.println("Fragment away!");
  
  return len;
}

bool OLCB_CAN_Link::sendVerifyNID(OLCB_NodeID *src, OLCB_NodeID *request)
{
  //first, see if a request is pending
//  uint32_t time = millis();
  //TODO this mechanism could be severely improved to permit multiple verifyIDs to be sent out!!
  if(!_nodeIDToBeVerified.empty() && ((millis() - _aliasCacheTimer) < 2000) ) //it's not zeros, and it hasn't yet been a full second since the last request
  {
    return false;
  }  
  if (!can_check_free_buffer()){
    return false;  // couldn't send just now
  }
  memcpy(&_nodeIDToBeVerified, request, sizeof(OLCB_NodeID));
  //sendVerifiedNID(_nodeID);
  txBuffer.setVerifyNIDGlobal(src, request);
  while(!sendMessage());  // wait for queue, but earlier check says will succeed
  _aliasCacheTimer = millis(); //set the clock going. A request will only be permitted to stand for 1 second.
  return true;
}

bool OLCB_CAN_Link::sendVerifyNID(OLCB_NodeID *src)
{
  //first, see if a request is pending
//  uint32_t time = millis();
  //TODO this mechanism could be severely improved to permit multiple verifyIDs to be sent out!!
  if(!_nodeIDToBeVerified.empty() && ((millis() - _aliasCacheTimer) < 2000) ) //it's not zeros, and it hasn't yet been a full second since the last request
  {
    return false;
  }  
  if (!can_check_free_buffer()){
    return false;  // couldn't send just now
  }
  txBuffer.setVerifyNIDGlobal(src);
  while(!sendMessage());  // wait for queue, but earlier check says will succeed
  _aliasCacheTimer = millis(); //set the clock going. A request will only be permitted to stand for 1 second.
  return true;
}

bool OLCB_CAN_Link::ackDatagram(OLCB_NodeID *source, OLCB_NodeID *dest)
{
  if(!can_check_free_buffer())
    return false;
  txBuffer.setDatagramAck(source, dest);
  //Serial.println("Sending ACK");
  //Serial.println(txBuffer.data[0], HEX);
  while(!sendMessage());
  return true;
}

bool OLCB_CAN_Link::nakDatagram(OLCB_NodeID *source, OLCB_NodeID *dest, int reason = DATAGRAM_REJECTED_PERMANENT_ERROR)
{
	//Serial.println("Sending Datagram NAK");
  if(!can_check_free_buffer())
    return false;
  txBuffer.setDatagramNak(source, dest, reason);
  //Serial.println(txBuffer.data[1], HEX);
  //Serial.println(txBuffer.data[2], HEX);
  while(!sendMessage());
  return true;
}

bool OLCB_CAN_Link::sendVerifiedNID(OLCB_NodeID *nid)
{
//	//Serial.println("Attempting to send VerifiedNID!");
  if(!can_check_free_buffer())
    return false;
  txBuffer.setVerifiedNID(nid);
  while(!sendMessage());
  return true;
}

bool OLCB_CAN_Link::sendAMR(OLCB_NodeID *nid)
{
  if(!can_check_free_buffer())
    return false;
  txBuffer.setAMR(nid);
  while(!sendMessage());
  return true;
}

bool OLCB_CAN_Link::sendAMD(OLCB_NodeID *nid)
{
  if(!can_check_free_buffer())
    return false;
  txBuffer.setAMD(nid);
  while(!sendMessage());
  return true;
}


bool OLCB_CAN_Link::sendIdent(OLCB_NodeID* source)
{
	OLCB_Event e(0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x00);
	return sendPCER(source, &e);
}

bool OLCB_CAN_Link::sendPCER(OLCB_NodeID* source, OLCB_Event *event)
{
    if(!can_check_free_buffer())
        return false;
    txBuffer.setPCEventReport(source, event);
    while(!sendMessage());
    return true;
}

bool OLCB_CAN_Link::sendConsumerIdentified(OLCB_NodeID* source, OLCB_Event *event)
{
	//Serial.println("sending Consumer Identified");
    if(!can_check_free_buffer())
    {
        return false;
    }
    txBuffer.setConsumerIdentified(source, event);
    while(!sendMessage());
    return true;
}

bool OLCB_CAN_Link::sendLearnEvent(OLCB_NodeID* source, OLCB_Event *event)
{
    if(!can_check_free_buffer())
        return false;
    txBuffer.setLearnEvent(source, event);
    while(!sendMessage()); //TODO make a new method for sendMessage that also repeats it back to the link!
    return true;
}

bool OLCB_CAN_Link::sendProducerIdentified(OLCB_NodeID* source, OLCB_Event *event)
{    
   	//Serial.println("sending Producer Identified");
    if(!can_check_free_buffer())
    {
    		//Serial.println("no free buffer");
        return false;
    }
    txBuffer.setProducerIdentified(source, event);
    while(!sendMessage());
    return true;
}

bool OLCB_CAN_Link::sendMessage(OLCB_Buffer *msg)
{
	if(!can_check_free_buffer())
    {
        return false;
    }
    memcpy(&txBuffer, (OLCB_CAN_Buffer*)msg, sizeof(OLCB_CAN_Buffer));
	//Serial.println("Message going out:");
	//Serial.println(txBuffer.id, HEX);
	//Serial.println(txBuffer.flags.rtr, BIN);
	//for(uint8_t i = 0; i < txBuffer.length; ++i)
	//{
		//Serial.println(txBuffer.data[i], HEX);
	//}
	//Serial.println("=====");

    while(!sendMessage())
    {
    }
    return true;
}



void OLCB_CAN_Link::addVNode(OLCB_Virtual_Node *vnode)
{
	OLCB_Link::addVNode(vnode);
	_aliasHelper.allocateAlias(vnode->NID);
}

void OLCB_CAN_Link::removeVNode(OLCB_Virtual_Node *vnode)
{
	OLCB_Link::removeVNode(vnode);
	_aliasHelper.releaseAlias(vnode->NID);
}

bool OLCB_CAN_Link::sendMessage()
{
	//ASSUMPTION! We are assuming that the message to send has been stashed safely in txBuffer! This might not be true, in which case the behavior of this method is undefined.
	if(!can_check_free_buffer())
	{
        return false;
    }
    while(!can_send_message(&txBuffer));
    
    //now, send it to us!
    //Notice that the buffer DOES NOT get clobbered, as only one vnode is ever updated during the update loop; so long as sendMessage is only called once per vnode per update. Notice that sending a message from handleMessage will clobber the message currently being handled.
	memcpy(&rxBuffer,&txBuffer, sizeof(OLCB_CAN_Buffer)); //copy the message into the txBuffer
	rxBuffer.setInternal();
	    
    return true;
}

bool OLCB_CAN_Link::wasActiveSet()
{
    return wasActive;
}

void OLCB_CAN_Link::resetWasActive()
{
    wasActive = false;
}
