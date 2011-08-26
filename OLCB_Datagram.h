#ifndef __OLCB_DATAGRAM_H__
#define __OLCB_DATAGRAM_H__

#include "OLCB_NodeID.h"

#ifndef DATAGRAM_LENGTH
#define DATAGRAM_LENGTH 70
#endif

// some error messages for datagram rejected response messages
#define DATAGRAM_REJECTED                        0x000
#define DATAGRAM_REJECTED_PERMANENT_ERROR        0x100
#define DATAGRAM_REJECTED_INFORMATION_LOGGED     0x101
#define DATAGRAM_REJECTED_SOURCE_NOT_PERMITTED   0x102
#define DATAGRAM_REJECTED_DATAGRAMS_NOT_ACCEPTED 0x104
#define DATAGRAM_REJECTED_BUFFER_FULL            0x200
#define DATAGRAM_REJECTED_OUT_OF_ORDER           0x600

#define DATAGRAM_REJECTED_NO_RESEND_MASK         0x100
#define DATAGRAM_REJECTED_RESEND_MASK            0x200
#define DATAGRAM_REJECTED_TRANSPORT_ERROR_MASK   0x400

#define MTI_DATAGRAM_RCV_OK             0x4CF
#define MTI_DATAGRAM_REJECTED           0x4DF



/*************************
 A few things to worry about.
 Sending a datagram:
   -> datagram chunk 1
   -> datagram chunk 2, etc.
   <- ACK
     or
   <- NAK
   Need a way to retrieve the ACK or NAK from the Link Controller. Means that certain kinds of events need to be passed to datagrams. Hrm.
*************************/

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
