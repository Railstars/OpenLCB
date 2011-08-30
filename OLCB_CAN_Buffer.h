#ifndef __OLCB_CAN_BUFFER_H__
#define __OLCB_CAN_BUFFER_H__

#include <string.h>
#include "OLCB_NodeID.h"
#include "OLCB_EventID.h"
#include "can.h"

/**
 * OpenLCB CAN MTI format (3 bits)
 */
#define MTI_FORMAT_SIMPLE_MTI               0    
#define MTI_FORMAT_COMPLEX_MTI              1
//
//
#define MTI_FORMAT_ADDRESSED_DATAGRAM       4    
#define MTI_FORMAT_ADDRESSED_DATAGRAM_LAST  5    
#define MTI_FORMAT_ADDRESSED_NON_DATAGRAM   6    
#define MTI_FORMAT_STREAM_CODE              7    


/**
 * Basic header MTI definitions for OpenLCB on CAN.
 * See the MtiAllocations.ods document for allocations.
 */
 
#define MTI_INITIALIZATION_COMPLETE     0x08F

#define MTI_VERIFY_NID                  0x0AF
#define MTI_VERIFY_NID_GLOBAL           0x0A0
#define MTI_VERIFIED_NID                0x0BF

#define MTI_IDENTIFY_CONSUMERS          0x24F
#define MTI_IDENTIFY_CONSUMERS_RANGE    0x25F
#define MTI_CONSUMER_IDENTIFIED         0x26F

#define MTI_IDENTIFY_PRODUCERS          0x28F
#define MTI_IDENTIFY_PRODUCERS_RANGE    0x29F
#define MTI_PRODUCER_IDENTIFIED         0x2AF

#define MTI_IDENTIFY_EVENTS             0x2BF

#define MTI_LEARN_EVENT                 0x2CF
#define MTI_PC_EVENT_REPORT             0x2DF

#define MTI_DATAGRAM_RCV_OK             0x4CF
#define MTI_DATAGRAM_REJECTED           0x4DF

#define RID_VAR_FIELD 0x0700
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
#define MASK_VARIABLE_FIELD  0x07FFF000L
#define SHIFT_VARIABLE_FIELD 12
#define MASK_DEST_ALIAS      0x00FFF000L

// bit 2-4, at the top of the variable field
#define MASK_OPENLCB_FORMAT 0x07000L
#define SHIFT_OPENLCB_FORMAT 12


//TODO: REFACTOR INTO BASE CLASS OLCB_CAN_Buffer AND DERIVED CLASS OLCB_CAN_Buffer!!!

/**
 * Originally OpenLcbCanBuffer, name changed to reflect fork of original codebase.
 *
 * Class to handle transforming OpenLCB (S9.6) frames to/from std CAN frames.
 * <p>
 * We're trying to localize the formating of frames to/from the node here,
 * so that only this class needs to change when/if the wire protocol changes.
 */
 class OLCB_CAN_Buffer : public tCAN {
  public: 
  
  // Initialize a buffer for transmission
  void init(OLCB_NodeID *sourceID);
  
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
  
  void setAMR(uint16_t alias);
  bool isAMR(void);

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
  
  // end of OpenLCB format support
  
  // start of OpenLCB messages
  
  void setInitializationComplete(OLCB_NodeID* nid);
  bool isInitializationComplete();

  void setPCEventReport(OLCB_EventID* eid);
  bool isPCEventReport();
  
  void setLearnEvent(OLCB_EventID* eid);
  bool isLearnEvent();
  
  void getEventID(OLCB_EventID* evt);
  void getNodeID(OLCB_NodeID* nid);
  
  bool isVerifyNID();
  bool isVerifyNIDglobal();
  void setVerifyNID(OLCB_NodeID* nid);
  void setVerifiedNID(OLCB_NodeID* nid);
  bool isVerifiedNID();

  bool isIdentifyConsumers();
  
  void setConsumerIdentified(OLCB_EventID* eid);
  
  // Mask uses an OLCB_EventID data structure; 1 bit means mask out when routing
  void setConsumerIdentifyRange(OLCB_EventID* eid, OLCB_EventID* mask);

  bool isIdentifyProducers();

  void setProducerIdentified(OLCB_EventID* eid);

  // Mask uses an OLCB_EventID data structure; 1 bit means mask out when routing
  void setProducerIdentifyRange(OLCB_EventID* eid, OLCB_EventID* mask);

  bool isIdentifyEvents();

  bool isDatagram();
  bool isLastDatagram();
  bool isDatagramAck();
  bool isDatagramNak();
  uint16_t getDatagramNakErrorCode();
  
  bool getDestinationNID(OLCB_NodeID *nid);
  void setDestinationNID(OLCB_NodeID *nid);
  
//  unsigned int getAlias() {return nodeAlias;}
  
  private: 
//  unsigned int nodeAlias;   // Initialization complete sets, all later use

  // service routine to copy content (0-7) to a previously-allocated Eid
  void loadFromEid(OLCB_EventID* eid);
  
  //TO BE MADE USE OF LATER. belong in OLCB_CAN_Buffer.
  OLCB_NodeID _source;
  OLCB_NodeID _destination;
};

#endif
