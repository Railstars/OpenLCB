#include <OpenLCB.h>

#include <can.h>


OLCB_Datagram dg;


class Throttle: public OLCB_Virtual_Node, public OLCB_Datagram_Handler
{
 public:
  void initialize(void)
  {
      speed = 0;
      old_speed = 127;
      direction = 1; //forwards
      initialized = false;
  }
  
  void create(OLCB_Link *link, OLCB_NodeID *nid)
  {
    OLCB_Virtual_Node::create(link, nid);
    OLCB_Datagram_Handler::create(link, nid);
  }
  
  void update(void)
  {
    if(isPermitted())
    {
      if(!initialized)
    {
      dg.data[0] = 0x30;
      dg.data[1] = 0x01; //attach
      dg.length = 2;
//      initialized = true;
      Serial.println("sending attach request");
      sendDatagram(&dg);
    }
    else
    {
      speed = map(analogRead(A0), 0, 1023, 0, 100); //divide by two to take a 0-1023 range number and make it 0-127 range.
      if(speed != old_speed)
      {
        dg.data[0] = 0x30;
        dg.data[1] = 0x06; //set speed
        dg.data[2] = direction;
        dg.data[3] = speed;
        dg.length = 4;
        if(sendDatagram(&dg))
        {
          Serial.println("Datagram away! Sent to:");
        }
        old_speed = speed;
      }
    }
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
     if(!initialized && accepted)
     {
       initialized = true;
     }
  }
  
  
  bool processDatagram(void)
  {
    if((_rxDatagramBuffer->data[0] == 0x30) && (_rxDatagramBuffer->data[1] == 0x02))
    {
    //TODO check the source. 
      Serial.println("Making state IDLE in processDatagram()");
      initialized = true;
      Serial.println("****Throttle attached!****");
      return true;
    }
    else return false;
}
  
  
  boolean initialized;
 private:
  unsigned char speed, old_speed;
  unsigned char direction;
};

OLCB_NodeID nid(2,1,13,0,0,2);
OLCB_CAN_Link link;
Throttle datagram_handler;

OLCB_NodeID dest(6,1,0,0,0,3); //send to DCC address "03"

void setup() {
  Serial.begin(115200);
  
  delay(1000);
  Serial.println("Beginning initialization");  

  link.initialize();

  Serial.print("This is my alias (should not be 0): ");
  Serial.println(nid.alias);
  datagram_handler.initialize();
  datagram_handler.create((OLCB_Link*)&link, &nid);
  link.addVNode(&datagram_handler);
  
  dg.destination.copy(&dest);
    
  Serial.println("At end of init");
}

void loop() {  
  link.update();
}
