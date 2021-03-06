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

class MyEventHandler: public OLCB_Event_Handler
{
public:
    ButtonLed* buttons[8];
    
    void initialize(void)
    {
        buttons[0] = buttons[1] = &p14;
        buttons[2] = buttons[3] = &p15;
        buttons[4] = buttons[5] = &p16;
        buttons[6] = buttons[7] = &p17;
    }

    bool consume(OLCB_Event *event)
    {
      // Link received PC events to specific actions of this node
      // This routine is aautomatically called if an event is received that
      // matches one of the node's events
      // invoked when an event is consumed
      // index indicates which event from the events array.
      int index = event->findIndexInArray(_events, _numEvents);
      if(index == -1)
          return false;
      Serial.print ("Consuming ");
      Serial.println(index, DEC);
      buttons[index]->on(index&0x1 ? 0x0L : ~0x0L ); // turn odd events off, and even events on
      return true;
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
        pce.newEvent(i,false,true); // produce, consume
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

