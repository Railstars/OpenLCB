/*
 * OLCB_Event_Handler.h
 *
 *  Created on: Sep 1, 2011
 *      Author: dgoodman
 */

#ifndef OLCB_EVENT_HANDLER_H_
#define OLCB_EVENT_HANDLER_H_

#include "Arduino.h"
#include "OLCB_NodeID.h"
#include "OLCB_Event.h"
#include "OLCB_Link.h"
#include "OLCB_Virtual_Node.h"

// Mark as waiting to have Identify sent
#define IDENT_FLAG 0x01
// Mark produced event for send
#define PRODUCE_FLAG 0x02
// Mark entry as really empty, ignore
#define EMPTY_FLAG 0x04
// Mark entry to written from next learn message
#define LEARN_FLAG 0x08
// Mark entry to send a learn message
#define TEACH_FLAG 0x10

class OLCB_Event_Handler: public OLCB_Handler
{
public:
    virtual void update(void); //this method should be overridden to detect conditions for the production of events

    void loadEvents(OLCB_Event* events, uint16_t numEvents);

    bool produce(uint16_t index); //OLCB_Event *event); //call to produce an event with ID = EID

    //this method should be overridden to handle the consumption of events.
    virtual bool consume(uint16_t index);

    /* Protocol level interactions for every kind of virtual node */
    bool handleMessage(OLCB_Buffer *buffer);

    /* I don't understand what this method does, so I just copied it directly */
    void newEvent(int index, bool p, bool c);
    
    void markToLearn(int index, bool mark);
    void markToTeach(int index, bool mark);
    bool markedToLearn(int index);
    bool markedToTeach(int index);


protected:
    OLCB_Event *_events; //an array of events
    uint16_t _numEvents; //the size of the above array
    uint16_t _sendEvent; //used as an index into _numEvents
    bool handleIdentifyEvents(void);
    bool handleLearnEvent(OLCB_Event *event);
    bool handlePCEventReport(OLCB_Event *event);
    bool handleIdentifyConsumers(OLCB_Event *event);
    bool handleIdentifyProducers(OLCB_Event *event);

    //override to save/load the currently learned events
    virtual bool store(void);
    virtual bool load(void);
};

#endif /* OLCB_EVENT_HANDLER_H_ */
