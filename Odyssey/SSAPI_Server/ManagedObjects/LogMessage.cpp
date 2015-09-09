//************************************************************************
// FILE:		LogMessage.cpp
//
// PURPOSE:		Implements the class used for wrapping up event log messages
//				before they are passed to the CS SSAPI
//************************************************************************

#include "LogMessage.h"
#include "SList.h"


//************************************************************************
// LogMessage:
//
// PURPOSE:		Default constructor
//************************************************************************

LogMessage::LogMessage( ListenManager *pListenManager, Event *pEvent )
:ManagedObject( pListenManager, SSAPI_OBJECT_CLASS_TYPE_LOG_MESSAGE ){

	U32				i, u32, paramCount;
	U8				u8;
	S8				s8;
	U16				u16;
	S16				s16;
	S32				s32;
	I64				s64;	
	U64				u64;
	UnicodeString	us;
	U32				cb  = sizeof(Event);

	m_sequenceNumber	= pEvent->GetSequenceNum();
	m_timeStamp			= pEvent->GetTimestamp();
	m_ec				= pEvent->GetEventCode();
	m_slot				= pEvent->GetSlot();
	m_did				= pEvent->GetDID();
	m_vdn				= pEvent->GetVDN();
	m_pParmVector		= new ValueSet;
	m_id				= DesignatorId( RowId(), GetClassType(), m_sequenceNumber );
	m_severity			= pEvent->GetSeverity();
	m_facility			= pEvent->GetFacility();
	paramCount			= pEvent->GetParameterCount();

	for( i = 0; i < pEvent->GetParameterCount(); i++ ){
		switch( pEvent->GetParameterType(i) ){
			case Event::CHAR_PARM:
			case Event::U8_PARM:
				memcpy( &u8, pEvent->GetPParameter(i), sizeof(u8) );
				m_pParmVector->AddU8( u8, i );
				break;

			case Event::S8_PARM:
				memcpy( &s8, pEvent->GetPParameter(i), pEvent->GetParameterSize(i) );
				m_pParmVector->AddInt8( s8, i );
				break;

			case Event::S16_PARM:
				memcpy( &s16, pEvent->GetPParameter(i), pEvent->GetParameterSize(i) );
				m_pParmVector->AddInt16( s16, i );
				break;

			case Event::U16_PARM: 
				memcpy( &u16, pEvent->GetPParameter(i), pEvent->GetParameterSize(i) );
				m_pParmVector->AddU16( u16, i );
				break;

			case Event::S32_PARM:
				memcpy( &s32, pEvent->GetPParameter(i), pEvent->GetParameterSize(i) );
				m_pParmVector->AddInt( s32, i );
				break;

			case Event::U32_PARM:
				memcpy( &u32, pEvent->GetPParameter(i), pEvent->GetParameterSize(i) );
				m_pParmVector->AddU32( u32, i );
				break;

			case Event::S64_PARM:
				memcpy( &s64, pEvent->GetPParameter(i), pEvent->GetParameterSize(i) );
				m_pParmVector->AddInt64( s64, i );
				break;

			case Event::U64_PARM:
				memcpy( &u64, pEvent->GetPParameter(i), pEvent->GetParameterSize(i) );
				m_pParmVector->AddU64( u64, i );
				break;

			case Event::HEX_PARM:
				m_pParmVector->AddGenericValue( (char *)pEvent->GetPParameter(i), pEvent->GetParameterSize(i), i );
				break;

			case Event::STR_PARM:
				us = StringClass( (char *)pEvent->GetPParameter(i) );
				m_pParmVector->AddString( &us, i );
				break;

			case Event::USTR_PARM:
				us = UnicodeString( (void *)pEvent->GetPParameter(i) );
				m_pParmVector->AddString( &us, i );
				break;

			default:
				ASSERT(0);
				break;
		}
	}
}


//************************************************************************
// ~LogMessage:
//
// PURPOSE:		The destructor
//************************************************************************

LogMessage::~LogMessage(){

	delete m_pParmVector;
}


//************************************************************************
// BuildYourValueSet:
//
// PURPOSE:		The method is responsible for adding all data to be 
//				transfered to a client into its value set. All derived 
//				objects must override this method if they have data members
//				they want client proxies to have. 
//
// NOTE:		If a derived object overrides this this method, it MUST call
//				it in its base class BEFORE doing any work!.
//
// RETURN:		true:		success
//************************************************************************

bool 
LogMessage::BuildYourValueSet(){

	ManagedObject::BuildYourValueSet();

	AddU32( m_sequenceNumber, SSAPI_LOG_MESSAGE_FID_SEQ_NUMBER );
	AddInt64( m_timeStamp, SSAPI_LOG_MESSAGE_FID_TIME_STAMP );
	AddInt( m_ec, SSAPI_LOG_MESSAGE_FID_EVENT_CODE );
	AddInt( m_slot, SSAPI_LOG_MESSAGE_FID_SLOT );
	AddInt( m_facility, SSAPI_LOG_MESSAGE_FID_FACILITY );
	AddInt( m_severity, SSAPI_LOG_MESSAGE_FID_SEVERITY );
	AddInt( m_did, SSAPI_LOG_MESSAGE_FID_DID );
	AddInt( m_vdn, SSAPI_LOG_MESSAGE_FID_VDN );
	AddValue( m_pParmVector, SSAPI_LOG_MESSAGE_FID_PARAMETER_VECTOR );

	return true;
}


//************************************************************************
// BuildYourselfFromYourValueSet:
//
// PURPOSE:		Populates data members based on the underlying value set
//
// NOTE:		All subclasses that override this method must call to it
//				somewhere in the overriding method
//
// RETURN:		true:		success
//************************************************************************

bool 
LogMessage::BuildYourselfFromYourValueSet(){

	ASSERT(0);	// msu tnever be called

	return false;
}


