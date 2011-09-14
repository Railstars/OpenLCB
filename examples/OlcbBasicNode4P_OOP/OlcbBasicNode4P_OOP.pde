#include "ButtonLedDON.h"

#include <OLCB_Alias_Cache.h>
#include <OLCB_Buffer.h>
#include <OLCB_CAN_Buffer.h>
#include <OLCB_CAN_Link.h>
#include <OLCB_Datagram.h>
#include <OLCB_Datagram_Handler.h>
#include <OLCB_Event.h>
#include <OLCB_Event_Handler.h>
#include <OLCB_Link.h>
#include <OLCB_NodeID.h>
#include <OLCB_Stream.h>
#include <OLCB_Virtual_Node.h>
#include <OpenLCB.h>

#include <can.h>

//==============================================================
// OlcbBasicNode_OOP
// A prototype of a basic 4-Consumer-Producer OpenLCB board
// This implements 4 buttons and consumes/produces 8 fixed events
//
// D.E. Goodman 2011
// based on code by David Harris 2011
// based on code by Bob Jacobsen 2010
// and examples by Alex Shepherd and David Harris
//==============================================================

#define NID 2,1,13,0,0,3 // This nodes ID (DIY space)
#define EVENT_NID 2,1,13,0,0,1
OLCB_NodeID nodeid(NID); // Create the node structure with NID

// Define the events that this node uses.
// We define two events for each LED,
// one for turning it on,
// and one for turning it off
OLCB_Event events[] = {
        OLCB_Event(EVENT_NID,0,1), // Event to turn LED1 on
        OLCB_Event(EVENT_NID,0,2), // " " " off
        OLCB_Event(EVENT_NID,0,3), // Event to turn LED2 on
        OLCB_Event(EVENT_NID,0,4), // " " " off
        OLCB_Event(EVENT_NID,0,5), // ...
        OLCB_Event(EVENT_NID,0,6),
        OLCB_Event(EVENT_NID,0,7),
        OLCB_Event(EVENT_NID,0,8), // Event to turn LED4 off
};
int eventNum = 8;

// Define the input buttons
ButtonLed p14(14, LOW); // button on pin 14
ButtonLed p15(15, LOW); // button on pin 15
ButtonLed p16(16, LOW); // button on pin 16
ButtonLed p17(17, LOW); // button on pin 17
// and group them to match the events, two events per button


// Basic OpenLCB set-up
OLCB_CAN_Link link;

class MyEventHandler: public OLCB_Virtual_Node, public OLCB_Event_Handler
{
public:
    ButtonLed* buttons[8];
    bool states[4];
    
    void create(OLCB_Link *link, OLCB_NodeID *nid)
    {
      OLCB_Event_Handler::create(link,nid);
      OLCB_Virtual_Node::create(link,nid);
    }
    
    bool handleMessage(OLCB_Buffer *buffer)
    {
      return OLCB_Event_Handler::handleMessage(buffer);
    }

    void initialize(void)
    {
        buttons[0] = buttons[1] = &p14;
        buttons[2] = buttons[3] = &p15;
        buttons[4] = buttons[5] = &p16;
        buttons[6] = buttons[7] = &p17;
        
        states[0] = states[1] = states[2] = states[3] = false;
    }

    void update(void)
    {
      if(isPermitted())
      {
        // called from loop(), this looks at pins and
        // and decides which events to fire
        // with pce.produce(i);
        // The first event of each pair is sent on button down,
        // and second on button up.
        for (int i = 0; i<eventNum/2; i++) { // for each button and event-pair
            if (states[i] != buttons[i*2]->state){ // . did its state change?
                states[i] = buttons[i*2]->state; // .. if so, remember it and
                Serial.print("button "); Serial.println(i);
                if (states[i]) { // .. if the new state is down
                    produce(i*2); // ... send the first event of the pair
                } else { // .. else
                    produce(i*2+1); // .., send the second event
                }
            }
        }
        OLCB_Event_Handler::update(); //called last to permit the new events to be sent out immediately.      
      }
    }
};

MyEventHandler pce; //
// =============== Setup ===================

void setup()
{
    Serial.begin(115200);
    Serial.println("4CP_OOP");
    link.initialize();
    pce.create(&link, &nodeid);
    pce.initialize();
    pce.loadEvents(events, eventNum);
    for (int i=0; i<eventNum; i++) {
        pce.newEvent(i,true,false); // produce, consume
    }
    link.addVNode(&pce);
}

// ================ Loop ===================

void loop()
{
    // OpenLCB statndard processing:
    link.update();
    // read the buttons (implements debouncing, etc.)
    p14.process();
    p15.process();
    p16.process();
    p17.process();
}

// ---------------------------------------------------

