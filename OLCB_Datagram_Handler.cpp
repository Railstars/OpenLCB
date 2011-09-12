#include "OLCB_Datagram_Handler.h"

void OLCB_Datagram_Handler::setLink(OLCB_Link *newLink)
{
    OLCB_Virtual_Node::setLink(newLink);
    _rxDatagramBuffer->destination.copy(NID);
    _txDatagramBuffer->source.copy(NID);
}

void OLCB_Datagram_Handler::setNID(OLCB_NodeID *newNID)
{
    OLCB_Virtual_Node::setNID(newNID);
    _rxDatagramBuffer->destination.copy(NID);
    _txDatagramBuffer->source.copy(NID);
}

bool OLCB_Datagram_Handler::sendDatagram(OLCB_Datagram *datagram)
{
    //someone wants to send a datagram! Check to see if our buffer is free, or two seconds have passed
    if(!_txDatagramBufferFree)
    {
        return false;
    }

    _txDatagramBufferFree = false;
    //Copy the datagram to free the original up for other uses.
    memcpy(_txDatagramBuffer,datagram,sizeof(OLCB_Datagram));
    _txDatagramBuffer->source.copy(NID); //set the source to be me!
    _txFlag = true;
    _loc = 0;
    _sentTime = millis(); //log the final transmission time for response timeout checking

    return true;
}

bool OLCB_Datagram_Handler::isDatagramSent(void)
{
    return(_txDatagramBufferFree);
}

bool OLCB_Datagram_Handler::handleFrame(OLCB_Buffer *frame)
{
    //First, make sure we are dealing with a datagram.
    if(!frame->isDatagram())
    {
        //see if it is an addressed non-datagram to us, and if it is an expected ACK or NAK
        if(frame->isDatagramAck())
        {
            OLCB_NodeID n;
            frame->getDestinationNID(&n);
            if(NID != 0 && n == *NID) //Yay! datagram sent OK
            {
                datagramResult(true,0);
                _txDatagramBufferFree = true;
                return true;
            }
        }
        else if(frame->isDatagramNak())
        {
            OLCB_NodeID n;
            frame->getDestinationNID(&n);
            if(NID != 0 && n == *NID) //Yay! datagram sent OK, but NAK'd
            {
                uint16_t errorCode = frame->getDatagramNakErrorCode();
                switch(errorCode)
                {
                    case DATAGRAM_REJECTED_BUFFER_FULL:
                        //send again!
                        _txFlag = true;
                        _loc = 0;
                        break;
                    default:  //give up.
                        _txFlag = false;
                        _loc = 0;
                        _txDatagramBufferFree = true;
                }
                datagramResult(false,errorCode);
                return true;
            }
        }
        return false; //whatever it was, it wasn't for us!
    }

    //now, check to see if this datagram is addressed to us. If not, ignore it. (Also, reject if we don't have a NID set!)
    //Note: Sometimes we want to process a datagram regardless of the address. This is the case for what I call "default handlers". If we are marked as such, skip this check.
    OLCB_NodeID n;
    frame->getDestinationNID(&n);
    if((NID == 0) || (n != *NID))
    {
        return false;
    }


    //we've established it's a datagram, and that it is addressed to us, now what? One of several possibilities.
    // - we are ready to receive a new datagram (and this is the first frame, otherwise error). prep buffer, start recording.
    // - We are currently receiving a datagram (and this is from the same source). continue filling buffer.
    // - We are currently receiving a datagram (and this is not from the same source): send "busy, send again" response.
    // - This is the last frame, and we like it. Send ACK.
    // - This is the last frame, and it's not something we can handle. send NAK.

    // check the source for this datagram against our receive buffer
    frame->getSourceNID(&n);
    if(_rxDatagramBufferFree || n == _rxDatagramBuffer->source)
    {
        //begin filling!

        //is this the first frame? Need to initialize the datagram buffer
        if(_rxDatagramBufferFree)
        {
            _rxDatagramBufferFree = false;
            //TODO FOR SOME REASON THE FOLLOWING LINE (or variants on it) CORRUPT THE FIRST TWO BYTES OF
            //_rxDatagramBuffer->source to be 0xc6. WHY GOD WHY!?
            //      memcpy(_rxDatagramBuffer->source, &n, sizeof(OLCB_NodeID));
            //        _rxDatagramBuffer->source->alias = n.alias;
            //        _rxDatagramBuffer->source->val[0] = n.val[0];
            //        _rxDatagramBuffer->source->val[0] = n.val[1];
            //        _rxDatagramBuffer->source->val[0] = n.val[2];
            //        _rxDatagramBuffer->source->val[0] = n.val[3];
            //        _rxDatagramBuffer->source->val[0] = n.val[4];
            //        _rxDatagramBuffer->source->val[0] = n.val[5];
            //      n.print();
            //      _rxDatagramBuffer->source->print();
            _rxDatagramBuffer->length = 0;
            frame->getSourceNID(&(_rxDatagramBuffer->source));
            //        _rxDatagramBuffer->source.print();
            //        n.print(); //TODO this one and the one above are DIFFERENT but should not be!!

        }

        for (int i=0; i< frame->length; i++) {
            _rxDatagramBuffer->data[i+_rxDatagramBuffer->length] = frame->data[i];
        }
        _rxDatagramBuffer->length += frame->length;

        if(frame->isLastDatagram()) //Last frame? Need to ACK or NAK!
        {
            if(processDatagram())
            {
                //Something isn't quite right here, not getting the right source NID? The alias is right...
                //        _rxDatagramBuffer->source->print();
                //        n.print(); //TODO this one and the one above are DIFFERENT but should not be!!
                while(!_link->ackDatagram(NID,&(_rxDatagramBuffer->source)));
            }
            else
            {
                while(!_link->nakDatagram(NID,&(_rxDatagramBuffer->source), DATAGRAM_REJECTED));
            }
            _rxDatagramBufferFree = true; //in either case, the buffer is now free
        }
    }
    else //we can't currently accept this frame, because the buffer is not free
    {
        while(!_link->nakDatagram(NID,&(_rxDatagramBuffer->source), DATAGRAM_REJECTED_BUFFER_FULL));
    }
}


void OLCB_Datagram_Handler::update(void)
{
    OLCB_Virtual_Node::update();
    if(!_initialized)  //check to see that we're initialized and ready to go.
    {
        return;
    }

    if(_txFlag) //We're in the middle of a transmission
    {
        uint8_t sent = _link->sendDatagramFragment(_txDatagramBuffer, _loc);
        //TODO We should be checking for a 0 result here, as that might indicate trouble
        if(!sent && (millis()-_sentTime) > DATAGRAM_ACK_TIMEOUT) //if nothing got transmitted, let's not get hung up. might not happen. Let it go for a couple of seconds, then give up
        {
            //give up
            _txDatagramBufferFree = true;
            _txFlag = false;
            datagramResult(false,DATAGRAM_ERROR_ABORTED);
        }
        else if(_loc+sent == _txDatagramBuffer->length) //we're done with this datagram
        {
            _txFlag = false; //done transmitting
            _loc = 0; //reset the location
            _sentTime = millis(); //log the final transmission time for response timeout checking
        }
        else
        {
            _loc += sent;
        }
    }

    //If we're not transmitting, but awaiting a response, make sure that the response hasn't timed out!
    else if(!_txDatagramBufferFree && ((millis()-_sentTime) > DATAGRAM_ACK_TIMEOUT) )
    {
        _txDatagramBufferFree = true;
        _txFlag = false;
        datagramResult(false,DATAGRAM_ERROR_ACK_TIMEOUT);
    }
}
