#include <OpenLCB.h>
#include <can.h>

class simpleDatagramHandler: public OLCB_Virtual_Node, public OLCB_Datagram_Handler
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

  void datagramResult(bool accepted, uint16_t errorcode)
  {
     Serial.print("The datagram was ");
     if(!accepted)
       Serial.print("not ");
     Serial.println("accepted.");
     if(!accepted)
     {
       Serial.print("   The reason: ");
       Serial.println(errorcode,HEX);
     }
  }
};

OLCB_NodeID nid(2,1,13,0,1,2);
OLCB_CAN_Link link;
simpleDatagramHandler datagram_handler;

OLCB_Datagram dg;
#define DEST 2,1,13,0,1
OLCB_NodeID dest(DEST,1);

unsigned long timer, timer2;
unsigned char i = 1;

void setup() {
  Serial.begin(115200);
  
  delay(1000);
  Serial.println("Beginning initialization");  

  link.initialize();
  
  datagram_handler.create(&link, &nid);  
  dg.destination.copy(&dest);
  dg.data[0] = 1;
  dg.data[1] = 2;
  dg.data[2] = 4;
  dg.length = 3;

  timer = 0;
  timer2 = millis();
  
  link.addVNode(&datagram_handler);

  Serial.println("At end of init");
}

void loop() {
  link.update();
  if(datagram_handler.isPermitted())
  {
    timer = millis();
    if((timer - timer2) > 5000)
    {
      //send datagram
      if(datagram_handler.sendDatagram(&dg))
      {
        Serial.println("Datagram away! Sent to:");
        dest.print();
        dest.set(DEST,++i);
        dg.destination.copy(&dest);
      }
      timer2 = timer;
    }
  }
}
