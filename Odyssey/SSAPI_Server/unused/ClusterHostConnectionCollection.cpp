//************************************************************************
// FILE:		ClusterHostConnectionCollection.cpp
//
// PURPOSE:		Implements the class used to collection host connections
//				into a collection of redundant/non-redundant host
//				connections TO THE SAME HOST (not a cluster!)
//************************************************************************

#include "ClusterHostConnectionCollection.h"


//************************************************************************
// ClusterHostConnectionCollection:
//
// PURPOSE:		Default constructor
//************************************************************************

ClusterHostConnectionCollection::ClusterHostConnectionCollection( ListenManager *pListenManager )
:HostConnectionCollection( pListenManager, SSAPI_OBJECT_CLASS_TYPE_CLUSTER_HOST_CONN_COLLECTION ){

}

//************************************************************************
// ~ClusterHostConnectionCollection:
//
// PURPOSE:		The destructor
//************************************************************************

ClusterHostConnectionCollection::~ClusterHostConnectionCollection(){
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
ClusterHostConnectionCollection::BuildYourValueSet(){

	return HostConnectionCollection::BuildYourValueSet();
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
ClusterHostConnectionCollection::BuildYourselfFromYourValueSet(){

	return HostConnectionCollection::BuildYourselfFromYourValueSet();
}


//************************************************************************
// BuildYourselfFropmPtsRow:
//
// PURPOSE:		Populates data members based on the PTS row
//************************************************************************

void 
ClusterHostConnectionCollection::BuildYourselfFromPtsRow( HostCollectionDescriptorRecord *pRow ){

	HostConnectionCollection::BuildYourselfFromPtsRow(	pRow );
}


//************************************************************************
// WriteYourselfIntoPtsRow:
//
// PURPOSE:		Populates pts row based on the data members
//************************************************************************

void 
ClusterHostConnectionCollection::WriteYourselfIntoPtsRow( HostCollectionDescriptorRecord *pRow ){

	HostConnectionCollection::WriteYourselfIntoPtsRow( pRow );

	pRow->collectionType = CLUSTER;
}