#include "OLCB_Link.h"

void OLCB_Link::update(void)//called repeatedly
{
  //call all our handler's update functions
  OLCB_Virtual_Node *iter = _handlers;
  while(iter)
  {
    iter->update();
    iter = iter->next;
  }
}
  
void OLCB_Link::addHandler(OLCB_Virtual_Node *handler)
{
//  Serial.print("OLCB_Link::addHandler: registering handler ");
//  Serial.println((uint16_t)handler, HEX);
  handler->next = _handlers;
  _handlers = handler;
//  Serial.println("====");
//  OLCB_Virtual_Node *iter = _handlers;
//  while(iter != NULL)
//  {
//    Serial.println((uint16_t)iter, HEX);
//    iter = iter->next;
//  }
//  Serial.println("====");
}

void OLCB_Link::removeHandler(OLCB_Virtual_Node *handler)
{
//  Serial.println("Removing Handler ");
  if(!_handlers) //nothing to remove!
    return;
    

//  Serial.println((uint16_t)handler, HEX);
  
//  Serial.println("====");
  OLCB_Virtual_Node *iter = _handlers;
//  while(iter != NULL)
//  {
//    Serial.println((uint16_t)iter, HEX);
//    iter = iter->next;
//  }
//  Serial.println("====");
  
  //Looking for the handler that comes before handler.
//  iter = _handlers;
  if(iter == handler)
  {
//    Serial.println("The first item is the one to remove!");
    _handlers = handler->next;
  }
  else
  {
    while(iter->next != NULL)
   {
     if(iter->next == handler)
     {
//       Serial.print("Found the one before: ");
//        Serial.println((uint16_t)iter, HEX);
        //remove it from the list
       iter->next = handler->next;
        break;
      }
     iter = iter->next;
    }
  }
  
//  Serial.println("==*==");
//  iter = _handlers;
//  while(iter != NULL)
//  {
//    Serial.println((uint16_t)iter, HEX);
//    iter = iter->next;
//  }
//  Serial.println("==*==");
}
