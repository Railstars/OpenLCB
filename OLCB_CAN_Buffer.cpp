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
  	data[(position>=length)?(length-1):position];
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
  	id = 0x1FFFF000 | (alias & MASK_SRC_ALIAS);
  }

  // start of basic message structure

  void OLCB_CAN_Buffer::setSourceAlias(uint16_t a) {
    id &= ~MASK_SRC_ALIAS;
    id = id | (a & MASK_SRC_ALIAS);
  }
  
  uint16_t OLCB_CAN_Buffer::getSourceAlias() {
      return id & MASK_SRC_ALIAS;
  }
  
  void OLCB_CAN_Buffer::setSourceNID(OLCB_NodeID *NID)
  {
    id &= ~MASK_SRC_ALIAS;
    id = id | (NID->alias & MASK_SRC_ALIAS);
    _source.copy(NID);
  }
  
  void OLCB_CAN_Buffer::getSourceNID(OLCB_NodeID *NID)
  {
    if(_source.empty()) //if _source.alias == 0, basically.
    {
      _source.set(0,0,0,0,0,0);
    }
    _source.alias = (id & MASK_SRC_ALIAS);
    NID->copy(&_source);
  }

  void OLCB_CAN_Buffer::setFrameTypeCAN() {
    id &= ~MASK_FRAME_TYPE;     
  }
  
  bool OLCB_CAN_Buffer::isFrameTypeCAN() {
    return (id & MASK_FRAME_TYPE) == 0x00000000L;
  }

  void OLCB_CAN_Buffer::setFrameTypeOpenLcb() {
    id |= MASK_FRAME_TYPE;     
  }
  
  bool OLCB_CAN_Buffer::isFrameTypeOpenLcb() {
    return (id & MASK_FRAME_TYPE) == MASK_FRAME_TYPE;
  }

  void OLCB_CAN_Buffer::setVariableField(uint16_t f) {
    id &= ~MASK_VARIABLE_FIELD;
    uint32_t temp = f;  // ensure 32 bit arithmetic
    id |=  ((temp << SHIFT_VARIABLE_FIELD) & MASK_VARIABLE_FIELD);
  }

  uint16_t OLCB_CAN_Buffer::getVariableField() {
    return (id & MASK_VARIABLE_FIELD) >> SHIFT_VARIABLE_FIELD;
  }
  
  // end of basic message structure
  
  // start of CAN-level messages
 
  void OLCB_CAN_Buffer::setCID(int i, uint16_t testval, uint16_t alias) {
  	init(alias);
    setFrameTypeCAN();
    uint16_t var =  (( (0x8-i) & 7) << 12) | (testval & 0xFFF); 
    setVariableField(var);
    length=0;
  }

  bool OLCB_CAN_Buffer::isCID() {
    return isFrameTypeCAN() && (getVariableField()&0x7000) >= 0x4000;
  }

  void OLCB_CAN_Buffer::setRID(uint16_t alias) {
  	init(alias);
    setFrameTypeCAN();
    setVariableField(RID_VAR_FIELD);
    length=6;
  }

  bool OLCB_CAN_Buffer::isRID() {
      return isFrameTypeCAN() && getVariableField() == RID_VAR_FIELD;
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
    return isFrameTypeCAN() && getVariableField() == AMR_VAR_FIELD;
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
  	return isFrameTypeCAN() && getVariableField() == AMD_VAR_FIELD;
  }
  
  void OLCB_CAN_Buffer::setAME(OLCB_NodeID *nid)
  {
  	setFrameTypeCAN();
  	setDestinationNID(nid); //sets the destination
  	setVariableField(AME_VAR_FIELD);
  }
  
  bool OLCB_CAN_Buffer::isAME()
  {
  	return isFrameTypeCAN() && getVariableField() == AME_VAR_FIELD;
  }

  // end of CAN-level messages
  
  // start of OpenLCB format support
  
  uint16_t OLCB_CAN_Buffer::getOpenLcbFormat() {
      return (getVariableField() & MASK_OPENLCB_FORMAT) >> SHIFT_OPENLCB_FORMAT;
  }

  void OLCB_CAN_Buffer::setOpenLcbFormat(uint16_t i) {
      uint16_t now = getVariableField() & ~MASK_OPENLCB_FORMAT;
      setVariableField( ((i << SHIFT_OPENLCB_FORMAT) & MASK_OPENLCB_FORMAT) | now);
  }

  // is the variable field a destID?
  bool OLCB_CAN_Buffer::isOpenLcDestIdFormat() {
      return ( getOpenLcbFormat() == MTI_FORMAT_ADDRESSED_NON_DATAGRAM);
  }
  
  // is the variable field a stream ID?
  bool OLCB_CAN_Buffer::isOpenLcbStreamIdFormat() {
      return ( getOpenLcbFormat() == MTI_FORMAT_STREAM_CODE);
  }
  
  void OLCB_CAN_Buffer::setOpenLcbMTI(uint16_t fmt, uint16_t mtiHeaderByte) {
        setFrameTypeOpenLcb();
        setVariableField(mtiHeaderByte);
        setOpenLcbFormat(fmt);  // order matters here
  }
  
  bool OLCB_CAN_Buffer::isOpenLcbMTI(uint16_t fmt, uint16_t mtiHeaderByte)
  {
      return isFrameTypeOpenLcb() 
                && ( getOpenLcbFormat() == fmt )
                && ( (getVariableField()&~MASK_OPENLCB_FORMAT) == mtiHeaderByte );
  }
  
  bool OLCB_CAN_Buffer::isOpenLcbMTI(uint16_t fmt) {
      return isFrameTypeOpenLcb() 
                && ( getOpenLcbFormat() == fmt );
  }

  // end of OpenLCB format and decode support
  
  // start of OpenLCB messages
  
  bool OLCB_CAN_Buffer::isRejectOptionalInteraction(void)
  {
    if(! (isFrameTypeOpenLcb() && (getOpenLcbFormat() == MTI_FORMAT_ADDRESSED_NON_DATAGRAM)) )
      return false;
    return (data[0] == ((MTI_OPTION_INT_REJECTED)&0xFF) );
  }
  
  void OLCB_CAN_Buffer::setRejectOptionalInteraction(OLCB_NodeID* source, OLCB_NodeID* dest)
  {
    init(dest->alias);
    setOpenLcbMTI(MTI_FORMAT_ADDRESSED_NON_DATAGRAM,source->alias);
    length=1;
    data[0] = MTI_OPTION_INT_REJECTED;
  }
    
  void OLCB_CAN_Buffer::setPCEventReport(OLCB_Event* eid) {
//    init(nodeAlias);
    setOpenLcbMTI(MTI_FORMAT_SIMPLE_MTI,MTI_PC_EVENT_REPORT);
    length=8;
    loadFromEid(eid);
  }
  
  bool OLCB_CAN_Buffer::isPCEventReport() {
      return isOpenLcbMTI(MTI_FORMAT_SIMPLE_MTI, MTI_PC_EVENT_REPORT);
  }

  void OLCB_CAN_Buffer::setLearnEvent(OLCB_Event* eid) {
//    init(nodeAlias);
    setOpenLcbMTI(MTI_FORMAT_SIMPLE_MTI,MTI_LEARN_EVENT);
    length=8;
    loadFromEid(eid);
  }

  bool OLCB_CAN_Buffer::isLearnEvent() {
      return isOpenLcbMTI(MTI_FORMAT_SIMPLE_MTI, MTI_LEARN_EVENT);
  }

  void OLCB_CAN_Buffer::setInitializationComplete(OLCB_NodeID* nid) {
    init(nid->alias);
    setOpenLcbMTI(MTI_FORMAT_COMPLEX_MTI,MTI_INITIALIZATION_COMPLETE);
    length=6;
    memcpy(data, nid->val, 6);
    //data[0] = nid->val[0];
    //data[1] = nid->val[1];
    //data[2] = nid->val[2];
    //data[3] = nid->val[3];
    //data[4] = nid->val[4];
    //data[5] = nid->val[5];
  }
  
  bool OLCB_CAN_Buffer::isInitializationComplete() {
      return isOpenLcbMTI(MTI_FORMAT_COMPLEX_MTI, MTI_INITIALIZATION_COMPLETE);
  }
  
  void OLCB_CAN_Buffer::getEventID(OLCB_Event* evt) {
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
  
  //This is used for VerifyID, and presumes that the NID to verify is contained in the first 6 bytes of the frame.
  //TODO if it's an addressed verifyID, need to offset by 1!
  void OLCB_CAN_Buffer::getNodeID(OLCB_NodeID* nid)
  {
  	uint8_t start = 0;
  	if(isVerifyNIDAddressed() || isIdentifyEventsAddressed())
  		start = 1;
  	if(length >= (6+start))
  	{
		memcpy(nid->val, data+start, 6);
	}
	// else just rely on the nid having been initialized to some value, which it will have been; by default 0.0.0.0.0.0
  }
  
  bool OLCB_CAN_Buffer::getDestinationNID(OLCB_NodeID *nid)
  {
    //Only do anything if this is an addressed frame
    if( (getOpenLcbFormat() == MTI_FORMAT_ADDRESSED_DATAGRAM) ||
        (getOpenLcbFormat() == MTI_FORMAT_ADDRESSED_DATAGRAM_LAST) ||
        (getOpenLcbFormat() == MTI_FORMAT_ADDRESSED_NON_DATAGRAM) )
    {
      if(_destination.empty())
      {
        _destination.set(0,0,0,0,0,0);
      }
      _destination.alias = (id & MASK_DEST_ALIAS) >> SHIFT_VARIABLE_FIELD;
      
      nid->copy(&_destination);
      return true;
    }
    return false;
  }
  
  void OLCB_CAN_Buffer::setDestinationNID(OLCB_NodeID *nid)
  {
    id &= ~MASK_DEST_ALIAS;
    id = id | ( ((uint32_t)(nid->alias) << SHIFT_VARIABLE_FIELD) & MASK_DEST_ALIAS );
    _destination.copy(nid);
  }
  
  void OLCB_CAN_Buffer::setVerifyNID(OLCB_NodeID* nid)
  {
//    init(nodeAlias);
    setOpenLcbMTI(MTI_FORMAT_SIMPLE_MTI, MTI_VERIFY_NID_GLOBAL);
    length = 6;
    memcpy(data, nid->val, 6);
  }

  bool OLCB_CAN_Buffer::isVerifyNIDGlobal() {
      return isOpenLcbMTI(MTI_FORMAT_SIMPLE_MTI, MTI_VERIFY_NID_GLOBAL);
  }

  bool OLCB_CAN_Buffer::isVerifyNIDAddressed()
  {
      if (getOpenLcbFormat() != MTI_FORMAT_ADDRESSED_NON_DATAGRAM) return false;
      if (length == 0) return false;
      if (data[0] != MTI_VERIFY_NID_ADDRESSED) return false;
      return true;
  }  
  
  bool OLCB_CAN_Buffer::isVerifiedNID()
  {
    return isOpenLcbMTI(MTI_FORMAT_SIMPLE_MTI, MTI_VERIFIED_NID);
  }

  void OLCB_CAN_Buffer::setVerifiedNID(OLCB_NodeID* nid) {
//    init(nodeAlias);
    setOpenLcbMTI(MTI_FORMAT_SIMPLE_MTI, MTI_VERIFIED_NID);
    length=6;
    memcpy(data, nid->val, 6);
    //data[0] = nid->val[0];
    //data[1] = nid->val[1];
    //data[2] = nid->val[2];
    //data[3] = nid->val[3];
    //data[4] = nid->val[4];
    //data[5] = nid->val[5];
  }

  bool OLCB_CAN_Buffer::isIdentifyConsumers() {
      return isOpenLcbMTI(MTI_FORMAT_SIMPLE_MTI, MTI_IDENTIFY_CONSUMERS);
  }

  void OLCB_CAN_Buffer::setConsumerIdentified(OLCB_Event* eid) {
//    init(nodeAlias);
    setOpenLcbMTI(MTI_FORMAT_COMPLEX_MTI,MTI_CONSUMER_IDENTIFIED);
    length=8;
    loadFromEid(eid);
  }

  void OLCB_CAN_Buffer::setConsumerIdentifyRange(OLCB_Event* eid, OLCB_Event* mask) {
    // does send a message, but not complete yet - RGJ 2009-06-14
//    init(nodeAlias);
    setOpenLcbMTI(MTI_FORMAT_COMPLEX_MTI,MTI_IDENTIFY_CONSUMERS_RANGE);
    length=8;
    loadFromEid(eid);
  }

  bool OLCB_CAN_Buffer::isIdentifyProducers() {
      return isOpenLcbMTI(MTI_FORMAT_SIMPLE_MTI, MTI_IDENTIFY_PRODUCERS);
  }

  void OLCB_CAN_Buffer::setProducerIdentified(OLCB_Event* eid) {
//    init(nodeAlias);
    setOpenLcbMTI(MTI_FORMAT_COMPLEX_MTI,MTI_PRODUCER_IDENTIFIED);
    length=8;
    loadFromEid(eid);
  }

  void OLCB_CAN_Buffer::setProducerIdentifyRange(OLCB_Event* eid, OLCB_Event* mask) {
    // does send a message, but not complete yet - RGJ 2009-06-14
//    init(nodeAlias);
    setOpenLcbMTI(MTI_FORMAT_COMPLEX_MTI,MTI_IDENTIFY_PRODUCERS_RANGE);
    length=8;
    loadFromEid(eid);
  }

  bool OLCB_CAN_Buffer::isIdentifyEvents() {
      return isOpenLcbMTI(MTI_FORMAT_SIMPLE_MTI, MTI_IDENTIFY_EVENTS);
  }
  
  bool OLCB_CAN_Buffer::isIdentifyEventsAddressed() {
      if (getOpenLcbFormat() != MTI_FORMAT_ADDRESSED_NON_DATAGRAM) return false;
      if (length == 0) return false;
      if (data[0] != MTI_IDENTIFY_EVENTS_ADDRESSED) return false;
      return true;
  }  

  void OLCB_CAN_Buffer::loadFromEid(OLCB_Event* eid) {
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
  bool OLCB_CAN_Buffer::isDatagram() {
    return isFrameTypeOpenLcb() &&
      ((getOpenLcbFormat() == MTI_FORMAT_ADDRESSED_DATAGRAM) || 
       (getOpenLcbFormat() == MTI_FORMAT_ADDRESSED_DATAGRAM_LAST));
  }
  
  // just checks 1st, assumes datagram already checked.
  bool OLCB_CAN_Buffer::isLastDatagram() {
    return isFrameTypeOpenLcb() && (getOpenLcbFormat() == MTI_FORMAT_ADDRESSED_DATAGRAM_LAST);
  }
  
  bool OLCB_CAN_Buffer::isDatagramAck()
  {
    if(! (isFrameTypeOpenLcb() && (getOpenLcbFormat() == MTI_FORMAT_ADDRESSED_NON_DATAGRAM)) )
      return false;
    return (data[0] == ((MTI_DATAGRAM_RCV_OK)&0xFF) );
  }
  
  bool OLCB_CAN_Buffer::isDatagramNak()
  {
    if(! (isFrameTypeOpenLcb() && (getOpenLcbFormat() == MTI_FORMAT_ADDRESSED_NON_DATAGRAM)) )
      return false;
    return (data[0] == ((MTI_DATAGRAM_REJECTED)&0xFF) );
  }
  
  uint16_t OLCB_CAN_Buffer::getDatagramNakErrorCode()
  {
  	if(length >= 3)
  	{
  		//Serial.print("NAK error code: ");
  		//Serial.println((data[1]<<8) | (data[2]), HEX);
	    return (data[1]<<8) | (data[2]);
	}
	else
		return 0;
  }
  
  
bool OLCB_CAN_Buffer::isProtocolSupportInquiry() {
  if (getOpenLcbFormat() != MTI_FORMAT_ADDRESSED_NON_DATAGRAM) return false;
  if (length == 0) return false;
  if ( ((data[0]<<8)|data[1]) != MTI_PROTOCOL_SUPPORT_INQUIRY) return false;
  return true;
}  
