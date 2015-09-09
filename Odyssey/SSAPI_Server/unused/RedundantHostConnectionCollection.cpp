//************************************************************************
// FILE:		RedundantHostConnectionCollection.cpp
//
// PURPOSE:		Implements the class used to collection host connections
//				into a collection of redundant/non-redundant host
//				connections TO THE SAME HOST (not a cluster!)
//************************************************************************

#include "RedundantHostConnectionCollection.h"



//************************************************************************
// RedundantHostConnectionCollection:
//
// PURPOSE:		Default constructor
//************************************************************************

RedundantHostConnectionCollection::RedundantHostConnectionCollection( ListenManager *pListenManager )
:HostConnectionCollection( pListenManager, SSAPI_OBJECT_CLASS_TYPE_REDUNDANT_HOST_CONN_COLLECTION ){

}

//************************************************************************
// ~RedundantHostConnectionCollection:
//
// PURPOSE:		The destructor
//************************************************************************

RedundantHostConnectionCollection::~RedundantHostConnectionCollection(){
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
RedundantHostConnectionCollection::BuildYourValueSet(){

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
RedundantHostConnectionCollection::BuildYourselfFromYourValueSet(){

	return HostConnectionCollection::BuildYourselfFromYourValueSet();
}


//************************************************************************
// BuildYourselfFropmPtsRow:
//
// PURPOSE:		Populates data members based on the PTS row
//************************************************************************

void 
RedundantHostConnectionCollection::BuildYourselfFromPtsRow( HostCollectionDescriptorRecord *pRow ){

	HostConnectionCollection::BuildYourselfFromPtsRow(	pRow );
}


//************************************************************************
// WriteYourselfIntoPtsRow:
//
// PURPOSE:		Populates pts row based on the data members
//************************************************************************

void 
RedundantHostConnectionCollection::WriteYourselfIntoPtsRow( HostCollectionDescriptorRecord *pRow ){

	HostConnectionCollection::WriteYourselfIntoPtsRow( pRow );

	pRow->collectionType = REDUNDANT;
}