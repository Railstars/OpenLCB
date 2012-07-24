#ifndef __OLCB_CAN_BUFFER_H__
#define __OLCB_CAN_BUFFER_H__

#include <string.h>
#include "can.h"
#include "OLCB_NodeID.h"
#include "OLCB_Event.h"

/**
 * OpenLCB Frame Transport
 */
#define MASK_VARIABLE_FIELD 0x07FFF000L
#define SHIFT_VARIABLE_FIELD 12
#define RID_VAR_FIELD 0x0700
#define AMD_VAR_FIELD 0x0701
#define AME_VAR_FIELD 0x0702
#define AMR_VAR_FIELD 0x0703

/**
 * OpenCB Frame Type (1 bit)
 */
#define FRAME_TYPE_MASK 0x08000000L

/**
 * OpenLCB CAN Frame Type (3 bits)
 */
#define CAN_FRAME_TYPE_MASK					0x07
#define CAN_FRAME_CLEAR_MASK				0x18FFFFFF
#define GET_CAN_FRAME_TYPE(x)				((x >> 24) & CAN_FRAME_TYPE_MASK)
#define SET_CAN_FRAME_TYPE(x)				((uint32_t)(x & CAN_FRAME_TYPE_MASK) << 24)
#define CLEAR_CAN_FRAME_TYPE(x)				(x & CAN_FRAME_CLEAR_MASK)

#define CAN_FRAME_TYPE_REGULAR				0x01
#define CAN_FRAME_TYPE_DATAGRAM_ONLY		0x02
#define CAN_FRAME_TYPE_DATAGRAM_FIRST		0x03
#define CAN_FRAME_TYPE_DATAGRAM_MIDDLE		0x04
#define CAN_FRAME_TYPE_DATAGRAM_LAST		0x05
#define CAN_FRAME_TYPE_STREAM				0x07

/**
 * OpenLCB CAN MTI format (12 bits)
 */

#define MTI_MASK						0xFFF
#define MTI_CLEAR_MASK					0x1F000FFF
#define CLEAR_MTI(x)					(x & MTI_CLEAR_MASK)
#define GET_MTI(x)						((x >> 12) & MTI_MASK)
#define	SET_MTI(x)						((x & MTI_MASK) << 12)
#define MTI_ADDRESSED_MASK				0x08
#define MTI_IS_ADDRESSED(x)				(GET_MTI(x) & MTI_ADDRESSED_MASK)

#define MTI_INITIALIZATION_COMPLETE     0x100
#define MTI_VERIFY_NID_ADDRESSED		0x488
#define MTI_VERIFY_NID_GLOBAL			0x490
#define MTI_VERIFIED_NID                0x170
#define MTI_OPT_INTERACTION_REJECTED	0x068
#define MTI_TERMINATE_DUE_TO_ERROR		0x0A8

#define MTI_PROTOCOL_SUPPORT_INQUIRY	0x828
#define MTI_PROTOCOL_SUPPORT_REPLY		0x668

#define MTI_IDENTIFY_CONSUMERS          0x84F
#define MTI_IDENTIFY_CONSUMERS_RANGE    0x4A4
#define MTI_CONSUMER_IDENTIFIED_UNKNOWN_VALIDITY         0x4C7
#define MTI_CONSUMER_IDENTIFIED_VALID   0x4C4
#define MTI_CONSUMER_IDENTIFIED_INVALID 0x4C5

#define MTI_IDENTIFY_PRODUCERS          0x914
#define MTI_IDENTIFY_PRODUCERS_RANGE    0x524
#define MTI_PRODUCER_IDENTIFIED_UNKNOWN_VALIDITY         0x547
#define MTI_PRODUCER_IDENTIFIED_VALID   0x544
#define MTI_PRODUCER_IDENTIFIED_INVALID 0x545

#define MTI_IDENTIFY_EVENTS_ADDRESSED	0x968
#define MTI_IDENTIFY_EVENTS_GLOBAL		0x970

#define MTI_LEARN_EVENT                 0x594
#define MTI_PC_EVENT_REPORT             0x5B4

#define MTI_SNII_REQUEST				0xDE8
#define MTI_SNII_REPLY					0xA08

#define MTI_DATAGRAM_OK					0xA28
#define MTI_DATAGRAM_REJECTED			0xA48

#define MTI_STREAM_INITIATE_REQUEST		0xCC8
#define MTI_STREAM_INITIATE_REPLY		0x868
#define MTI_STREAM_DATA_PROCEED			0x888
#define MTI_STREAM_DATA_COMPLETE		0x8A8
//from CanFrameTransferS
#define RID_VAR_FIELD					0x0700
#define AMD_VAR_FIELD					0x0701
#define AME_VAR_FIELD					0x0702
#define AMR_VAR_FIELD					0x0703


// for definiton, see
// http://openlcb.sf.net/trunk/documents/can/index.html
// 
// In the following masks, bit 0 of the frame is 0x10000000L
//

#define SRC_ALIAS_MASK 0xFFF
#define SRC_CLEAR_MASK 0x1FFFF000
#define CLEAR_SRC_ALIAS(x)		(x & SRC_CLEAR_MASK)
#define GET_SRC_ALIAS(x) (x & SRC_ALIAS_MASK)
#define SET_SRC_ALIAS(x) (x & SRC_ALIAS_MASK)
#define DEST_ALIAS_MASK 0xFFF
#define DEST_CLEAR_MASK					0x1F000FFF
#define CLEAR_DEST_ALIAS(x)					(x & DEST_CLEAR_MASK)
#define GET_DEST_ALIAS(x) ((x >> 12) & DEST_ALIAS_MASK)
#define SET_DEST_ALIAS(x) ((x & DEST_ALIAS_MASK) << 12)

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
  
		uint8_t* getData(void);
		uint8_t getLength(void);
		void setLength(uint8_t len);
		void setData(uint8_t *bytes, uint8_t len);
		void setDataByte(uint8_t byte, uint8_t position);
		void init(OLCB_NodeID *sourceID);
		void init(uint16_t alias);
		void setSourceAlias(uint16_t a);
		uint16_t getSourceAlias(void);
		void setSourceNID(OLCB_NodeID *NID);
		void getSourceNID(OLCB_NodeID *NID);
		void setFragmentPosition(uint8_t pos);
		void setDestAlias(uint16_t a);
		uint16_t getDestAlias(void);
		void setDestNID(OLCB_NodeID *NID);
		uint16_t getDestNID(OLCB_NodeID *NID);
		void setFrameTypeCAN(void);
		bool isFrameTypeCAN(void);
		void setFrameTypeOpenLcb(void);
		bool isFrameTypeOpenLcb(void);
		void setMTI(uint16_t f);
		uint16_t getMTI(void);
		void setVariableField(uint16_t f);
		uint16_t getVariableField(void);
		void setCID(int i, uint16_t testval, uint16_t alias);
		bool isCID(void);
		void setRID(uint16_t alias);
		bool isRID(void);
		void setAMR(OLCB_NodeID *nid);
		bool isAMR();
		void setAMD(OLCB_NodeID *nid);
		bool isAMD();
		void setAME(OLCB_NodeID *nid);
		bool isAME();
		bool isAddressed();
		void setRejectOptionalInteraction(OLCB_NodeID* source, OLCB_NodeID* dest, uint16_t code = 0x0000);
		bool isRejectOptionalInteraction(void);
		void setPCEventReport(OLCB_NodeID* source, OLCB_Event* eid);
		bool isPCEventReport(void);
		void setLearnEvent(OLCB_NodeID* source, OLCB_Event* eid);
		bool isLearnEvent(void);
		void setInitializationComplete(OLCB_NodeID* source);
		bool isInitializationComplete(void);
		void getEventID(OLCB_Event* evt);
		void getNodeID(OLCB_NodeID* nid);
		void setVerifyNID(OLCB_NodeID* source);
		void setVerifyNID(OLCB_NodeID* source, OLCB_NodeID *dest);
		bool isVerifyNIDGlobal(void);
		bool isVerifyNIDAddressed();
		bool isVerifyNID(void);
		bool isVerifiedNID();
		void setVerifiedNID(OLCB_NodeID* source);
		bool isIdentifyConsumers(void);
		void setConsumerIdentified(OLCB_NodeID *source, OLCB_Event* eid);
		void setConsumerIdentifyRange(OLCB_NodeID* source, OLCB_Event* eid, OLCB_Event* mask);
		bool isIdentifyProducers(void);
		void setProducerIdentified(OLCB_NodeID* source, OLCB_Event* eid);
		void setProducerIdentifyRange(OLCB_NodeID* source, OLCB_Event* eid, OLCB_Event* mask);
		bool isIdentifyEventsGlobal(void);
		bool isIdentifyEventsAddressed(void);
		bool isIdentifyEvents(void);
		bool isDatagram(void);
		bool isLastDatagram(void);
		bool isFirstDatagram(void);
		bool isMiddleDatagram(void);
		bool isOnlyDatagram(void);
		void setOnlyDatagram(OLCB_NodeID* source, OLCB_NodeID* dest);
		void setFirstDatagram(OLCB_NodeID* source, OLCB_NodeID* dest);
		void setMiddleDatagram(OLCB_NodeID* source, OLCB_NodeID* dest);
		void setLastDatagram(OLCB_NodeID* source, OLCB_NodeID* dest);
		bool isDatagramAck();
		bool isDatagramNak();
		void setDatagramAck(OLCB_NodeID* source, OLCB_NodeID* dest);
		void setDatagramNak(OLCB_NodeID* source, OLCB_NodeID* dest, uint16_t reason);
		uint16_t getDatagramNakErrorCode();
		bool isProtocolSupportInquiry(void);
		bool isProtocolSupportReply(void);
		bool setProtocolSupportInquiry(OLCB_NodeID* source, OLCB_NodeID* dest);
		bool setProtocolSupportReply(OLCB_NodeID* source, OLCB_NodeID* dest);
		bool isSNIIRequest(void);
		bool isSNIIReply(void);
		void setSNIIReply(OLCB_NodeID* source, OLCB_NodeID* dest);
  
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
