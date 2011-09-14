#include <OpenLCB.h>
#include <can.h>

class simpleDatagramReceiver: public OLCB_Virtual_Node, public OLCB_Datagram_Handler
{
 public:
  void create(OLCB_Link *link, OLCB_NodeID *nid)
    {
      OLCB_Datagram_Handler::create(link,nid);
      OLCB_Virtual_Node::create(link,nid);
    }

  bool handleMessage(OLCB_Buffer *buffer)
    {
      return OLCB_Datagram_Handler::handleMessage(buffer);
    }
    
  void update(void)
  {
    if(isPermitted())
    {
      OLCB_Datagram_Handler::update();
    }
  }

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

OLCB_NodeID nid(2,1,13,0,1,1);
OLCB_CAN_Link link;
simpleDatagramReceiver datagram_handler;

void setup() {
  Serial.begin(115200);
  Serial.println("Begin!");
  
  // put your setup code here, to run once:
  link.initialize();

  datagram_handler.create(&link, &nid);
  link.addVNode(&datagram_handler);
}

void loop() {
  link.update();
}
