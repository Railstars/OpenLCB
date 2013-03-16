#ifndef __OLCB_DATAGRAM_H__
#define __OLCB_DATAGRAM_H__

#include "OLCB_NodeID.h"

#ifndef DATAGRAM_LENGTH
#define DATAGRAM_LENGTH 72 //could be less if it is known that no datagram that will ever be sent on this network will be this long.
#endif

// some error messages for datagram rejected response messages
#define DATAGRAM_REJECTED                        0x0000
#define DATAGRAM_REJECTED_PERMANENT_ERROR        0x1000
#define DATAGRAM_REJECTED_INFORMATION_LOGGED     0x1010
#define DATAGRAM_REJECTED_SOURCE_NOT_PERMITTED   0x1020
#define DATAGRAM_REJECTED_DATAGRAM_TYPE_NOT_ACCEPTED 0x1040
#define DATAGRAM_REJECTED_BUFFER_FULL            0x2000
#define DATAGRAM_REJECTED_OUT_OF_ORDER           0x6000

#define DATAGRAM_REJECTED_NO_RESEND_MASK         0x1000
#define DATAGRAM_REJECTED_RESEND_MASK            0x2000
#define DATAGRAM_REJECTED_TRANSPORT_ERROR_MASK   0x4000


//A class for representing a datagram object
class OLCB_Datagram
{
 public:
  OLCB_Datagram() {return;}
  OLCB_NodeID source;
  OLCB_NodeID destination;
  uint8_t length;
  uint8_t data[DATAGRAM_LENGTH];
};

#endif
