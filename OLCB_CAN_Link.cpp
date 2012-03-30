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

bool OLCB_CAN_Link::sendRID(OLCB_NodeID* nodeID) {
  if (!can_check_free_buffer()) return false;  // couldn't send just now
  txBuffer.setRID(nodeID->alias);
  memcpy(txBuffer.data, nodeID->val, 6); //This seems an important part of the message that is being left off!
  while(!sendMessage());  // wait for queue, but earlier check says will succeed
  return true;
}

bool OLCB_CAN_Link::sendInitializationComplete(OLCB_NodeID* nodeID) {
  if (!can_check_free_buffer()) return false;  // couldn't send just now
  txBuffer.setInitializationComplete(nodeID);
  
  while(!sendMessage());    // wait for queue, but earlier check says will succeed
  return true;
}

bool OLCB_CAN_Link::sendRejectOptionalInteraction(OLCB_NodeID* source, OLCB_NodeID* dest)
{
  if (!can_check_free_buffer()) return false;  // couldn't send just now
  txBuffer.setRejectOptionalInteraction(source, dest);
  while(!sendMessage());  // wait for queue, but earlier check says will succeed
  return true;
}

/////// Methods for sending and receiving OLCB packets over CAN

bool OLCB_CAN_Link::handleTransportLevel()
{
	//Serial.println("handleTransportLevel");
	OLCB_NodeID n(0,0,0,0,0,0);
    // see if this is a Verify request to us; first check type
    if (rxBuffer.isVerifyNIDGlobal() || rxBuffer.isVerifyNIDAddressed())
    {
      // check address
      rxBuffer.getNodeID(&n);
      //pass it off to the alias helper, since it maintains a definitive list of registered nodeIDs
      _aliasHelper.verifyNID(&n);
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
    }
    else if(rxBuffer.isAMR()) //is someone releasing their alias? Remove it from the cache.
    {
      _translationCache.removeByAlias(rxBuffer.getSourceAlias());
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
  else if(can_get_message(&rxBuffer))
  {
  	//Serial.println("Got message on wire");
  	//Serial.println(rxBuffer.id, HEX);
	rxBuffer.setExternal();
	deliverMessage();
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
    if(rxBuffer.getDestinationNID(&n))
    {
      //see if we can pull the actual NID from the cache
      if(_translationCache.getNIDByAlias(&n))
        rxBuffer.setDestinationNID(&n);
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
    if(rxBuffer.isExternal() && dest_node && !handled) //if it came from the outside, and was addressed to one of ours, and it went unhandled, then:
    {
    	//Serial.println("Message was external, addressed to us, and not handled");
    	rxBuffer.getSourceNID(&n);
    	sendRejectOptionalInteraction(dest_node, &n);
    }
}

uint8_t OLCB_CAN_Link::sendDatagramFragment(OLCB_Datagram *datagram, uint8_t start)
{
  //datagram is the datagram to transmit.
  //start is the index of the next byte to start from.
  //returns the number of bytes sent.
  //Serial.println("sendDGfragment");
  //datagram->destination.print();
  if(!datagram->destination.alias)
  {
    //try the cache
    uint16_t alias = _translationCache.getAliasByNID(&(datagram->destination));
    //Serial.println(alias, HEX);
    if(!alias) //not cached!
    {
      //need to ask
      //Serial.println("Link: Gonna have to ask with a VerifyNID");
      sendVerifyNID(&(datagram->source), &(datagram->destination)); //if it can't go through, it'll get called again. no need to loop.
      return 0;
    }
  }
  //datagram->destination.print();
  //now, figure out how many bytes remain, and whether this is the last fragment that needs to be sent.
  // Notice that the CAN link can send 8 bytes per frame.
  //set the source, and init the buffer.
  txBuffer.init(&(datagram->source));
  //set the destination
  txBuffer.setDestinationNID(&(datagram->destination));
  txBuffer.setFrameTypeOpenLcb();
  uint8_t len = min(datagram->length-start,8);
  txBuffer.length = len;
  for (uint8_t i = 0; i<txBuffer.length; i++)
         txBuffer.data[i] = datagram->data[i+start];
  if(txBuffer.length+start < datagram->length) //not yet done!
  {
    txBuffer.setOpenLcbFormat(MTI_FORMAT_ADDRESSED_DATAGRAM);
  }
  else //last fragment!
  {
    txBuffer.setOpenLcbFormat(MTI_FORMAT_ADDRESSED_DATAGRAM_LAST);
  }

  while(!sendMessage());
  
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
  //txBuffer.init(_nodeID);
  txBuffer.init(src); //set the source NID to the requesting NID
  txBuffer.setVerifyNID(request);
  while(!sendMessage());  // wait for queue, but earlier check says will succeed
  _aliasCacheTimer = millis(); //set the clock going. A request will only be permitted to stand for 1 second.
  return true;
}

bool OLCB_CAN_Link::ackDatagram(OLCB_NodeID *source, OLCB_NodeID *dest)
{
  if(!can_check_free_buffer())
    return false;
  txBuffer.init(source);
  txBuffer.setDestinationNID(dest);
  txBuffer.setFrameTypeOpenLcb();
  txBuffer.setOpenLcbFormat(MTI_FORMAT_ADDRESSED_NON_DATAGRAM);
  txBuffer.data[0] = MTI_DATAGRAM_RCV_OK;
  txBuffer.length = 1;
  //Serial.println("Sending ACK");
  //Serial.println(txBuffer.data[0], HEX);
  while(!sendMessage());
  return true;
}

bool OLCB_CAN_Link::nakDatagram(OLCB_NodeID *source, OLCB_NodeID *dest, int reason = DATAGRAM_REJECTED)
{
	//Serial.println("Sending Datagram NAK");
  if(!can_check_free_buffer())
    return false;
  txBuffer.init(source);
  txBuffer.setDestinationNID(dest);
  txBuffer.setFrameTypeOpenLcb();
  txBuffer.setOpenLcbFormat(MTI_FORMAT_ADDRESSED_NON_DATAGRAM);
  txBuffer.data[0] = MTI_DATAGRAM_REJECTED;
  txBuffer.data[1] = (reason>>8)&0xFF;
  txBuffer.data[2] = reason&0xFF;
  //Serial.println(txBuffer.data[1], HEX);
  //Serial.println(txBuffer.data[2], HEX);
  txBuffer.length = 3;
  while(!sendMessage());
  return true;
}

bool OLCB_CAN_Link::sendVerifiedNID(OLCB_NodeID *nid)
{
//	//Serial.println("Attempting to send VerifiedNID!");
  if(!can_check_free_buffer())
    return false;
  txBuffer.init(nid);
  txBuffer.setVerifiedNID(nid);
  while(!sendMessage());
  return true;
}

bool OLCB_CAN_Link::sendAMR(OLCB_NodeID *nid)
{
  if(!can_check_free_buffer())
    return false;
  txBuffer.init(nid);
  txBuffer.setAMR(nid);
  while(!sendMessage());
  return true;
}

bool OLCB_CAN_Link::sendAMD(OLCB_NodeID *nid)
{
  if(!can_check_free_buffer())
    return false;
  txBuffer.init(nid);
  txBuffer.setAMD(nid);
  while(!sendMessage());
  return true;
}


bool OLCB_CAN_Link::sendIdent(void)
{
	OLCB_Event e(0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x00);
	return sendPCER(&e);
}

bool OLCB_CAN_Link::sendPCER(OLCB_Event *event)
{
    if(!can_check_free_buffer())
        return false;
    txBuffer.setPCEventReport(event);
    while(!sendMessage());
    return true;
}

bool OLCB_CAN_Link::sendConsumerIdentified(OLCB_Event *event)
{
	//Serial.println("sending Consumer Identified");
    if(!can_check_free_buffer())
    {
        return false;
    }
    txBuffer.setConsumerIdentified(event);
    while(!sendMessage());
    return true;
}

bool OLCB_CAN_Link::sendLearnEvent(OLCB_Event *event)
{
    if(!can_check_free_buffer())
        return false;
    txBuffer.setLearnEvent(event);
    while(!sendMessage()); //TODO make a new method for sendMessage that also repeats it back to the link!
    return true;
}

bool OLCB_CAN_Link::sendProducerIdentified(OLCB_Event *event)
{    
	//Serial.println("sending Producer Identified");
    if(!can_check_free_buffer())
    {
        return false;
    }
    txBuffer.setProducerIdentified(event);
    while(!sendMessage())
    {
    }
    return true;
}

bool OLCB_CAN_Link::sendMessage(OLCB_Buffer *msg)
{
	if(!can_check_free_buffer())
    {
        return false;
    }
    Serial.println("memcpy sendmessage");
    memcpy(&txBuffer, (OLCB_CAN_Buffer*)msg, sizeof(OLCB_CAN_Buffer));
	Serial.println("Message going out:");
	Serial.println(txBuffer.id, HEX);
	Serial.println(txBuffer.flags.rtr, BIN);
	for(uint8_t i = 0; i < txBuffer.length; ++i)
	{
		Serial.println(txBuffer.data[i], HEX);
	}
	Serial.println("=====");

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
	memcpy(&rxBuffer,&txBuffer, sizeof(OLCB_CAN_Buffer)); //copy the message into the txBuffer
	rxBuffer.setInternal();
    //deliverMessage(); //send the message to local nodes
    
    return true;
}
