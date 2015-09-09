//************************************************************************
// FILE:		Alarm.h
//
// PURPOSE:		Defines the alarm object to be used for SSAPI alarms
//************************************************************************

#ifndef __ALARM_MO_H__
#define	__ALARM_MO_H__

#include "ManagedObject.h"
#include "alarmRecordTable.h"


class DdmSSAPI;
struct AlarmLogRecord;
#ifdef WIN32
#pragma pack(4)
#endif

class Alarm : public ManagedObject{

	STATUS			m_ec;
	U16				m_severity;
	ValueSet		*m_pParmVector;	// contains pointers to copes of parameters
	ValueSet		*m_pHistoryVector; // contains history cells
	bool			m_isAcknowledged;
	bool			m_isActive;
	bool			m_isClearable;
	int				m_sourceManagerId;
	DesignatorId	m_sourceId;

	U32				m_numberOfHistoryCells;	// internal
	DdmSSAPI		*m_pDdmSSAPI;
	void			*m_pContext;
	U32				m_contextSize;
	bool			m_isSourceFound;

public:

	typedef void (Alarm::*SOURCE_IDENTIFIER)( U32&, DesignatorId& );

//************************************************************************
// Alarm:
//
// PURPOSE:		Default constructor
//************************************************************************

Alarm( ListenManager *pManager, DdmSSAPI *pDdmSSAPI );


//************************************************************************
// ~Alarm:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~Alarm();


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

virtual bool BuildYourValueSet();


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

virtual bool BuildYourselfFromYourValueSet();


//************************************************************************
// operator=:
//************************************************************************

virtual const ValueSet& operator=(const ValueSet& obj ){ return *((ValueSet*)this) = obj; }


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		Populates data members based on the contents of the
//				row provided
//************************************************************************

void BuildYourselfFromPtsRow( AlarmRecord *pRow, AlarmLogRecord *pHistoryCells, U32 historyCellsMaxCount );


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new Alarm( GetListenManager(), m_pDdmSSAPI ); }


//************************************************************************
// Accessors:
//************************************************************************

DesignatorId	GetSourceId() const { return m_sourceId; }
bool			GetIsClearable() const { return m_isClearable; }		
bool			GetIsActive() const { return m_isActive; }
bool			GetIsAcknowledged() const { return m_isAcknowledged; }
bool			GetIsSourceFound() const { return m_isSourceFound; }

private:

//************************************************************************
// IdentifySourceObject:
//
// PURPOSE:		Attempts to identify the MO for the alarm. 
//************************************************************************

void IdentifySourceObject( int &sourceManagerClassType, DesignatorId &sourceId );


public:

//************************************************************************
// Source Identfifiers:
//
//************************************************************************

void IdentifySourceForUserManagerAlarms( U32 &managerType, DesignatorId &id );

};

struct ALARM_SOURCE_IDENTIFIER_CELL{

	Alarm::SOURCE_IDENTIFIER	pVector;
	U32							eventCode;
};


#endif	// __ALARM_MO_H__