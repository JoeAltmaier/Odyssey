//******************************************************************************
// FILE:		LogMetaData.cpp
//
// PURPOSE:		Implements the object used to contain meta data for the system
//				log.
//******************************************************************************

#include "LogMetaData.h"
#include "byte_array_utils.h"

//******************************************************************************
// LogMetaData:
//
// PURPOSE:		Default constructor
//******************************************************************************

LogMetaData::LogMetaData( ListenManager *pListenManager ):
ManagedObject( pListenManager, SSAPI_OBJECT_CLASS_LOG_META_DATA ){

	U32							i;
	FIRST_SEQUENCE_NUMBER		fsn;
	LAST_SEQUENCE_NUMBER		lsn;
	MESSAGE_COUNT				mc;

	m_id = DesignatorId( RowId(), SSAPI_OBJECT_CLASS_LOG_META_DATA );

	// intialize the object
	for( i = Event::GetMinSeverity(); i <= Event::GetMaxSeverity(); i++ ){	
		// first seq. numbers
		fsn.sequenceNumber = 0;
		fsn.severity = (U32)i;
		m_firstSequenceNumbers.AddGenericValue( (char *)&fsn, sizeof(fsn), i);

		// last seq numbers
		lsn.sequenceNumber = 0;
		lsn.severity = (U32)i;
		m_firstSequenceNumbers.AddGenericValue( (char *)&lsn, sizeof(lsn), i);

		// message counts
		mc.severity = (U32)i;
		mc.count = 0;
		m_messageCounts.AddGenericValue( (char *)&mc, sizeof(mc), i );
	}
}


//******************************************************************************
// ~LogMetaData:
//
// PURPOSE:		The destructor
//******************************************************************************

LogMetaData::~LogMetaData(){
}


//******************************************************************************
// BuildYourSelf:
//
// PURPOSE:		Populates data members based on the query message supplied
//******************************************************************************

bool
LogMetaData::BuildYourSelf( MsgQueryLogMetaData *pMsg ){

	U32							i, temp;
	FIRST_SEQUENCE_NUMBER		fsn;
	LAST_SEQUENCE_NUMBER		lsn;
	MESSAGE_COUNT				mc;

	m_firstSequenceNumbers.Clear();
	m_lastSequenceNumbers.Clear();
	m_messageCounts.Clear();



	for( i = Event::GetMinSeverity(); i <= Event::GetMaxSeverity(); i++ ){	
		// first seq. numbers
		temp = pMsg->GetFirstSequenceNumber( i );
		byte_array_utils::Write( (char *)&fsn.sequenceNumber, (char *)&temp, sizeof(temp) );
		byte_array_utils::Write( (char *)&fsn.severity, (char *)&i, sizeof(i) );
		m_firstSequenceNumbers.AddGenericValue( (char *)&fsn, sizeof(fsn), i);

		// last seq numbers
		temp = pMsg->GetLastSequenceNumber( i );
		byte_array_utils::Write( (char *)&lsn.sequenceNumber, (char *)&temp, sizeof(temp) );
		byte_array_utils::Write( (char *)&lsn.severity, (char *)&i, sizeof(i) );

		m_lastSequenceNumbers.AddGenericValue( (char *)&lsn, sizeof(lsn), i);

		// message counts
		temp = pMsg->GetEntryCount( i );
		byte_array_utils::Write( (char *)&mc.severity, (char *)&i, sizeof(U32) );
		byte_array_utils::Write( (char *)&mc.count, (char *)&temp, sizeof(temp) );
		m_messageCounts.AddGenericValue( (char *)&mc, sizeof(mc), i );
	}


	return true;
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
LogMetaData::BuildYourValueSet(){

	ManagedObject::BuildYourValueSet();

	AddValue( &m_firstSequenceNumbers, SSAPI_LOG_META_DATA_FID_FIRST_SEQ_NUMBER_VECTOR );
	AddValue( &m_lastSequenceNumbers, SSAPI_LOG_META_DATA_FID_LAST_SEQ_NUMBER_VECTOR );
	AddValue( &m_messageCounts, SSAPI_LOG_META_DATA_FID_MESSAGE_COUNT_VECTOR );

	return true;
}

