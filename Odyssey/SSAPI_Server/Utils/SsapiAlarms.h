//************************************************************************
// FILE:		SsapiAlarms.h
//
// PURPOSE:		Defines alarms submittable by the SSAPI Ddm
//************************************************************************

#ifndef __SSAPI_ALARMS_H__
#define	__SSAPI_ALARMS_H__


#include "SsapiTypes.h"
#include "CtTypes.h"
#include "Ssapi_Codes.h"
#include "DesignatorId.h"

#ifdef WIN32
#pragma pack(4)
#endif



//************************************************************************
// class SsapiAlarmContext
//
// PURPOSE:		Base class for all SSAPI alarms
//
// USAGE: must be derived from for each alarm type
//************************************************************************

class SsapiAlarmContext {

	U32				m_managerType;
	U32				m_eventCode;
	DesignatorId	m_objectId;
	U32				m_contextSize;


protected:

//************************************************************************
// SsapiAlarmContext:
//
// PURPOSE:			Default constructor
//************************************************************************

SsapiAlarmContext( U32 managerType, U32 eventCode, DesignatorId objectId, U32 size ){
	m_managerType		= managerType;
	m_eventCode			= eventCode;
	m_objectId			= objectId;
	m_contextSize		= size;
}


public:

//************************************************************************
// Accessors:
//************************************************************************

U32	GetManagerType() const { return m_managerType; }
U32 GetEventCode() const { return m_eventCode; }
DesignatorId GetObjectId() const { return m_objectId; }
U32	GetSize() const { return m_contextSize; }


};


//************************************************************************
// class SsapiAlarmContextTooManyWrongLogins:
//
// PURPOSE:		Defines context for the "Too many wrong logins" alarm
//************************************************************************

class SsapiAlarmContextTooManyWrongLogins : public SsapiAlarmContext {

public:


//************************************************************************
// SsapiAlarmContextTooManyWrongLogins:
//
// PURPOSE:		Default contructor
//************************************************************************

SsapiAlarmContextTooManyWrongLogins( DesignatorId objectId )
:SsapiAlarmContext( SSAPI_MANAGER_CLASS_TYPE_ALARM_MANAGER,
					CTS_SSAPI_ALARM_TOO_MANY_WRONG_LOGINS,
					objectId,
					sizeof(SsapiAlarmContextTooManyWrongLogins) ) {}

};

#endif