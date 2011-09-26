// Permits control of a single locomotive with address 03.


#include <DCCPacket.h>
#include <DCCPacketQueue.h>
#include <DCCPacketScheduler.h>

#include <can.h>

#include <OpenLCB.h>

class DCC_Train : public OLCB_Virtual_Node, public OLCB_Datagram_Handler, public OLCB_DCC_Train
{
  public:
  
    void create(OLCB_Link *link, OLCB_NodeID *nid, DCCPacketScheduler *dcc)
    {
      OLCB_Datagram_Handler::create(link,nid);
      OLCB_Virtual_Node::create(link,nid);
      DCC_Train_create(dcc);
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
      DCC_Train_update();
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
  
  void initialize(void)
  {
    DCC_Train_initialize();
  }
  
  bool processDatagram(void)
  {
    if(isPermitted() && (_rxDatagramBuffer->destination == *OLCB_Virtual_Node::NID))
    {
      return DCC_Train_processDatagram(_rxDatagramBuffer);
    }
  }
  
  
};

OLCB_NodeID nid(6,1,0,0,0,3);
DCC_Train myLoco;
OLCB_CAN_Link link;
DCCPacketScheduler controller;

void setup()
{
  Serial.begin(115200);
  
  Serial.println("SimpleDCS begin!");
  
  link.initialize();
  myLoco.initialize();
  myLoco.create(&link, &nid, &controller);
  link.addVNode(&myLoco);
  
  Serial.println("Initialization complete!");
}

void loop()
{
  link.update();
}
