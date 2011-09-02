#include <can.h>
#include <OpenLCB.h>
#include "ButtonLed.h"

//==============================================================
// OlcbBasicNode_OOP
// A prototype of a basic 4-Producer OpenLCB board
// This implements 4 buttons and generates 8 fixed events
//
// D.E. Goodman 2011
// based on code by David Harris 2011
// based on code by Bob Jacobsen 2010
// and examples by Alex Shepherd and David Harris
//==============================================================

#define NID 2,1,13,0,0,1 // This nodes ID (DIY space)
OLCB_NodeID nodeid(NID); // Create the node structure with NID

// PC Events this node produces
// We define two events for each button:
// one for when the button is depressed, and
// another for when it is released
OLCB_Event events[] = {
        OLCB_Event(NID,0,1), // Event for Button14 pressed
        OLCB_Event(NID,0,2), // " " " released
        OLCB_Event(NID,0,3), // Event for Button15 pressed
        OLCB_Event(NID,0,4), // ...
        OLCB_Event(NID,0,5),
        OLCB_Event(NID,0,6),
        OLCB_Event(NID,0,7),
        OLCB_Event(NID,0,8), // Event for Button17 pressed
};
int eventNum = 8;

// Define the input buttons
ButtonLed p14(14, LOW); // button on pin 14
ButtonLed p15(15, LOW); // button on pin 15
ButtonLed p16(16, LOW); // button on pin 16
ButtonLed p17(17, LOW); // button on pin 17
// and group them to match the events, two events per button


// Basic OpenLCB set-up
OLCB_CAN_Link link(&nodeid);

class MyEventHandler: public OLCB_Event_Handler
{
public:
    ButtonLed* buttons[8];
    bool states[4];
    void init(void)
    {
        OLCB_Event_Handler::init();
        buttons[0] = buttons[1] = &p14;
        buttons[2] = buttons[3] = &p15;
        buttons[4] = buttons[5] = &p16;
        buttons[6] = buttons[7] = &p17;

        states[0] = states[1] = states[2] = states[3] = false;
    }
    void update(void)
    {
        // called from loop(), this looks at pins and
        // and decides which events to fire
        // with pce.produce(i);
        // The first event of each pair is sent on button down,
        // and second on button up.
        for (int i = 0; i<eventNum/2; i++) { // for each button and event-pair
            if (states[i] != buttons[i*2]->state) { // . did its state change?
                states[i] = buttons[i*2]->state; // .. if so, remember it and
                //Serial.print("button "); Serial.println(i);
                if (states[i]) { // .. if the new state is down
                    produce(&_events[i*2]); // ... send the first event of the pair
                } else { // .. else
                    produce(&_events[i*2+1]); // .., send the second event
                }
            }
        }
    }
};

MyEventHandler pce; // create the producer/condumer data structure

// Link the button presses (triggers) to PC events
bool states[] = {false, false, false, false};


// =============== Setup ===================

void setup()
{
    link.initialize();
    pce.setNID(&nodeid);
    pce.setLink(&link);
    pce.loadEvents(events, 8);
    pce.init();
    for (int i=0; i<eventNum; i++) {
        pce.newEvent(i,true,false); // produce, consume
    }
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

