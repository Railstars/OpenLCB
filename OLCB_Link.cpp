#include "OLCB_Link.h"
    
void OLCB_Link::update(void)//called repeatedly
{
    //call all our handler's update functions
    //Updated to only call one at a time, to permit faster message handling.
    static OLCB_Virtual_Node *iter = _handlers;
    //while(iter)
    //{
        iter->update();
        iter = iter->next;
        if(!iter) //reached end of list
        iter = _handlers; //return to beginning for next time.
    //}
}

void OLCB_Link::addVNode(OLCB_Virtual_Node *vnode)
{
    //Serial.println("link adding vnode");
    //Serial.println((int)_handlers, HEX);
    vnode->next = _handlers;
    _handlers = vnode;
    //Serial.println((int)_handlers, HEX);
    //Serial.println((int)(_handlers->next), HEX);
}

void OLCB_Link::removeVNode(OLCB_Virtual_Node *vnode)
{
    //  //Serial.println("Removing Handler ");
    if(!_handlers) //nothing to remove!
    return;
    

    //  //Serial.println((uint16_t)handler, HEX);
    
    //  //Serial.println("====");
    OLCB_Virtual_Node *iter = _handlers;
    //  while(iter != NULL)
    //  {
        //    //Serial.println((uint16_t)iter, HEX);
        //    iter = iter->next;
    //  }
    //  //Serial.println("====");
    
    //Looking for the handler that comes before handler.
    //  iter = _handlers;
    if(iter == vnode)
    {
        //    //Serial.println("The first item is the one to remove!");
        _handlers = vnode->next;
        vnode->next = NULL;
    }
    else
    {
        while(iter->next != NULL)
        {
            if(iter->next == vnode)
            {
                //       //Serial.print("Found the one before: ");
                //        //Serial.println((uint16_t)iter, HEX);
                //remove it from the list
                iter->next = vnode->next;
                vnode->next = NULL;
                break;
            }
            iter = iter->next;
        }
    }
    
    //  //Serial.println("==*==");
    //  iter = _handlers;
    //  while(iter != NULL)
    //  {
        //    //Serial.println((uint16_t)iter, HEX);
        //    iter = iter->next;
    //  }
    //  //Serial.println("==*==");
}
