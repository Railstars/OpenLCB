#include "OLCB_Datagram_Handler.h"

bool OLCB_Datagram_Handler::sendDatagram(OLCB_Datagram *datagram)
{
    //someone wants to send a datagram! Check to see if our buffer is free, or two seconds have passed
    if(!_txDatagramBufferFree)
    {
    	//Serial.println("No free datagram tx buffer!");
        return false;
    }
    //Serial.println("settting dg buffer to occupied");
    _txDatagramBufferFree = false;
    //Copy the datagram to free the original up for other uses.
    memcpy(_txDatagramBuffer,datagram,sizeof(OLCB_Datagram));
    _txDatagramBuffer->source.copy(NID); //set the source to be me!
    _txFlag = true;
    _loc = 0;
    _sentTime = millis(); //log the final transmission time for response timeout checking

    //Serial.println(_txDatagramBuffer->length, DEC);
    //for(uint8_t i = 0; i < _txDatagramBuffer->length; ++i)
	    //Serial.println(_txDatagramBuffer->data[i], HEX);

    return true;
}

bool OLCB_Datagram_Handler::isDatagramSent(void)
{
    return(_txDatagramBufferFree);
}

bool OLCB_Datagram_Handler::handleMessage(OLCB_Buffer *frame)
{
	if(!isPermitted())
	{
		return false;
	}
	  //Serial.println("datagram handle message");
    //First, make sure we are dealing with a datagram.
    if(!frame->isDatagram())
    {
    	  //Serial.println("not a datagram");
        //see if it is an addressed non-datagram to us, and if it is an expected ACK or NAK
        if(frame->isDatagramAck())
        {
            OLCB_NodeID n;
            frame->getDestNID(&n);
            if(NID != 0 && n == *NID) //Yay! datagram sent OK
            {
            	//Serial.println("Got an ACK, freeing TX buffer");
                datagramResult(true,0);
                _txDatagramBufferFree = true;
                return true;
            }
        }
        else if(frame->isDatagramNak())
        {
        	OLCB_NodeID n;
            frame->getDestNID(&n);
            if(NID != 0 && n == *NID) //Yay! datagram sent OK, but NAK'd
            {
            	//Serial.println("It's a NAK!");
                uint16_t errorCode = frame->getDatagramNakErrorCode();
                if(errorCode & DATAGRAM_REJECTED_RESEND_MASK)
                {
                    	//Serial.println("resending datagram");
                        //send again!
                        _txFlag = true;
                        _loc = 0;
                }
                else
                {
                		//Serial.println("give up");
                        _txFlag = false;
                        _loc = 0;
                        _txDatagramBufferFree = true;
                }
                datagramResult(false,errorCode);
                return true;
            }
        }
        //Serial.println("Wasn't anything we care about, apparently");
        return false; //whatever it was, it wasn't for us!
    }

    //now, check to see if this datagram is addressed to us. If not, ignore it. (Also, reject if we don't have a NID set!)
    //Note: Sometimes we want to process a datagram regardless of the address. This is the case for what I call "default handlers". If we are marked as such, skip this check.
    OLCB_NodeID n;
    frame->getDestNID(&n);
    //Serial.print("got datagram fragment for ");
    //n.print();
    //NID->print();

    if((NID == 0) || (n != *NID))
    {
    	//Serial.println("Someone else");
        return false;
    }
    //Serial.println("us");


    //we've established it's a datagram, and that it is addressed to us, now what? One of several possibilities.
    // - we are ready to receive a new datagram (and this is the first frame, otherwise error). prep buffer, start recording.
    // - We are currently receiving a datagram (and this is from the same source). continue filling buffer.
    // - We are currently receiving a datagram (and this is not from the same source): send "busy, send again" response.
    // - This is the last frame, and we like it. Send ACK.
    // - This is the last frame, and it's not something we can handle. send NAK.

    // check the source for this datagram against our receive buffer
    frame->getSourceNID(&n);

    uint8_t *frame_data = frame->getData();;
    uint8_t frame_length = frame->getLength();
    if( (frame->isFirstDatagram() && _rxDatagramBufferFree) || (frame->isOnlyDatagram() && _rxDatagramBufferFree) ||
        ( (n == _rxDatagramBuffer->source) && !_rxDatagramBufferFree) ) //first frame and free buffer, or continuing frame and not free buffer
    {
        //begin filling!

        //is this the first frame? Need to initialize the datagram buffer
        if(frame->isFirstDatagram() || frame->isOnlyDatagram())
        {
        	//Serial.println("Is first fragment!");
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
            frame->getSourceNID(&(_rxDatagramBuffer->source)); //TODO NOTICE that this only gets us the alias, and not the entire NID!!!
            //_rxDatagramBuffer->source.print();
            //_rxDatagramBuffer->destination.print();
            //        n.print(); //TODO this one and the one above are DIFFERENT but should not be!!

        }

		//copy data from frame into _rxDatagramBuffer->data
        for (uint8_t i=0; i< frame_length; i++)
        {
            _rxDatagramBuffer->data[i+_rxDatagramBuffer->length] = *(frame_data+i);
        }
        _rxDatagramBuffer->length += frame_length;

        if(frame->isLastDatagram() || frame->isOnlyDatagram() ) //Last frame? Need to ACK or NAK!
        {
        	//Serial.println("Is last fragment!");
        	uint16_t errorcode = processDatagram();
        	//Serial.print("from processDatagram, received result code ");
        	//Serial.println(errorcode, HEX);
            if(errorcode == DATAGRAM_ERROR_OK)
            {
            	//TODO we should probably move this to the update loop! Don't want to block here, I don't think.
                //Serial.println("ACKING!");
                //TODO BROKEN! SHOULD NOT DO THIS!!!
                sendAck(&(_rxDatagramBuffer->source));
                //while(!_link->ackDatagram(NID,&(_rxDatagramBuffer->source)));
            }
            else
            {
            	//Serial.println("NAKING!");
            	//TODO BROKEN! SHOULD NOT DO THIS!!!
                sendNak(&(_rxDatagramBuffer->source), errorcode);
                //while(!_link->nakDatagram(NID,&(_rxDatagramBuffer->source), errorcode));
            }
            //Serial.println("freeing rx buffer");
            _rxDatagramBufferFree = true; //in either case, the buffer is now free
        }
        return true;
    }
    else if(frame->isLastDatagram() || frame->isOnlyDatagram()) //we can't currently accept this frame, either because the buffer is not free, or we didn't see the first datagram fragment
    {
    	if(_rxDatagramBufferFree) //we missed the first frame somehow; perhaps it came while we were processing an earlier datagram
    	{
    		//Serial.println("NAKing datagram, missed first frame");
    		sendNak(&n, DATAGRAM_REJECTED_OUT_OF_ORDER);
    		//while(!_link->nakDatagram(NID, &n, DATAGRAM_REJECTED_OUT_OF_ORDER));
    	}
    	else
    	{
    		//Serial.println("NAKing datagram, buffer full");
    		sendNak(&n, DATAGRAM_REJECTED_BUFFER_FULL);
        	//while(!_link->nakDatagram(NID, &n, DATAGRAM_REJECTED_BUFFER_FULL));
        }
        return true;
    }
    return true; //if we reach here, we have a middle DG fragment addressed to us; just don't take any kind of action whatsoever, including sending OptionalInteractionRejected (which is what would happen if we returned false
}

void OLCB_Datagram_Handler::sendNak(OLCB_NodeID *dest, uint16_t reason)
{
	memcpy(&_ackDest, dest, sizeof(OLCB_NodeID));
	_ackReason = reason;
}

void OLCB_Datagram_Handler::sendAck(OLCB_NodeID *dest)
{
	memcpy(&_ackDest, dest, sizeof(OLCB_NodeID));
	_ackReason = DATAGRAM_ERROR_OK;
}

void OLCB_Datagram_Handler::update(void)
{
	if(!isPermitted())
	{
		return;
	}
	else if(_ackReason != 0xFFFF) //we have an ack or nak to send
	{
		if(_ackReason == DATAGRAM_ERROR_OK)
		{
			while(!_link->ackDatagram(NID, &_ackDest));
		}
		else
		{
			while(!_link->nakDatagram(NID, &_ackDest, _ackReason));
		}
		_ackReason = 0xFFFF;
	}
	else if(_txFlag) //We're in the middle of a transmission
	{
			//Serial.println("in dghandler update");
			//Serial.println("In middle of transmission");
			uint8_t sent = _link->sendDatagramFragment(_txDatagramBuffer, _loc);
			//TODO We should be checking for a 0 result here, as that might indicate trouble
			if(!sent && (millis()-_sentTime) > DATAGRAM_ACK_TIMEOUT) //if nothing got transmitted, let's not get hung up. might not happen. Let it go for a couple of seconds, then give up
			{
					//give up
				  //Serial.println("DG transmission timeout reached, freeing buffer");
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
    	//Serial.println("ACK timeout reached, freeing buffer");
        _txDatagramBufferFree = true;
        _txFlag = false;
        datagramResult(false,DATAGRAM_ERROR_ACK_TIMEOUT);
    }
}

void OLCB_Datagram_Handler::clearBuffer(OLCB_NodeID *nodeid)
{
	if((!_rxDatagramBufferFree) && (*nodeid == _rxDatagramBuffer->source))
	{
		//Serial.println("DG clearing buffer");
		_rxDatagramBufferFree = true; //kill transmission
	}
}
