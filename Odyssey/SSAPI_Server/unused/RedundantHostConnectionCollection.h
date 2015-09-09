//************************************************************************
// FILE:		RedundantHostConnectionCollection.h
//
// PURPOSE:		Defines the class used to collection host connections
//				into a collection of redundant/non-redundant host
//				connections TO THE SAME HOST (not a cluster!)
//************************************************************************

#ifndef __REDUNDANT_HOST_CONN_COLLECTION_H__
#define	__REDUNDANT_HOST_CONN_COLLECTION_H__


#include "HostConnectionCollection.h"
#include "HostCollectionDescriptorTable.h"

#pragma pack(4)

class RedundantHostConnectionCollection : public HostConnectionCollection{

public:

//************************************************************************
// RedundantHostConnectionCollection:
//
// PURPOSE:		Default constructor
//************************************************************************

RedundantHostConnectionCollection( ListenManager *pListenManager );


//************************************************************************
// ~RedundantHostConnectionCollection:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~RedundantHostConnectionCollection();


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
// Assignment operator overloaded
//************************************************************************

const ValueSet& operator=(const ValueSet& obj ){ *(ValueSet *)this = obj; return obj; }


//************************************************************************
// BuildYourselfFropmPtsRow:
//
// PURPOSE:		Populates data members based on the PTS row
//************************************************************************

virtual void BuildYourselfFromPtsRow( HostCollectionDescriptorRecord *pRow );


//************************************************************************
// WriteYourselfIntoPtsRow:
//
// PURPOSE:		Populates pts row based on the data members
//************************************************************************

virtual void WriteYourselfIntoPtsRow( HostCollectionDescriptorRecord *pRow );


//************************************************************************
//************************************************************************

};

#endif	// __REDUNDANT_HOST_CONN_COLLECTION_H__