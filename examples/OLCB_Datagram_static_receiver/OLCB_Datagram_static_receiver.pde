#include <OpenLCB.h>
#include <can.h>

class simpleDatagramReceiver: public OLCB_Datagram_Handler
{
 public:
  bool processDatagram(void) //NOT "boolean"!
  {
     //To have made it this far, we can be sure that _rxDatagramBuffer has a valid datagram loaded up, and that it is in fact addressed to us.
     Serial.println("Received a datagram!");
     for(int i = 0; i < _rxDatagramBuffer->length; ++i)
     {
       Serial.print("    ");
       Serial.println(_rxDatagramBuffer->data[i], HEX);
     }
     return true; //returning true causes an ACK; returning false a NAK. Not very sophisticated yet...
  }
};

OLCB_NodeID nid(2,1,13,0,0,2);
OLCB_NodeID nid2(6,1,0,0,0,4);
OLCB_CAN_Link link(&nid);
simpleDatagramReceiver datagram_handler;

void setup() {
  Serial.begin(115200);
  Serial.println("Begin!");
  
  delay(1000);
  // put your setup code here, to run once:
  link.initialize();

  Serial.print("This is my alias (should not be 0): ");
  Serial.println(nid.alias);
  datagram_handler.setLink((OLCB_Link*)&link);
  datagram_handler.setNID(&nid2);    //causes it to be set up as a virtual node with it's own NID
}

void loop() {
  link.update();
}
