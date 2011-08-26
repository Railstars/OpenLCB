#include <OLCB_AliasCache.h>
#include <OLCB_Buffer.h>
#include <OLCB_CAN_Buffer.h>
#include <OLCB_Link.h>
#include <OLCB_CAN_Link.h>
#include <OLCB_Datagram.h>
#include <OLCB_Datagram_Handler.h>
#include <OLCB_Event.h>
#include <OLCB_EventID.h>
#include <OLCB_Handler.h>
#include <OLCB_NodeID.h>
#include <OLCB_Stream.h>

#include <can.h>

class virtualNodeFactory;

class virtualNode: public OLCB_Datagram_Handler
{
  friend class virtualNodeFactory;

 public:
  bool processDatagram(void) //NOT "boolean"!
  {
     //To have made it this far, we can be sure that _rxDatagramBuffer has a valid datagram loaded up, and that it is in fact addressed to us.
     Serial.println("Node with id Received a datagram!");
     _rxDatagramBuffer->destination.print(); //is this really us?
     NID->print();
     for(int i = 0; i < _rxDatagramBuffer->length; ++i)
     {
       Serial.print("    ");
       Serial.println(_rxDatagramBuffer->data[i], HEX);
     }
     return true; //returning true causes an ACK; returning false a NAK. Not very sophisticated yet...
  }
  
//  void update(void)
//  {
//    OLCB_Datagram_Handler::update();
//  }
  
};

class virtualNodeFactory: public OLCB_Datagram_Handler
{
 public:
  void init(void)
  {
    Serial.println("Initializing");
    for(int i = 0; i < 10; ++i)
      avail[i] = 0;
    Serial.println("Done initializing");
  }
  
   //intercept verifyNID messages intended for locomotives, and try to create a new virtual node, if possible
   //notice that because of the order the various handlers get added to link, this method will only ever be
   //called if none of the extant virtualNodes have already handled it. So we know going into it that there
   //isn't already a virtual node at this address.
  
  bool verifyNID(OLCB_NodeID *nid)
  {
    if( (nid->val[0] == 6) && (nid->val[1] == 1) ) //if it's intended for a DCC locomotive
    {
      Serial.println("Producing a new virtual node for address: ");
      nid->print();
      //find a slot for it
      for(int i = 0; i < 10; ++i)
      {
        if(avail[i] == 0) //an empty slot is found!
        {
          Serial.print("    Installing in slot ");
          Serial.println(i,DEC);
          nodes[i].setLink(_link);
          nodes[i].setNID(nid);
          avail[i] = 1;
          return false; //what the what? we're actually not yet ready to send out the verifiedNID packet!
        }
        else
        {  
          Serial.print("   Slot taken: ");
          Serial.println(i,DEC);
        }
      }
      Serial.println("    Out of slots. Too bad.");
    }
    return false; //no room availble, or not a request for a loco.
  }
  
  void update(void)
  {
    for(int i = 0; i < 10; ++i)
    {
      if(avail[i] == 1)
      {
        if(nodes[i].NID->alias > 0)
        {
          Serial.println("Sending verified NID from LocoFactory");
          Serial.println(i,DEC);
          Serial.println(avail[i],DEC);
          nodes[i].NID->print();
          OLCB_CAN_Link * temp = (OLCB_CAN_Link *)_link;
          temp->sendVerifiedNID(nodes[i].NID);
          avail[i] = 2;
        }
      }
    }
    OLCB_Datagram_Handler::update();
  }
  
  virtualNode nodes[10];
  uint8_t avail[10];
};

virtualNodeFactory locoFactory;

OLCB_NodeID nid(2,1,13,0,0,1);
OLCB_CAN_Link link(&nid);

void setup() {
  Serial.begin(115200);
  Serial.println("Begin!");
  delay(1000);
  // put your setup code here, to run once:
  link.initialize();

  Serial.print("This is my alias (should not be 0): ");
  Serial.println(nid.alias);
  locoFactory.setLink((OLCB_Link*)&link);
}

uint8_t * heapptr, * stackptr;
void check_mem() {
  stackptr = (uint8_t *)malloc(4);          // use stackptr temporarily
  heapptr = stackptr;                     // save value of heap pointer
  free(stackptr);      // free up the memory again (sets stackptr to 0)
  stackptr =  (uint8_t *)(SP);           // save value of stack pointer
}


void loop() {
  link.update();
//  check_mem();
//  Serial.println(stackptr - heapptr, DEC);
}
