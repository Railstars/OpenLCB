#ifndef __OLCB_EVENT_H__
#define __OLCB_EVENT_H__

#include <stdint.h>
#include "WProgram.h"

class OLCB_Event {
  public: 
  
  uint8_t val[8];

  OLCB_Event() : flags(0) {
      val[0] = 0;
      val[1] = 0;
      val[2] = 0;
      val[3] = 0;
      val[4] = 0;
      val[5] = 0;
      val[6] = 0;
      val[7] = 0;
  }
  
  OLCB_Event(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7) : flags(0) {
      val[0] = b0;
      val[1] = b1;
      val[2] = b2;
      val[3] = b3;
      val[4] = b4;
      val[5] = b5;
      val[6] = b6;
      val[7] = b7;
  }
  
  bool operator==(const OLCB_Event &other) const
  {
    return  (val[0]==other.val[0])&&(val[1]==other.val[1])
          &&(val[2]==other.val[2])&&(val[3]==other.val[3])
          &&(val[4]==other.val[4])&&(val[5]==other.val[5])
          &&(val[6]==other.val[6])&&(val[7]==other.val[7]);
  }
  
  bool operator!=(const OLCB_Event &other) const
  {
    return !(*this == other);
  }
  
  /**
   * Check to see if this object is equal
   * to any in an array of OLCB_EventIDs
   */
  OLCB_Event* findEidInArray(OLCB_Event* array, int len) {
      for (int i = 0; i<len; i++) {
          if (*this == *(array+i)) return array+i;
      }
      return 0;
  }
  
  int16_t findIndexInArray(OLCB_Event* array, int len) {
      return findIndexInArray(array, len, 0);
  }

  int16_t findIndexInArray(OLCB_Event* array, int len, int index)
  {
      if(index >= len) return -1;
      for (int i = index; i<len; i++)
      {
          if (*this == *(array+i)) return i;
      }
      return -1;
  }
  
  bool isEmpty(void)
  {
  	return(!(val[0]|val[1]|val[2]|val[3]|val[4]|val[5]|val[6]|val[7]);
  }
  
  void print(void)
  {
#if defined(__AVR__) & defined(OLCB_DEBUG)
    char id[] = "eid:   ";
    Serial.print(id);
    Serial.println(val[0],DEC);
    Serial.print(id);
    Serial.println(val[1],DEC);
    Serial.print(id);
    Serial.println(val[2],DEC);
    Serial.print(id);
    Serial.println(val[3],DEC);
    Serial.print(id);
    Serial.println(val[4],DEC);
    Serial.print(id);
    Serial.println(val[5],DEC);
    Serial.print(id);
    Serial.println(val[6],DEC);
    Serial.print(id);
    Serial.println(val[7],DEC);
#endif
  }

  // bit mask local flags
  uint8_t flags;

  // Mark entry as consumer
  static const int CAN_CONSUME_FLAG = 0x20;
  // Mark entry as producer
  static const int CAN_PRODUCE_FLAG = 0x40;

};

#endif // __OLCB_EVENT__
