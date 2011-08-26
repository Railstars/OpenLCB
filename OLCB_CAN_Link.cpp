#include <WProgram.h>
//Code taken from LinkControl.cpp

#include "OLCB_CAN_Link.h"


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
    if(!_aliasHelper.allocateAlias(_nodeID))
    {
//      Serial.println("Couldn't initiate alias negotiation!");
      return false;
    }

    //check to see if any new messages require handling
//    if(can_get_message(&rxBuffer))
//    {
//        handleTransportLevel();
//      }
//  }
  return true;
}


// send the next CID message.  "i" is the 0-3 ordinal number of the message, which
// becomes F-C in the CID itself. Returns true if successfully sent.
bool OLCB_CAN_Link::sendCID(OLCB_NodeID *nodeID, uint8_t i) {
  if (!can_check_free_buffer()) return false;  // couldn't send just now
  uint16_t fragment;
  switch (i) {
    case 0:  fragment = ( (nodeID->val[0]<<4)&0xFF0) | ( (nodeID->val[1] >> 4) &0xF);
             break;
    case 1:  fragment = ( (nodeID->val[1]<<8)&0xF00) | ( nodeID->val[2] &0xF);
             break;
    case 2:  fragment = ( (nodeID->val[3]<<4)&0xFF0) | ( (nodeID->val[4] >> 4) &0xF);
             break;
    default:
    case 3:  fragment = ( (nodeID->val[4]<<8)&0xF00) | ( nodeID->val[5] &0xF);
             break;
  }
  txBuffer.setCID(i,fragment,nodeID->alias);
//  Serial.println("Sending CID");
  while(!can_send_message(&txBuffer));  // wait for queue, but earlier check says will succeed
  return true;
}

bool OLCB_CAN_Link::sendRID(OLCB_NodeID* nodeID) {
  if (!can_check_free_buffer()) return false;  // couldn't send just now  if (!isTxBufferFree()) return false;  // couldn't send just now
  txBuffer.setRID(nodeID->alias);
//  Serial.println("Sending RIM");
  while(!can_send_message(&txBuffer));  // wait for queue, but earlier check says will succeed
  return true;
}

bool OLCB_CAN_Link::sendInitializationComplete(OLCB_NodeID* nodeID) {
  if (!can_check_free_buffer()) return false;  // couldn't send just now  if (!isTxBufferFree()) return false;  // couldn't send just now
  txBuffer.setInitializationComplete(nodeID);
//  Serial.print("Assigning alias ");
//  Serial.print(getAlias(), DEC);
//  Serial.println(" to");
//  _NIDtoNegotiate->print();
//  Serial.println("==================");
  
  while(!can_send_message(&txBuffer));    // wait for queue, but earlier check says will succeed
  return true;
}

/////// Methods for sending and receiving OLCB packets over CAN

bool OLCB_CAN_Link::handleTransportLevel()
{
    
    if (rxBuffer.isCID())
    {
        //Serial.println("Someone's sent a CIM!");
        // somebody else trying to allocate, tell them
        _aliasHelper.handleCID(&rxBuffer);
    }
    else if (rxBuffer.isRID())
    {
        _aliasHelper.handleRID(&rxBuffer);
    }
//    else
//    {
//        Serial.println("This is a problem!");
//    }
    
    // see if this is a Verify request to us; first check type
    if (rxBuffer.isVerifyNID()) {
      // check address
//      Serial.println("It's a verify ID!");
      OLCB_NodeID n;
      rxBuffer.getNodeID(&n);
//      n.print();
      if (n == *_nodeID) //if the verification matches our nodeID, respond
      {
        // reply; should be threaded, but isn't
//        Serial.println("Sending verified NID for node");
//        _nodeID->print();
//        Serial.println("----");
        sendVerifiedNID(_nodeID);
        return true;
      }
      else //it might yet match a virtual NID; check each handler to see
      {
        bool flag = false;
        //check all handlers, not just datagram handlers
        OLCB_Virtual_Node *iter = _handlers;
        while(iter)
        {
//          Serial.println("checking iter to see if it will verify the ID");
          if(iter->verifyNID(&n))
          {
//            Serial.println("Yep! Sending verified NID for vnode");
//            iter->NID->print();
//            Serial.println("----");
            sendVerifiedNID(iter->NID);
            break;
          }
          else
          {
//            Serial.println("Nope! :(");
            iter = iter->next;
          }
        }
      }
//      Serial.println("Done, returning");
      return true;
    }
    // Maybe this is a global Verify request, in which case we have a lot of packets to send!
    else if (rxBuffer.isVerifyNIDglobal()) {
      // reply to global request
      // ToDo: This should be threaded
      sendVerifiedNID(_nodeID);
      //and again for all virtual nodes
      OLCB_Virtual_Node *iter = _handlers;
      while(iter != NULL)
      {
        if(iter->verifyNID(iter->NID))
        {
          sendVerifiedNID(iter->NID);
        }
        iter = iter->next;
      }
      return true;
    }
    // Perhaps it is someone sending a Verified NID packet. We might have requested that, in which case we should cache it
    else if (rxBuffer.isVerifiedNID()) {
    // We have a packet that contains a verified NID. We might have requested this. Let's check.
      OLCB_NodeID n;
      rxBuffer.getSourceNID(&n); //get the alias from the message header
      rxBuffer.getNodeID(&n); //Get the actual NID from the message body
//      Serial.println("Received a Verified Packet from");
//      n.print();
//      Serial.println("***Expected a Verified Packet from   ");
//      _nodeIDToBeVerified.print();
      if(n.alias == 0) return false;
      if(n == _nodeIDToBeVerified)
      {
//        Serial.println("Caching it");
        //add it to the NID/NIDa translation cache
        _translationCache.add(&n);
        //remove from temp buffer
        _nodeIDToBeVerified.set(0,0,0,0,0,0);
      }
//      else
//      {
//        Serial.println("Not going to cache it");
//      }
      return true;
    }
    else if(rxBuffer.isAMR()) //is someone releasing their alias? Remove it from the cache.
    {
      _translationCache.removeByAlias(rxBuffer.getSourceAlias());
    }
    return false;
}

void OLCB_CAN_Link::update(void)
{
  //check to see if any new messages require handling
  if(can_get_message(&rxBuffer))
  {
//    Serial.println("Got a message!");
//    Serial.println(rxBuffer.id, HEX);
//    for(int i = 0; i < rxBuffer.length; ++i)
//      Serial.println(rxBuffer.data[i],HEX);
//    Serial.println("==================");
    
    // See if this message is a CAN-level message that we should be handling.
    if(handleTransportLevel())
     //bail early, the packet grabbed isn't for any of the attached handlers to deal with.
      return;
    //otherwise, let's pass it on to our handlers
    
//    Serial.println("Not a message for Link to handle, should be passed on");
    //First, if there is a source for this message, see if we can pull the full NID from the cache!
    //TODO NONE OF THIS BELOW WILL WORK BECAUSE OLCB_CAN_Buffer NEVER STORES FULL NIDS! Which is silly!
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
    OLCB_Virtual_Node *iter = _handlers;
    while(iter)
    {
//      Serial.println("Trying a handler...");
      if(iter->handleFrame(&rxBuffer))
      {
//        Serial.println("Handler succeeded.");
        break;
      }
//      else
//        Serial.println("Handler refused.");
      iter = iter->next;
    }
  }

  //update alias allocation
  _aliasHelper.update();

  //OLCB_Virtual_Node::update();
  //call all our handler's update functions
  OLCB_Virtual_Node *iter = _handlers;
  while(iter)
  {
    iter->update();
    iter = iter->next;
  }
}

uint8_t OLCB_CAN_Link::sendDatagramFragment(OLCB_Datagram *datagram, uint8_t start)
{
  //datagram is the datagram to transmit.
  //start is the index of the next byte to start from.
  //returns the number of bytes sent.
//  Serial.println("sending datagram fragment to ");
//  datagram->destination.print();
  if(!datagram->destination.alias)
  {
    //try the cache
    uint16_t alias = _translationCache.getAliasByNID(&(datagram->destination));
    if(!alias) //not cached!
    {
//      Serial.println("Alias not in cache...");
      //need to ask
      sendNIDVerifyRequest(&(datagram->destination)); //if it can't go through, it'll get called again. no need to loop.
      return 0;
    }
//    else
//    {
//      Serial.print("found an alias in the cache! ");
//      Serial.println(datagram->destination.alias,DEC);
//    }
  }
  
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
//  Serial.print("=== ");
//  Serial.println(txBuffer.length+start);
  if(txBuffer.length+start < datagram->length) //not yet done!
  {
    txBuffer.setOpenLcbFormat(MTI_FORMAT_ADDRESSED_DATAGRAM);
//    Serial.println("Non-last datagram fragment");
  }
  else //last fragment!
  {
    txBuffer.setOpenLcbFormat(MTI_FORMAT_ADDRESSED_DATAGRAM_LAST);
//    Serial.println("last datagram fragment");
  }
//  Serial.println("Set up datagram buffer to send...");
//  Serial.println(txBuffer.id,HEX);

  while(!can_send_message(&txBuffer));
  
  return len;
}

bool OLCB_CAN_Link::sendNIDVerifyRequest(OLCB_NodeID *nid)
{
  //first, see if a request is pending
//  uint32_t time = millis();
  
//  Serial.println("=====");
//  Serial.println((time - _aliasCacheTimer), DEC);
//  Serial.println("=====");
  if(!_nodeIDToBeVerified.empty() && ((millis() - _aliasCacheTimer) < 2000) ) //it's not zeros, and it hasn't yet been a full second since the last request
  {
//    Serial.println("Previous request still outstanding, fail");
    return false;
  }  
  if (!can_check_free_buffer()){
//    Serial.println("No free buffer to xmit! fail.");
    return false;  // couldn't send just now
  }
  memcpy(&_nodeIDToBeVerified, nid, sizeof(OLCB_NodeID));
//  Serial.print("Sending request for NID using alias ");
//  Serial.println(_nodeID->alias,DEC);
//  _nodeIDToBeVerified.print();
//  nid->print();
  sendVerifiedNID(_nodeID);
  txBuffer.init(_nodeID);
  txBuffer.setVerifyNID(nid);
  while(!can_send_message(&txBuffer));  // wait for queue, but earlier check says will succeed
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
  txBuffer.data[0] = MTI_DATAGRAM_RCV_OK>>4;
  txBuffer.length = 1;
  while(!can_send_message(&txBuffer));
  return true;
}

bool OLCB_CAN_Link::nakDatagram(OLCB_NodeID *source, OLCB_NodeID *dest, int reason = DATAGRAM_REJECTED)
{
  if(!can_check_free_buffer())
    return false;
  txBuffer.init(source);
  txBuffer.setDestinationNID(dest);
  txBuffer.setFrameTypeOpenLcb();
  txBuffer.setOpenLcbFormat(MTI_FORMAT_ADDRESSED_NON_DATAGRAM);
  txBuffer.data[0] = MTI_DATAGRAM_REJECTED>>4;
  txBuffer.data[1] = (reason>>8)&0xFF;
  txBuffer.data[2] = reason&0xFF;
  txBuffer.length = 3;
  while(!can_send_message(&txBuffer));
  return true;
}

bool OLCB_CAN_Link::sendVerifiedNID(OLCB_NodeID *nid)
{
  if(!can_check_free_buffer())
    return false;
  txBuffer.init(nid);
  txBuffer.setVerifiedNID(nid);
  while(!can_send_message(&txBuffer));
  return true;
}

bool OLCB_CAN_Link::sendAMR(OLCB_NodeID *nid)
{
  if(!can_check_free_buffer())
    return false;
  txBuffer.init(nid);
  txBuffer.setAMR(nid->alias);
  while(!can_send_message(&txBuffer));
  return true;
}
