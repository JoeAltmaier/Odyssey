//************************************************************************
// FILE:		AlarmHistory.cpp
//
// PURPOSE:		Implements the object used to represent an alarm history
//				information cell.
//************************************************************************


#include "AlarmHistory.h"

//************************************************************************
// AlarmHistory:
//
// PURPOSE:		Default constructor
//************************************************************************

AlarmHistory::AlarmHistory() : ValueSet( NULL ){
}


//************************************************************************
// ~AlarmHistory:
//
// PURPOSE:		The destructor
//************************************************************************

AlarmHistory::~AlarmHistory(){
}


//************************************************************************
// BuildYourValueSet:
//
// PURPOSE:		Packs all data members into the underlying value set
//************************************************************************

void 
AlarmHistory::BuildYourValueSet(){

	AddInt64( m_timeStamp, SSAPI_ALARM_HISTORY_TIME_STAMP );
	AddString( &m_userName, SSAPI_ALARM_HISTORY_USER_NAME );
	AddString( &m_userNotes, SSAPI_ALARM_HISTORY_USER_NOTES );
	AddInt( m_actionType, SSAPI_ALARM_HISTORY_ACTION_TYPE );
}


//************************************************************************
// BuildYourselfFromYourValueSet:
//
// PURPOSE:		Unpacks the underlying value set in the appropriate data
//				members.
//************************************************************************

void 
AlarmHistory::BuildYourselfFromYourValueSet(){

	GetInt64( SSAPI_ALARM_HISTORY_TIME_STAMP, &m_timeStamp );
	GetString( SSAPI_ALARM_HISTORY_USER_NAME, &m_userName );
	GetString( SSAPI_ALARM_HISTORY_USER_NOTES, &m_userNotes );
	GetInt( SSAPI_ALARM_HISTORY_ACTION_TYPE, &m_actionType );
}


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		Populates data members based on the information provided
//				in the row.
//************************************************************************

void 
AlarmHistory::BuildYourselfFromPtsRow( AlarmLogRecord *pRow ){

	memcpy( &m_timeStamp, &pRow->timeStamp, sizeof(pRow->timeStamp) );
	m_actionType	= pRow->action; 
	m_userName		= UnicodeString( (void *)&pRow->userName );
	m_userNotes		= UnicodeString( (void *)&pRow->notes );
}