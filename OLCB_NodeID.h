#ifndef __OLCB_NODEID_H__
#define __OLCB_NODEID_H__

#include <stdint.h>

#define OLCB_DEBUG
#include "Arduino.h"

class OLCB_NodeID {
  public:
  
  uint8_t val[6];
  uint16_t alias; //used by CAN and other buses.
  bool initialized;

  
  OLCB_NodeID() : alias(0), initialized(false)
  {
      val[0] = 0;
      val[1] = 0;
      val[2] = 0;
      val[3] = 0;
      val[4] = 0;
      val[5] = 0;
  }
  
  OLCB_NodeID(uint8_t b0, uint8_t b1, uint8_t b2, 
         uint8_t b3, uint8_t b4, uint8_t b5) : alias(0), initialized(false)
  {
      val[0] = b0;
      val[1] = b1;
      val[2] = b2;
      val[3] = b3;
      val[4] = b4;
      val[5] = b5;
  }
  
  void set(uint8_t b0, uint8_t b1, uint8_t b2, 
         uint8_t b3, uint8_t b4, uint8_t b5)
  {
      alias = 0; //have to reset alias!
      initialized = false;
      val[0] = b0;
      val[1] = b1;
      val[2] = b2;
      val[3] = b3;
      val[4] = b4;
      val[5] = b5;
  }
  
  //***********TODO THERE IS A SERIOUS ISSUE HERE THAT I AM HAVING PROBLEMS DEBUGGING!!!
  //For some reason, once the alias check falls through (and it does when one or both aliases are 0, which means no alias has been assigned), then we return '0' when the NodeID's differ, but for whatever crazy stupid reason, putting that in an if statement causes it to be executed.
  //That is, assume that NodeID1 and NodeID2 are distinct NodeIDs, and that one or the other hasn't had an alias assigned.
  //Suppose we have this code:
  //if(NodeID1 == NodeID2) {do crap};
  //Then, although this operator will properly return 0, crap gets done anyway. Which is deeply disturbing to me.
  //In other words, DO NOT RELY ON THIS METHOD IT DOES NOT WORK!
  bool operator==(const OLCB_NodeID &other) const
  { 
    //first, check the aliases. Only pay them heed if they both are non-zero.
    if((alias!=0) && (other.alias!=0))
    {
      //Serial.println("==: comparing aliases");
      //Serial.println(alias,DEC);
      //Serial.println(other.alias, DEC);
      //Serial.println(alias==other.alias, DEC);
      return (alias==other.alias); //same alias, same NID. Effectively.
    }
    //if one or both aliases are zero, compare the NID itself. Aliases can't always be trusted :D
    else
    {
        //Serial.println("==: comparing NID directly");
        //Serial.println(val[0]==other.val[0], DEC);
        //Serial.println(val[1]==other.val[1], DEC);
        //Serial.println(val[2]==other.val[2], DEC);
        //Serial.println(val[3]==other.val[3], DEC);
        //Serial.println(val[4]==other.val[4], DEC);
        //Serial.println(val[5]==other.val[5], DEC);
        //Serial.println((val[0]==other.val[0])&&(val[1]==other.val[1])
        //  &&(val[2]==other.val[2])&&(val[3]==other.val[3])
        //  &&(val[4]==other.val[4])&&(val[5]==other.val[5]), DEC);
    return((val[0]==other.val[0])&&(val[1]==other.val[1])
          &&(val[2]==other.val[2])&&(val[3]==other.val[3])
          &&(val[4]==other.val[4])&&(val[5]==other.val[5]));
    }
  }
  
bool sameNID(const OLCB_NodeID *other) const
  { 
    return  (val[0]==other->val[0])&&(val[1]==other->val[1])
          &&(val[2]==other->val[2])&&(val[3]==other->val[3])
          &&(val[4]==other->val[4])&&(val[5]==other->val[5]);
  }
  
  bool operator!=(const OLCB_NodeID &other) const
  {
  	return !(*this == other);
  }
  
  bool empty(void)
  {
    if(alias)
    {
      return false;
    }
    else if (val[0] || val[1] || val[2] || val[3] || val[4] || val[5])
    {
      return false;
    }

    return true;
  }

  /**
   * Check to see if this object is equal
   * to any in an array of OLCB_NodeIDs
   */
  OLCB_NodeID* findInArray(OLCB_NodeID* array, int len) {
      for (int i = 0; i<len; i++) {
          if (*this == *(array+i)) return array+i;
      }
      return 0;
  }
  
  void copy(OLCB_NodeID *src)
  {
    alias = src->alias;
    for (int i = 0; i<6; i++)
    {
      val[i] = src->val[i];
    }
  }

  void print(void)
  {
// #if defined(__AVR__) & defined(OLCB_DEBUG)
//#if defined(OLCB_DEBUG)
//     char id[] = "nid:   ";
//     Serial.print("alias: ");
//     Serial.println(alias,HEX);
//      Serial.print(id);
//      Serial.println(val[0],DEC);
//      Serial.print(id);
//      Serial.println(val[1],DEC);
//      Serial.print(id);
//      Serial.println(val[2],DEC);
//      Serial.print(id);
//      Serial.println(val[3],DEC);
//      Serial.print(id);
//      Serial.println(val[4],DEC);
//      Serial.print(id);
//      Serial.println(val[5],DEC);
//#endif
  }

};

#endif
