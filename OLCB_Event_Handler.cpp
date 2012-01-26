/*
 * OLCB_Event_Handler.cpp
 *
 *  Created on: Sep 1, 2011
 *      Author: dgoodman
 *
 *  Much of this code taken from PCE.h and PCE.cpp, written by who, I know not. Bob Jacobsen perhaps?
 */

#include "OLCB_Event_Handler.h"


void OLCB_Event_Handler::update(void) //this method should be overridden to detect conditions for the production of events
{
    // see if any replies are waiting to send
    while (_sendEvent < _numEvents)
    {
    	Serial.print("    i = ");
        Serial.print(_sendEvent, DEC);
        Serial.print(" ");
        Serial.print(_events[_sendEvent].flags, HEX);
        Serial.print(":  ");
        // OK to send, see if marked for some cause
        // ToDo: This only sends _either_ producer ID'd or consumer ID'd, not both
        if ( (_events[_sendEvent].flags & (IDENT_FLAG | OLCB_Event::CAN_PRODUCE_FLAG)) == (IDENT_FLAG | OLCB_Event::CAN_PRODUCE_FLAG))
        {
        	Serial.println("identify producer");
            if(_link->sendProducerIdentified(&_events[_sendEvent]))
            {
            	Serial.println(_events[_sendEvent].flags, HEX);
            	Serial.println("reset IDENT flag");
            	_events[_sendEvent].flags &= ~IDENT_FLAG;    // reset flag
            	Serial.println(_events[_sendEvent].flags, HEX);
            }
            else
            {
            	Serial.println("failed");
            }
            break; // only send one from this loop
        }
        else if ( (_events[_sendEvent].flags & (IDENT_FLAG | OLCB_Event::CAN_CONSUME_FLAG)) == (IDENT_FLAG | OLCB_Event::CAN_CONSUME_FLAG))
        {
        	Serial.println("identify consumer");
            if(_link->sendConsumerIdentified(&_events[_sendEvent]))
            {
            	_events[_sendEvent].flags &= ~IDENT_FLAG;    // reset flag
            }
            break; // only send one from this loop
        }
        else if (_events[_sendEvent].flags & PRODUCE_FLAG)
        {
	        Serial.println("produce");
            if(_link->sendPCER(&_events[_sendEvent]))
         		_events[_sendEvent].flags &= ~PRODUCE_FLAG;    // reset flag   
            break; // only send one from this loop
        }
        else if (_events[_sendEvent].flags & TEACH_FLAG)
        {
			Serial.println("teach");
            _link->sendLearnEvent(&_events[_sendEvent]);
            	_events[_sendEvent].flags &= ~TEACH_FLAG;    // reset flag
            break; // only send one from this loop
        }
        else
        {
			Serial.println("skip");
            // just skip
            ++_sendEvent;
        }
    }
}

void OLCB_Event_Handler::loadEvents(OLCB_Event* events, uint16_t numEvents)
{
    _events = events;
    _numEvents = numEvents;
    Serial.println("loadEvents");
    for(uint16_t i = 0; i < _numEvents; ++i)
    {
    	_events[i].flags = 0;
    	Serial.print(i, DEC);
    	Serial.print(" ");
    	Serial.println(_events[i].flags, HEX);
    }
}

bool OLCB_Event_Handler::produce(uint16_t index) //OLCB_Event *event) //call to produce an event with ID = EID
{
	Serial.print("Producing ");
	Serial.println(index);
	bool retval = false;
//	int8_t index = 0;
//	while (-1 != (index = event->findIndexInArray(_events, _numEvents, index)))
//    {
//        // yes, we have to reply with ConsumerIdentified
        if (_events[index].flags & OLCB_Event::CAN_PRODUCE_FLAG)
        {
        	retval = true;
            _events[index].flags |= PRODUCE_FLAG;
            _sendEvent = min(_sendEvent, index);
        }
//        ++index;
//        if (index>=_numEvents) break;
//    }
    return retval;
}

//this method should be overridden to handle the consumption of events.
//return true if we were able to successfully consume the event, false otherwise
bool OLCB_Event_Handler::consume(OLCB_Event *event)
{
    return false;
}


/* Protocol level interactions for every kind of virtual node */
bool OLCB_Event_Handler::handleMessage(OLCB_Buffer *buffer)
{
	Serial.println("Event handler handling message!");
    bool retval = false;
    //first, check to see if it is an event
    if(buffer->isPCEventReport()) //it's what is colloquially known as an event! Or technically as a Producer-Consumer Event Report (!?)
    {
        //great, get the EID, and call consume
        OLCB_Event event;
        buffer->getEventID(&event);
        //check whether the event is in our array
        retval = handlePCEventReport(&event);
    }

    else if(buffer->isIdentifyProducers()) //we may need to respond to this one
    {
        // see if we produce the listed event
        OLCB_Event event;
        buffer->getEventID(&event);
        retval = handleIdentifyProducers(&event);
    }

    else if(buffer->isIdentifyConsumers())
    {
        // see if we consume the listed event
        OLCB_Event event;
        buffer->getEventID(&event);
        retval = handleIdentifyConsumers(&event);
    }

    else if(buffer->isIdentifyEvents())
    {
        // See if addressed to us
        OLCB_NodeID n;
        buffer->getNodeID(&n);
        retval = handleIdentifyEvents(&n);
    }

    else if(buffer->isLearnEvent())
    {
        OLCB_Event event;
        buffer->getEventID(&event);
        retval = handleLearnEvent(&event);
    }

    return retval; //default: not something for us to handle!
}

bool OLCB_Event_Handler::handleIdentifyEvents(OLCB_NodeID *nodeid)
{
    bool retval = false;
    if (&nodeid == &NID) //compare to our address
    {
        retval = true;
        // if so, send _all_ ProducerIndentfied, ConsumerIdentified
        for (int i = 0; i < _numEvents; ++i) {
            _events[i].flags |= IDENT_FLAG;
        }
        _sendEvent = 0;
    }
    return retval;
}

bool OLCB_Event_Handler::handlePCEventReport(OLCB_Event *event)
{
    bool retval = false;
    int16_t index = 0;
    while (-1 != (index = event->findIndexInArray(_events, _numEvents, index)))
    {
        if (_events[index].flags & OLCB_Event::CAN_CONSUME_FLAG)
        {
            // yes, notify our own code
            if(consume(event))
            {
                retval = true;
            }
        }
        ++index;
        if (index>=_numEvents)
        {
            break;
        }
    }
    return retval;
}

bool OLCB_Event_Handler::handleIdentifyProducers(OLCB_Event *event)
{
    bool retval = false;
    int16_t index = 0;
    // find producers of event
    while (-1 != (index = event->findIndexInArray(_events, _numEvents, index)))
    {
        // yes, we have to reply with ProducerIdentified
        if (_events[index].flags & OLCB_Event::CAN_PRODUCE_FLAG)
        {
            retval = true;
            _events[index].flags |= IDENT_FLAG;
            _sendEvent = min(_sendEvent, index);
        }
        ++index;
        if (index>=_numEvents) break;
    }
    // ToDo: add identify flags so that events that are both produced and consumed
    // have only one form sent in response to a specific request.
    return retval;
}

bool OLCB_Event_Handler::handleIdentifyConsumers(OLCB_Event *event)
{
    bool retval = false;
    int index = 0;
    // find consumers of event
    while (-1 != (index = event->findIndexInArray(_events, _numEvents, index)))
    {
        // yes, we have to reply with ConsumerIdentified
        if (_events[index].flags & OLCB_Event::CAN_CONSUME_FLAG)
        {
            retval = true;
            _events[index].flags |= IDENT_FLAG;
            _sendEvent = min(_sendEvent, index);
        }
        ++index;
        if (index>=_numEvents) break;
    }
    return retval;
}

bool OLCB_Event_Handler::handleLearnEvent(OLCB_Event *event)
{
    bool save = false;
    for (uint16_t i=0; i<_numEvents; ++i)
    {
        if ( (_events[i].flags & LEARN_FLAG ) != 0 )
        {
            memcpy(_events[i].val, event->val, 8);
            _events[i].flags |= IDENT_FLAG; // notify new eventID
            _events[i].flags &= ~LEARN_FLAG; // enough learning
            _sendEvent = min(_sendEvent, i);
            save = true;
        }
    }

    if (save)
    {
        store(); //TODO check for success here!
    }
    return save;
}

void OLCB_Event_Handler::newEvent(int index, bool p, bool c)
{

  _events[index].flags |= IDENT_FLAG;
  _sendEvent = min(_sendEvent, index);
  if (p) _events[index].flags |= OLCB_Event::CAN_PRODUCE_FLAG;
  if (c) _events[index].flags |= OLCB_Event::CAN_CONSUME_FLAG;


  Serial.print("new event ");
      	Serial.print(index, DEC);
    	Serial.print(" ");
    	Serial.println(_events[index].flags, HEX);


}


bool OLCB_Event_Handler::store(void)
{
    return false; //false for not saved!
}

bool OLCB_Event_Handler::load(void)
{
    return false;
}
