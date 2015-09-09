//************************************************************************
// FILE:		ClusterHostConnectionCollection.h
//
// PURPOSE:		Defines the class used to collection host connections
//				into a collection of host connections belonging to several
//				hosts (a cluster!)
//************************************************************************

#ifndef __CLUSTER_HOST_CONN_COLLECTION_H__
#define	__CLUSTER_HOST_CONN_COLLECTION_H__


#include "HostConnectionCollection.h"
#include "HostCollectionDescriptorTable.h"

#pragma pack(4)

class ClusterHostConnectionCollection : public HostConnectionCollection{

public:

//************************************************************************
// ClusterHostConnectionCollection:
//
// PURPOSE:		Default constructor
//************************************************************************

ClusterHostConnectionCollection( ListenManager *pListenManager );


//************************************************************************
// ~ClusterHostConnectionCollection:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~ClusterHostConnectionCollection();


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
// operator=:
//
//************************************************************************

virtual const ValueSet& operator=(const ValueSet& obj ){ return *((ValueSet *)this) = obj; }


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

#endif	// __CLUSTER_HOST_CONN_COLLECTION_H__