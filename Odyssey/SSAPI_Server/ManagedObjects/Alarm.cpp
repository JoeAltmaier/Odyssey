//************************************************************************
// FILE:		Alarm.h
//
// PURPOSE:		Implements the alarm object to be used for SSAPI alarms
//************************************************************************

#include "Alarm.h"
#include "Event.h"
#include "AlarmHistory.h"
#include "DdmSSAPI.h"
#include "DeviceManager.h"
#include "SsapiAlarms.h"

ALARM_SOURCE_IDENTIFIER_CELL SourceIdentifierTable[] = {
	{
		METHOD_ADDRESS( Alarm, IdentifySourceForUserManagerAlarms),
		CTS_SSAPI_ALARM_TOO_MANY_WRONG_LOGINS,
	},
};

#define	SOURCE_IDENTIFIER_TABLE_SIZE sizeof(SourceIdentifierTable)/sizeof(SourceIdentifierTable[0])

//************************************************************************
// Alarm:
//
// PURPOSE:		Default constructor
//************************************************************************

Alarm::Alarm( ListenManager *pManager, DdmSSAPI *pDdmSSAPI  )
:ManagedObject( pManager, SSAPI_OBJECT_CLASS_TYPE_ALARM ){

	m_pParmVector		= new ValueSet();
	m_pHistoryVector	= new ValueSet();
	m_pDdmSSAPI			= pDdmSSAPI;
	m_pContext			= NULL;
}


//************************************************************************
// ~Alarm:
//
// PURPOSE:		The destructor
//************************************************************************

Alarm::~Alarm(){

	delete m_pParmVector;
	delete m_pHistoryVector;
	delete m_pContext;
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
Alarm::BuildYourValueSet(){

	ManagedObject::BuildYourValueSet();

	AddInt( m_ec, SSAPI_ALARM_FID_MESSAGE_CODE );
	AddInt( m_severity, SSAPI_ALARM_FID_SEVERITY );
	AddGenericValue( (char *)&m_sourceId, sizeof(m_sourceId), SSAPI_ALARM_FID_SOURCE_OBJECT_ID );
	AddInt( m_sourceManagerId, SSAPI_ALARM_FID_SOURCE_OBJECT_MANAGER );
	AddInt( m_isAcknowledged? 1 : 0, SSAPI_ALARM_FID_IS_ACKNOWLEDGED );
	AddInt( m_isActive? 1 : 0, SSAPI_ALARM_FID_IS_ACTIVE );
	AddInt( m_isClearable? 1 : 0, SSAPI_ALARM_FID_IS_CLEARABLE );
	AddValue( m_pParmVector, SSAPI_ALARM_FID_PARAMETER_VECTOR );
	AddValue( m_pHistoryVector, SSAPI_ALARM_FID_HISTORY_VECTOR );

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
Alarm::BuildYourselfFromYourValueSet(){

	int			temp;
	ValueSet	*pVs;

	ManagedObject::BuildYourselfFromYourValueSet();

	GetInt( SSAPI_ALARM_FID_MESSAGE_CODE, &m_ec );
	GetU16( SSAPI_ALARM_FID_SEVERITY, &m_severity );
	GetGenericValue( (char *)&m_sourceId, sizeof(m_sourceId), SSAPI_ALARM_FID_SOURCE_OBJECT_ID );
	GetInt( SSAPI_ALARM_FID_SOURCE_OBJECT_MANAGER, &m_sourceManagerId );
	GetInt( SSAPI_ALARM_FID_IS_ACKNOWLEDGED, &temp );
	m_isAcknowledged = temp? true : false;
	GetInt( SSAPI_ALARM_FID_IS_ACTIVE, &temp );
	m_isActive = temp? true : false;
	GetInt( SSAPI_ALARM_FID_IS_CLEARABLE, &temp );
	m_isClearable = temp ? true : false;
	pVs = (ValueSet *)GetValue( SSAPI_ALARM_FID_PARAMETER_VECTOR );
	if( pVs )
		*m_pParmVector = *pVs;
	pVs = (ValueSet *)GetValue( SSAPI_ALARM_FID_HISTORY_VECTOR );
	if( pVs )
		*m_pHistoryVector = *pVs;


	return true;
}


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		Populates data members based on the contents of the
//				row provided
//************************************************************************

void 
Alarm::BuildYourselfFromPtsRow( AlarmRecord *pRow, AlarmLogRecord *pHistoryCells, U32 historyCellsMaxCount ){

	Event			*pEvent = new Event( pRow->eventData );
	AlarmHistory	*pAlarmHistory;
	U32				i, u32;
	U8				u8;
	S8				s8;
	U16				u16;
	S16				s16;
	S32				s32, historyNum;
	I64				s64;	
	U64				u64;
	UnicodeString	us;

	m_isAcknowledged	= pRow->acknowledged? true : false;
	m_isActive			= pRow->active? true : false;
	m_isClearable		= pRow->clearable? true : false;
	m_id				= DesignatorId( pRow->rid, GetClassType() );
	m_ec				= pEvent->GetEventCode();
	m_severity			= pEvent->GetSeverity();
	m_numberOfHistoryCells = pRow->numberOfEvents;

	// get context
	m_pContext = new char[ m_contextSize = pRow->cbContext ];
	memcpy( m_pContext, pRow->alarmContext, m_contextSize );

	// Collect history cells
	for( u32 = historyNum = 0 ; u32 < historyCellsMaxCount; u32++, pHistoryCells++ ){
		if( RowId(pRow->rid) == pHistoryCells->alarmRid ){
			pAlarmHistory = new AlarmHistory();
			pAlarmHistory->BuildYourselfFromPtsRow( pHistoryCells );
			pAlarmHistory->BuildYourValueSet();
			m_pHistoryVector->AddValue( pAlarmHistory, historyNum++ );
			delete pAlarmHistory;
		}
	}

	// get source MO
	IdentifySourceObject( m_sourceManagerId, m_sourceId );

	// collect parameters
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

	delete pEvent;
}


//************************************************************************
// IdentifySourceObject:
//
// PURPOSE:		Attempts to identify the MO for the alarm. 
//************************************************************************

void 
Alarm::IdentifySourceObject( int &sourceManagerClassType, DesignatorId &sourceId ){

	U32				index;
	DeviceManager	*pDManager = (DeviceManager *)m_pDdmSSAPI->GetObjectManager(SSAPI_MANAGER_CLASS_TYPE_DEVICE_MANAGER);

	for( index = 0; index < SOURCE_IDENTIFIER_TABLE_SIZE; index++ ){
		if( SourceIdentifierTable[index].eventCode == m_ec ){
			(this->*SourceIdentifierTable[index].pVector)( (U32&)sourceManagerClassType, sourceId );
			return;
		}	
	}
	
	sourceId = pDManager->GetChassisDeviceId();
	sourceManagerClassType = SSAPI_MANAGER_CLASS_TYPE_DEVICE_MANAGER;
	m_isSourceFound = true;
}


//************************************************************************
// Source Identfifiers:
//
//************************************************************************

void 
Alarm::IdentifySourceForUserManagerAlarms( U32 &managerType, DesignatorId &id ){

	SsapiAlarmContext	*pContext = (SsapiAlarmContext *)m_pContext;
	UserManager			*pUManager = (UserManager *)m_pDdmSSAPI->GetObjectManager(SSAPI_MANAGER_CLASS_TYPE_USER_MANAGER);
	DesignatorId		tempId;

	managerType = SSAPI_MANAGER_CLASS_TYPE_USER_MANAGER;
	id			= pContext->GetObjectId();
	if(	pUManager->GetDesignatorIdByRowId( pContext->GetObjectId().GetRowId(), tempId ) )
		m_isSourceFound = true;
	else
		m_isSourceFound = false;
}