#ifndef __OLCB_CAN_BUFFER_H__
#define __OLCB_CAN_BUFFER_H__

#include <string.h>
#include "can.h"
#include "OLCB_NodeID.h"
#include "OLCB_Event.h"

/**
 * OpenLCB CAN MTI format (4 bits)
 */
#define MTI_FORMAT_UNADDRESSED_MESSAGE		 0x8
#define MTI_FORMAT_ADDRESSED_DATAGRAM_ONLY	 0xA
#define MTI_FORMAT_ADDRESSED_DATAGRAM_FIRST  0xB
#define MTI_FORMAT_ADDRESSED_DATAGRAM_MIDDLE 0xC
#define MTI_FORMAT_ADDRESSED_DATAGRAM_LAST	 0xD
#define MTI_FORMAT_ADDRESSED_MESSAGE		 0xE
#define MTI_FORMAT_ADDRESSED_STREAM          0xF


/**
 * Basic 12-bit header MTI definitions for OpenLCB on CAN.
 * See the MtiAllocations.ods document for allocations.
 *
 * Note: This is just the low 12 bits, and does not include
 * 0-7 format MTI format field just above this.
 */
 
#define MTI_INITIALIZATION_COMPLETE     0x087

#define MTI_VERIFY_NID_GLOBAL           0x8A7
#define MTI_VERIFIED_NID                0x8B7

#define MTI_IDENTIFY_CONSUMERS          0xA4F
#define MTI_IDENTIFY_CONSUMERS_RANGE    0x25F
#define MTI_CONSUMER_IDENTIFIED         0x26B

#define MTI_IDENTIFY_PRODUCERS          0xA8F
#define MTI_IDENTIFY_PRODUCERS_RANGE    0x29F
#define MTI_PRODUCER_IDENTIFIED         0x2AB

#define MTI_IDENTIFY_EVENTS             0xAB7

#define MTI_LEARN_EVENT                 0xACF
#define MTI_PC_EVENT_REPORT             0xADF

/**
 * basic 8-bit Message Type byte values (from data[0])
 * for addressed messages.
 */
#define MTI_OPTION_INT_REJECTED			0x0C

#define MTI_VERIFY_NID_ADDRESSED        0x0A
#define MTI_IDENTIFY_EVENTS_ADDRESSED   0x2B

#define MTI_DATAGRAM_RCV_OK             0x4C
#define MTI_DATAGRAM_REJECTED           0x4D

/**
 * basic 16-bit Message Type byte values (from data[0,1])
 * for addressed messages.
 */

#define MTI_PROTOCOL_SUPPORT_INQUIRY	0x32E4
#define MTI_PROTOCOL_SUPPORT_REPLY		0x32F4


//from CanFrameTransferS
#define RID_VAR_FIELD 0x0700
#define AMD_VAR_FIELD 0x0701
#define AME_VAR_FIELD 0x0702
#define AMR_VAR_FIELD 0x0703


// for definiton, see
// http://openlcb.sf.net/trunk/documents/can/index.html
// 
// In the following masks, bit 0 of the frame is 0x10000000L
//

// bit 1
#define MASK_FRAME_TYPE 0x08000000L

// bit 17-28
#define MASK_SRC_ALIAS 0x00000FFFL

// bit 2-16
#define MASK_VARIABLE_FIELD 0x0FFFF000L
#define MASK_DEST_ALIAS      0x00FFF000L
#define SHIFT_VARIABLE_FIELD 12

// bit 2-4, at the top of the variable field
#define MASK_OPENLCB_FORMAT 0xF000L
#define SHIFT_OPENLCB_FORMAT 12


/**
 * Originally OpenLcbCanBuffer, name changed to reflect fork of original codebase.
 *
 * Class to handle transforming OpenLCB (S9.7) frames to/from std CAN frames.
 * <p>
 * We're trying to localize the formating of frames to/from the node here,
 * so that only this class needs to change when/if the wire protocol changes.
 */
 class OLCB_CAN_Buffer : public tCAN {
  public: 
  
  //access to payload
  uint8_t *getData(void);
  uint8_t getLength(void);
  void setLength(uint8_t length);
  void setData(uint8_t *bytes, uint8_t length);
  void setDataByte(uint8_t byte, uint8_t position);

  
  // Initialize a buffer for transmission
  void init(OLCB_NodeID *sourceID);
  void init(uint16_t alias);
  
  // start of basic message structure

  void setFrameTypeCAN();
  bool isFrameTypeCAN();
  
  void setFrameTypeOpenLcb();
  bool isFrameTypeOpenLcb();
  
  void setVariableField(uint16_t f);
  uint16_t getVariableField();
  
  void setSourceAlias(uint16_t a);
  uint16_t getSourceAlias();
  void setSourceNID(OLCB_NodeID *NID);
  void getSourceNID(OLCB_NodeID *NID);
  
  // end of basic message structure
  
  // start of CAN-level messages
  
  void setCID(int i, uint16_t testval, uint16_t alias);
  bool isCID();
  
  void setRID(uint16_t alias);
  bool isRID();
  
  void setAMR(OLCB_NodeID *nid);
  bool isAMR(void);
  
  void setAMD(OLCB_NodeID *nid);
  bool isAMD(void);
  
  void setAME(OLCB_NodeID *nid);
  bool isAME(void);

  // end of CAN-level messages
  
  // start of OpenLCB format support

  uint16_t getOpenLcbFormat();
  void setOpenLcbFormat(uint16_t i);
  
  bool isOpenLcbMtiFormat();
  bool isOpenLcDestIdFormat();
  bool isOpenLcbStreamIdFormat();

  void setOpenLcbMTI(uint16_t fmt, uint16_t mti);
  bool isOpenLcbMTI(uint16_t fmt, uint16_t mti);
  bool isOpenLcbMTI(uint16_t fmt);
  
  bool getDestinationNID(OLCB_NodeID *nid);
  void setDestinationNID(OLCB_NodeID *nid);
  
  // end of OpenLCB format support
  
  // start of OpenLCB messages

  // basic messages  
  bool isRejectOptionalInteraction(void);
  void setRejectOptionalInteraction(OLCB_NodeID* source, OLCB_NodeID* dest);

  
  bool isInitializationComplete(void);
  void setInitializationComplete(OLCB_NodeID* nid);

  bool isVerifyNIDGlobal(void);
  bool isVerifyNIDAddressed(void);
  void setVerifyNID(OLCB_NodeID* nid);  //use nid=0.0.0.0.0.0 to  send global
  
  bool isVerifiedNID(void);
  void setVerifiedNID(OLCB_NodeID* nid);
  

  // event related messages
  void getEventID(OLCB_Event* evt);
  void getNodeID(OLCB_NodeID* nid);

  bool isPCEventReport(void);
  void setPCEventReport(OLCB_Event* eid);
  
  bool isLearnEvent(void);
  void setLearnEvent(OLCB_Event* eid);

  bool isIdentifyConsumers(void);
  void setIdentifyConsumers(OLCB_Event* evt);
  
  bool isConsumerIdentified(void);
  bool isConsumerIdentifiedRange(void);
  void setConsumerIdentified(OLCB_Event* eid);
  void setConsumerIdentifyRange(OLCB_Event* eid, OLCB_Event* mask);   // Mask uses an OLCB_Event data structure; 1 bit means mask out when routing


  bool isIdentifyProducers(void);
  bool setIdentifyProducers(OLCB_Event* evt);

  bool isProducerIdentified(void);
  bool isProducerIdentifiedRange(void);
  void setProducerIdentified(OLCB_Event* eid);
  // Mask uses an OLCB_Event data structure; 1 bit means mask out when routing
  void setProducerIdentifyRange(OLCB_Event* eid, OLCB_Event* mask);

  bool isIdentifyEvents(void);
  bool isIdentifyEventsAddressed(void);
  bool setIdentifyEvents(OLCB_NodeID* nid); //use nid=0.0.0.0.0.0 to  send global


// datagram related messages
  bool isDatagram(void);
  bool isLastDatagram(void);
  bool isFirstDatagram(void);
  bool isDatagramAck(void);
  bool isDatagramNak(void);
  void setDatagram(void);
  void setLastDatagram(void);
  void setDatagramAck(void);
  void setDatagramNak(void);
  uint16_t getDatagramNakErrorCode(void);
  
 //PIP messages
  bool isProtocolSupportInquiry(void);
  
  
  void setInternal(void) {internal = true;}
  void setExternal(void) {internal = false;}
  bool isInternal(void) {return internal;}
  bool isExternal(void) {return !internal;}
  
  private: 
//  unsigned int nodeAlias;   // Initialization complete sets, all later use

  // service routine to copy content (0-7) to a previously-allocated Eid
  void loadFromEid(OLCB_Event* eid);
  
  //TO BE MADE USE OF LATER. belong in OLCB_CAN_Buffer.
  OLCB_NodeID _source;
  OLCB_NodeID _destination;
  bool internal;
};

#endif
