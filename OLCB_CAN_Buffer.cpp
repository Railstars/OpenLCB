#include "OLCB_CAN_Buffer.h"
#include "Arduino.h"

uint8_t* OLCB_CAN_Buffer::getData(void)
{
	return data;
}

uint8_t OLCB_CAN_Buffer::getLength(void)
{
	return length;
}

void OLCB_CAN_Buffer::setLength(uint8_t len)
{
	length = len;
}

void OLCB_CAN_Buffer::setData(uint8_t *bytes, uint8_t len)
{
  length = (len>8)?8:len;
	memcpy(data, bytes, length);
}

void OLCB_CAN_Buffer::setDataByte(uint8_t byte, uint8_t position)
{
	data[(position>=length)?(length-1):position] = byte;
}


void OLCB_CAN_Buffer::init(OLCB_NodeID *sourceID)
{
	init(sourceID->alias);
}

void OLCB_CAN_Buffer::init(uint16_t alias)
{
internal = 0;
	flags.extended = 1;
	flags.rtr = 0;
	length = 0;
	id = 0x19000000; //defaults to CAN_FRAME_TYPE_REGULAR, to be overridden if needed
	SET_SRC_ALIAS(id, alias);
}

// start of basic message structure

void OLCB_CAN_Buffer::setSourceAlias(uint16_t a)
{
	CLEAR_SRC_ALIAS(id);
	SET_SRC_ALIAS(id, a);
}

uint16_t OLCB_CAN_Buffer::getSourceAlias(void)
{
		return GET_SRC_ALIAS(id);
}

void OLCB_CAN_Buffer::setSourceNID(OLCB_NodeID *NID)
{
	setSourceAlias(NID->alias);
	_source.copy(NID);
}

void OLCB_CAN_Buffer::getSourceNID(OLCB_NodeID *NID)
{
	if(_source.empty()) //if _source.alias == 0, basically.
	{
		_source.set(0,0,0,0,0,0);
	}
	_source.alias = getSourceAlias();
	NID->copy(&_source);
}

void OLCB_CAN_Buffer::setFragmentPosition(uint8_t pos)
{
	if(GET_CAN_FRAME_TYPE(id) == CAN_FRAME_TYPE_REGULAR)
		if(length == 0) length = 1;
		data[0] |= (pos & 0x0F) << 12;
}

void OLCB_CAN_Buffer::setDestAlias(uint16_t a)
{
	//can only be called after the frame type and MTI have been set, as there are two ways of setting the destination
	if(GET_CAN_FRAME_TYPE(id) == CAN_FRAME_TYPE_REGULAR)
	{
		//Serial.println("putting dest alias in payload");
		if(length == 0) length = 2;
		data[0] = (a & 0xFFF)>>8; //TODO overwrites "only" flag; multi-part messages will need to write flags here AFTER calling this method.
		data[1] = (a & 0xFF);
		//Serial.println(a, HEX);
		//Serial.println(data[0], HEX);
		//Serial.println(data[1], HEX);
	} //TODO no checks in place to see if MTI is really addressed!
	else //datagram or stream
	{
		CLEAR_DEST_ALIAS(id);
		SET_DEST_ALIAS(id, a);
	}
}

uint16_t OLCB_CAN_Buffer::getDestAlias(void)
{
	if(isFrameTypeCAN())
	{
		//Serial.println("Not an OpenLCB frame");
		return 0;
	}
	if( (GET_CAN_FRAME_TYPE(id) == CAN_FRAME_TYPE_REGULAR) )
	{
		if(MTI_IS_ADDRESSED(getMTI()) && (length >= 2) )
		{
			uint16_t alias = (((uint16_t)(data[0]) << 8) | data[1]) & 0x0FFF;
			return alias & 0xFFF;
		}
		else
		{
			return 0;
		}
	} //TODO no checks in place to see if MTI is really addressed!
	else if( isDatagram() || isStream() ) //datagram or stream
	{
		return GET_DEST_ALIAS(id);
	}
	
	return 0;
}

void OLCB_CAN_Buffer::setDestNID(OLCB_NodeID *NID)
{
	setDestAlias(NID->alias);
}

uint16_t OLCB_CAN_Buffer::getDestNID(OLCB_NodeID *NID)
{
	NID->alias = getDestAlias();
	return NID->alias;
}


void OLCB_CAN_Buffer::setFrameTypeCAN(void)
{
	id &= ~FRAME_TYPE_MASK;
}

bool OLCB_CAN_Buffer::isFrameTypeCAN(void)
{
	return (id & FRAME_TYPE_MASK) == 0x00000000L;
}

void OLCB_CAN_Buffer::setFrameTypeOpenLcb(void)
{
	id |= FRAME_TYPE_MASK;     
}

bool OLCB_CAN_Buffer::isFrameTypeOpenLcb(void)
{
	return (id & FRAME_TYPE_MASK) == FRAME_TYPE_MASK;
}

void OLCB_CAN_Buffer::setMTI(uint16_t f)
{
	CLEAR_MTI(id);
	SET_MTI(id, f);
}

uint16_t OLCB_CAN_Buffer::getMTI(void)
{
	return GET_MTI(id);
}

void OLCB_CAN_Buffer::setVariableField(uint16_t f)
{
	id &= ~MASK_VARIABLE_FIELD;
	uint32_t temp = f;  // ensure 32 bit arithmetic
	id |=  ((temp << SHIFT_VARIABLE_FIELD) & MASK_VARIABLE_FIELD);
}

uint16_t OLCB_CAN_Buffer::getVariableField(void)
{
	return (id & MASK_VARIABLE_FIELD) >> SHIFT_VARIABLE_FIELD;
}

// end of basic message structure

// start of CAN-level messages

void OLCB_CAN_Buffer::setCID(int i, uint16_t testval, uint16_t alias)
{
	init(alias);
	setFrameTypeCAN();
	uint16_t var =  (( (0x8-i) & 7) << 12) | (testval & 0xFFF); 
	setVariableField(var);
	length=0;
}

bool OLCB_CAN_Buffer::isCID(void)
{
	return isFrameTypeCAN() && (getVariableField()&0x7000) >= 0x4000;
}

void OLCB_CAN_Buffer::setRID(uint16_t alias)
{
	init(alias);
	setFrameTypeCAN();
	setVariableField(RID_VAR_FIELD);
	length=6;
}

bool OLCB_CAN_Buffer::isRID(void)
{
		return isFrameTypeCAN() && (getVariableField() == RID_VAR_FIELD);
}

void OLCB_CAN_Buffer::setAMR(OLCB_NodeID *nid)
{
	init(nid->alias);
	setFrameTypeCAN();
	setVariableField(AMR_VAR_FIELD);
	length=6;
	memcpy(data, nid->val, 6);
}

bool OLCB_CAN_Buffer::isAMR()
{
	return isFrameTypeCAN() && (getVariableField() == AMR_VAR_FIELD);
}

//TODO The source alias isn't getting set right here!
void OLCB_CAN_Buffer::setAMD(OLCB_NodeID *nid)
{
	init(nid->alias);
	setFrameTypeCAN();
	setVariableField(AMD_VAR_FIELD);
	length=6;
	memcpy(data, nid->val, 6);
}

bool OLCB_CAN_Buffer::isAMD()
{
	return isFrameTypeCAN() && (getVariableField() == AMD_VAR_FIELD);
}

void OLCB_CAN_Buffer::setAME(OLCB_NodeID *nid)
{
	setFrameTypeCAN();
	setVariableField(AME_VAR_FIELD);
//put the full node id in the data segment
length = 6;
memcpy(data, nid->val, 6);
}

bool OLCB_CAN_Buffer::isAME()
{
	return isFrameTypeCAN() && (getMTI() == AME_VAR_FIELD);
}

// end of CAN-level messages

// start of OpenLCB format support

bool OLCB_CAN_Buffer::isAddressed()
{
  if( isFrameTypeOpenLcb() && (GET_CAN_FRAME_TYPE(id) == CAN_FRAME_TYPE_REGULAR) && MTI_IS_ADDRESSED(GET_MTI(id)) )
  {
	  return true;
	}
  return false;
}

// end of OpenLCB format and decode support

// start of OpenLCB messages


void OLCB_CAN_Buffer::setRejectOptionalInteraction(OLCB_NodeID* source, OLCB_NodeID* dest, uint16_t MTI, uint16_t code)
{
	init(source->alias);
	setMTI(MTI_OPT_INTERACTION_REJECTED);
	setDestAlias(dest->alias);
	length = 6; //first two byte is dest
	data[2] = (MTI>>8)&0x0F;
	data[3] = MTI & 0xFF;
	data[4] = (code>>8)&0xFF;
	data[5] = code & 0xFF;
}

bool OLCB_CAN_Buffer::isRejectOptionalInteraction(void)
{
	return isFrameTypeOpenLcb() && (getMTI() == MTI_OPT_INTERACTION_REJECTED);
}
	
void OLCB_CAN_Buffer::setPCEventReport(OLCB_NodeID* source, OLCB_Event* eid)
{
init(source->alias);
	setMTI(MTI_PC_EVENT_REPORT);
	length=8;
	loadFromEid(eid);
}

bool OLCB_CAN_Buffer::isPCEventReport(void)
{
		return isFrameTypeOpenLcb() && (getMTI() == MTI_PC_EVENT_REPORT);
}

void OLCB_CAN_Buffer::setLearnEvent(OLCB_NodeID* source, OLCB_Event* eid)
{
	init(source->alias);
	setMTI(MTI_LEARN_EVENT);
	length=8;
	loadFromEid(eid);
}

bool OLCB_CAN_Buffer::isLearnEvent(void)
{
		return isFrameTypeOpenLcb() && (getMTI() == MTI_LEARN_EVENT);
}

void OLCB_CAN_Buffer::setInitializationComplete(OLCB_NodeID* source)
{
	init(source->alias);
	setMTI(MTI_INITIALIZATION_COMPLETE);
	length=6;
	memcpy(data, source->val, 6);
}

bool OLCB_CAN_Buffer::isInitializationComplete(void)
{
		return isFrameTypeOpenLcb() && (getMTI() == MTI_INITIALIZATION_COMPLETE);
}

void OLCB_CAN_Buffer::getEventID(OLCB_Event* evt)
{
	memcpy(evt->val, data, 8);
	//evt->val[0] = data[0];
	//evt->val[1] = data[1];
	//evt->val[2] = data[2];
	//evt->val[3] = data[3];
	//evt->val[4] = data[4];
	//evt->val[5] = data[5];
	//evt->val[6] = data[6];
	//evt->val[7] = data[7];
}

//This is used for messages that contain full nodeiD in payload
void OLCB_CAN_Buffer::getNodeID(OLCB_NodeID* nid)
{
	//Serial.println("In getNodeID");
	if( (length >= 6) && (isInitializationComplete() || isVerifyNID() || isVerifiedNID() || isAMD() || isAMR() || isAME()) )
	{
		uint8_t start = 0;
		//in some cases, there will be a destination alias in the first byte.
		if(isAddressed())
		{
			//Serial.println("message is addressed");
			//Serial.println(getMTI(), HEX);
			//Serial.println(MTI_IS_ADDRESSED(getMTI()), HEX);
			//Serial.println(isAddressed(), HEX);
			start = 2;
		}
		memcpy(nid->val, data+start, 6);
	}
// else just rely on the nid having been initialized to some value, which it will have been; by default 0.0.0.0.0.0
}

void OLCB_CAN_Buffer::setVerifyNIDGlobal(OLCB_NodeID* source)
{
	init(source->alias);
	setMTI(MTI_VERIFY_NID_GLOBAL);
}

void OLCB_CAN_Buffer::setVerifyNIDGlobal(OLCB_NodeID* source, OLCB_NodeID *dest)
{
	init(source->alias);
	setMTI(MTI_VERIFY_NID_GLOBAL);
	length = 6;
	memcpy(data, dest->val, 6);
}

void OLCB_CAN_Buffer::setVerifyNIDAddressed(OLCB_NodeID* source, OLCB_NodeID *dest)
{
	//TODO presumes a valid alias!
	init(source->alias);
	setMTI(MTI_VERIFY_NID_ADDRESSED);
	setDestAlias(dest->alias);
}

bool OLCB_CAN_Buffer::isVerifyNIDGlobal(void)
{
		return isFrameTypeOpenLcb() && (getMTI() == MTI_VERIFY_NID_GLOBAL);
}

bool OLCB_CAN_Buffer::isVerifyNIDAddressed()
{
	return isFrameTypeOpenLcb() && (getMTI() == MTI_VERIFY_NID_ADDRESSED);
}

bool OLCB_CAN_Buffer::isVerifyNID(void)
{
	return isVerifyNIDGlobal() || isVerifyNIDAddressed();
}  

bool OLCB_CAN_Buffer::isVerifiedNID()
{
	return isFrameTypeOpenLcb() && (getMTI() == MTI_VERIFIED_NID);
}

void OLCB_CAN_Buffer::setVerifiedNID(OLCB_NodeID* source)
{
	init(source->alias);
	setMTI(MTI_VERIFIED_NID);
	length=6;
	memcpy(data, source->val, 6);
	//data[0] = nid->val[0];
	//data[1] = nid->val[1];
	//data[2] = nid->val[2];
	//data[3] = nid->val[3];
	//data[4] = nid->val[4];
	//data[5] = nid->val[5];
}

bool OLCB_CAN_Buffer::isIdentifyConsumers(void)
{
		return isFrameTypeOpenLcb() && (getMTI() == MTI_IDENTIFY_CONSUMERS);
}

void OLCB_CAN_Buffer::setConsumerIdentified(OLCB_NodeID *source, OLCB_Event* eid)
{
	init(source->alias);
	setMTI(MTI_CONSUMER_IDENTIFIED_UNKNOWN_VALIDITY);
	length=8;
	loadFromEid(eid);
}

void OLCB_CAN_Buffer::setConsumerIdentifyRange(OLCB_NodeID* source, OLCB_Event* eid, OLCB_Event* mask)
{
	//TODO NO MASK SET! HOW TO TRANSMIT MASK?
	init(source->alias);
	setMTI(MTI_IDENTIFY_CONSUMERS_RANGE);
	length=8;
	loadFromEid(eid);
}

bool OLCB_CAN_Buffer::isIdentifyProducers(void)
{
		return isFrameTypeOpenLcb() && (getMTI() == MTI_IDENTIFY_PRODUCERS);
}

void OLCB_CAN_Buffer::setProducerIdentified(OLCB_NodeID* source, OLCB_Event* eid)
{
	init(source->alias);
	setMTI(MTI_PRODUCER_IDENTIFIED_UNKNOWN_VALIDITY);
	length=8;
	loadFromEid(eid);
}

void OLCB_CAN_Buffer::setProducerIdentifyRange(OLCB_NodeID* source, OLCB_Event* eid, OLCB_Event* mask)
{
	// TODO does send a message, but not complete yet - RGJ 2009-06-14
	init(source->alias);
	setMTI(MTI_IDENTIFY_PRODUCERS_RANGE);
	length=8;
	loadFromEid(eid);
}

bool OLCB_CAN_Buffer::isIdentifyEventsGlobal(void)
{
		return isFrameTypeOpenLcb() && (getMTI() == MTI_IDENTIFY_EVENTS_GLOBAL);
}

bool OLCB_CAN_Buffer::isIdentifyEventsAddressed(void)
{
		return isFrameTypeOpenLcb() && (getMTI() == MTI_IDENTIFY_EVENTS_ADDRESSED);
}

bool OLCB_CAN_Buffer::isIdentifyEvents(void)
{
	return isIdentifyEventsGlobal() || isIdentifyEventsAddressed();
}

void OLCB_CAN_Buffer::loadFromEid(OLCB_Event* eid)
{
	memcpy(data, eid->val, 8);
	//data[0] = eid->val[0];
	//data[1] = eid->val[1];
	//data[2] = eid->val[2];
	//data[3] = eid->val[3];
	//data[4] = eid->val[4];
	//data[5] = eid->val[5];
	//data[6] = eid->val[6];
	//data[7] = eid->val[7];
}

// general, but not efficient
bool OLCB_CAN_Buffer::isDatagram(void)
{
	return isOnlyDatagram() || isFirstDatagram() || isMiddleDatagram() || isLastDatagram();
}

bool OLCB_CAN_Buffer::isLastDatagram(void)
{
	return isFrameTypeOpenLcb() && (GET_CAN_FRAME_TYPE(id) == CAN_FRAME_TYPE_DATAGRAM_LAST);
}

bool OLCB_CAN_Buffer::isFirstDatagram(void)
{
	return isFrameTypeOpenLcb() && (GET_CAN_FRAME_TYPE(id) == CAN_FRAME_TYPE_DATAGRAM_FIRST);
}

bool OLCB_CAN_Buffer::isMiddleDatagram(void)
{
	return isFrameTypeOpenLcb() && (GET_CAN_FRAME_TYPE(id) == CAN_FRAME_TYPE_DATAGRAM_MIDDLE);
}

bool OLCB_CAN_Buffer::isOnlyDatagram(void)
{
	return isFrameTypeOpenLcb() && (GET_CAN_FRAME_TYPE(id) == CAN_FRAME_TYPE_DATAGRAM_ONLY);
}

void OLCB_CAN_Buffer::setOnlyDatagram(OLCB_NodeID* source, OLCB_NodeID* dest)
{
	init(source->alias);
	CLEAR_CAN_FRAME_TYPE(id);
	SET_CAN_FRAME_TYPE(id, CAN_FRAME_TYPE_DATAGRAM_ONLY);
	setDestAlias(dest->alias);
}

void OLCB_CAN_Buffer::setFirstDatagram(OLCB_NodeID* source, OLCB_NodeID* dest)
{
	init(source->alias);
	CLEAR_CAN_FRAME_TYPE(id);
	SET_CAN_FRAME_TYPE(id, CAN_FRAME_TYPE_DATAGRAM_FIRST);
	setDestAlias(dest->alias);
}

void OLCB_CAN_Buffer::setMiddleDatagram(OLCB_NodeID* source, OLCB_NodeID* dest)
{
	init(source->alias);
	CLEAR_CAN_FRAME_TYPE(id);
	SET_CAN_FRAME_TYPE(id, CAN_FRAME_TYPE_DATAGRAM_MIDDLE);
	setDestAlias(dest->alias);
}

void OLCB_CAN_Buffer::setLastDatagram(OLCB_NodeID* source, OLCB_NodeID* dest)
{
	init(source->alias);
	CLEAR_CAN_FRAME_TYPE(id);
	SET_CAN_FRAME_TYPE(id, CAN_FRAME_TYPE_DATAGRAM_LAST);
	setDestAlias(dest->alias);
}

bool OLCB_CAN_Buffer::isDatagramAck()
{
	return isFrameTypeOpenLcb() && (getMTI() == MTI_DATAGRAM_OK);
}

void OLCB_CAN_Buffer::setDatagramAck(OLCB_NodeID* source, OLCB_NodeID* dest)
{
	init(source->alias);
	setMTI(MTI_DATAGRAM_OK);
	setDestAlias(dest->alias);
}

bool OLCB_CAN_Buffer::isDatagramNak()
{
	return isFrameTypeOpenLcb() && (getMTI() == MTI_DATAGRAM_REJECTED);
}

void OLCB_CAN_Buffer::setDatagramNak(OLCB_NodeID* source, OLCB_NodeID* dest, uint16_t reason)
{
	init(source->alias);
	setMTI(MTI_DATAGRAM_REJECTED);
	setDestAlias(dest->alias);
	length = 4; //two for dest alias, two for reason
  data[2] = (reason>>8)&0xFF;//TODO!!
	data[3] = reason & 0xFF;

}

uint16_t OLCB_CAN_Buffer::getDatagramNakErrorCode()
{
	if(length >= 3)
	{
		return (data[1]<<8) | (data[2]);
	}
	else
		return 0;
}

bool OLCB_CAN_Buffer::isStream(void)
{
	return isFrameTypeOpenLcb() && (GET_CAN_FRAME_TYPE(id) == CAN_FRAME_TYPE_STREAM);
}

bool OLCB_CAN_Buffer::isProtocolSupportInquiry(void)
{
	return isFrameTypeOpenLcb() && (getMTI() == MTI_PROTOCOL_SUPPORT_INQUIRY);
}

bool OLCB_CAN_Buffer::isProtocolSupportReply(void)
{
	return isFrameTypeOpenLcb() && (getMTI() == MTI_PROTOCOL_SUPPORT_REPLY);
}

bool OLCB_CAN_Buffer::setProtocolSupportInquiry(OLCB_NodeID* source, OLCB_NodeID* dest)
{
	init(source->alias);
	setMTI(MTI_PROTOCOL_SUPPORT_INQUIRY);
	setDestAlias(dest->alias);
}

bool OLCB_CAN_Buffer::setProtocolSupportReply(OLCB_NodeID* source, OLCB_NodeID* dest)
{
	//Serial.println("in setProtocolSupportReply");
	init(source->alias);
	//Serial.println(id, HEX);
	setMTI(MTI_PROTOCOL_SUPPORT_REPLY);
	//Serial.println(id, HEX);
	setDestAlias(dest->alias);
}



bool OLCB_CAN_Buffer::isSNIIRequest(void)
{
	return isFrameTypeOpenLcb() && (getMTI() == MTI_SNII_REQUEST);
}

bool OLCB_CAN_Buffer::isSNIIReply(void)
{
	return isFrameTypeOpenLcb() && (getMTI() == MTI_SNII_REPLY);
}

void OLCB_CAN_Buffer::setSNIIReply(OLCB_NodeID* source, OLCB_NodeID* dest)
{
	init(source->alias);
	setMTI(MTI_SNII_REPLY);
	setDestAlias(dest->alias);
}
