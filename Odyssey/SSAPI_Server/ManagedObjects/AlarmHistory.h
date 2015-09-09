//************************************************************************
// FILE:		AlarmHistory.h
//
// PURPOSE:		Defines the object used to represent an alarm history
//				information cell.
//************************************************************************

#ifndef __ALARM_HISTORY_H__
#define	__ALARM_HISTORY_H__


#include "ValueSet.h"
#include "SSAPITypes.h"
#include "UnicodeString.h"
#include "AlarmLogTable.h"

#ifdef WIN32
#pragma pack(4)
#endif

class AlarmHistory : public ValueSet {
	
	I64				m_timeStamp;
	UnicodeString	m_userName;
	UnicodeString	m_userNotes;
	int				m_actionType;


public:


//************************************************************************
// AlarmHistory:
//
// PURPOSE:		Default constructor
//************************************************************************

AlarmHistory();


//************************************************************************
// ~AlarmHistory:
//
// PURPOSE:		The destructor
//************************************************************************

~AlarmHistory();


//************************************************************************
// BuildYourValueSet:
//
// PURPOSE:		Packs all data members into the underlying value set
//************************************************************************

void BuildYourValueSet();


//************************************************************************
// BuildYourselfFromYourValueSet:
//
// PURPOSE:		Unpacks the underlying value set in the appropriate data
//				members.
//************************************************************************

void BuildYourselfFromYourValueSet();


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		Populates data members based on the information provided
//				in the row.
//************************************************************************

void BuildYourselfFromPtsRow( AlarmLogRecord *pRow );
};

#endif // __ALARM_HISTORY_H__