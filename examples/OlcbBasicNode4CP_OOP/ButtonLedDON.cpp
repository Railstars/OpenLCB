#include "WConstants.h"
#include "ButtonLedDON.h"

void ButtonLed::init(uint8_t s) {
  sense = s;
  pinMode(pin,OUTPUT);                    // default to driving LED..
  digitalWrite(pin,sense);                // ..and turn it on
  lastState=false;                        // start with led off
  duration=lastDuration=0;                // init durations
  state=false;                            // start with button off
}


ButtonLed::ButtonLed(const uint8_t p,uint8_t s) : pin(p), sense(s) {      // define Button with pin and sense, 
//                                        // Arduino pin the button/LED is attached
//                                        // sense: HIGH=active high, LOW=active low. 
  init(s);
}

ButtonLed::ButtonLed(uint8_t p) : pin(p) {            // default to sense=HIGH
  init(HIGH);
}

void ButtonLed::on(long mask) {
      pattern = mask;
}

void ButtonLed::blink(uint8_t mask) {
  once |= mask;
  // wait for next time step to display
}

void ButtonLed::process() {
  if(bnext && (millis()&0x1f)==0) {             // check button state every 32 ms
	bnext=false;                                // only want to check once per 
	pinMode(pin, INPUT);                        // need to change the pin to input..
	digitalWrite(pin,HIGH);                     // .. and activate pull up
	newState=(sense==digitalRead(pin));         // is the button up or down
	pinMode(pin,OUTPUT);                        // return pin to output mode
	digitalWrite(pin,ledState);                 // and make sure its showing its state
	if(newState != lastState) {                 // if button changed then..
	  lastState = newState;                     // ..remember button state
	  // now debounced
	} else {                                    // else button position is unchanged..
	  if(state!=newState) {                     // Debounced, but is it a new state?..
	    state = newState;                       // ..yes, so update
	    lastDuration = duration + 32;           //    and remember the duration of the last state
	    lastTime = millis();                    // ..so we can calc duration
	    duration = 0;                           // ..start timing ne state
	  } else {                                  // else same state continuing, so
	    duration = millis() - lastTime;         // .. calculate its duration
	  } 
	}
  } else {
	if((millis()&0x1f) != 0) bnext = true;      // partial through this 32ms period, so can trigger next period
  }
  // process LED
  if ( next && (millis()&0x3F) == 0) {          // trigger every 64ms, but only once
    if ((pattern & 0x1) !=0) {                  // if low bit 1 then ..
	  ledState = sense;                         // .. update LED and 
	  pattern = 0x80000000 | (pattern>>1);      // ..mimic roll with 1 in
	} else {                                    // else low bit is 0, so ..
	  ledState = !sense;                        // .. update LED and
	  pattern = 0x7FFFFFFF & (pattern>>1);      // .. mimic a roll with a 0 in
	}
	if ((once & 0x1) != 0) ledState = LOW;      // handle once-through pattern
	once = once>>1;
	digitalWrite(pin,ledState);                 // set the pin
	next = false;                               // we are complete for this 64ms period
  } else {
	if ( (millis()&0x3F) != 0) next = true;     // set-up to do the next period
  } 
  return;
}

