//******************************************************************************
// FILE:		LogMetaData.h
//
// PURPOSE:		Defines the object used to contain meta data for the system
//				log.
//******************************************************************************

#ifndef __LOG_META_DATA_H__
#define	__LOG_META_DATA_H__

#include "Address.h"
#include "LogMasterMessages.h"
#include "ManagedObject.h"
#include "Event.h"
#ifdef WIN32
#pragma pack(4)
#endif



class LogMetaData : public ManagedObject{

	ValueSet			m_firstSequenceNumbers;
	ValueSet			m_lastSequenceNumbers;
	ValueSet			m_messageCounts;



public:

//******************************************************************************
// LogMetaData:
//
// PURPOSE:		Default constructor
//******************************************************************************

LogMetaData( ListenManager *pListenManager );


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new LogMetaData( GetListenManager() ); }


//******************************************************************************
// ~LogMetaData:
//
// PURPOSE:		The destructor
//******************************************************************************

virtual ~LogMetaData();


//******************************************************************************
// BuildYourSelf:
//
// PURPOSE:		Populates data members based on the query message supplied
//******************************************************************************

bool BuildYourSelf( MsgQueryLogMetaData *pMsg );


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
// Assignment operator overloaded
//************************************************************************

const ValueSet& operator=(const ValueSet& obj ){ *(ValueSet *)this = obj; return obj; }


};


struct FIRST_SEQUENCE_NUMBER{
	U32			severity;
	U32			sequenceNumber;
};

struct LAST_SEQUENCE_NUMBER{
	U32			severity;
	U32			sequenceNumber;
};

struct MESSAGE_COUNT{
	U32			severity;
	U32			count;
};


#endif // __LOG_META_DATA_H__