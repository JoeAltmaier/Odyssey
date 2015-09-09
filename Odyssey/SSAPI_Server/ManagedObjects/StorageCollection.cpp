//************************************************************************
// FILE:		StorageCollection.cpp
//
// PURPOSE:		Implements the object whose instance will aggregate 
//				storage element based on some logical similarity.
//************************************************************************


#include "StorageCollection.h"


//************************************************************************
// StorageCollection:
//
// PURPOSE:		Default constructor
//************************************************************************

StorageCollection::StorageCollection( ListenManager *pListenManager, U32 objectClassType, ObjectManager *pManager )
:StorageElementBase( pListenManager, objectClassType, pManager ){
}


//************************************************************************
// ~StorageCollection:
//
// PURPOSE:		Default destructor
//************************************************************************

StorageCollection::~StorageCollection(){
}



//******************************************************************************
// SetYourState:
//
// PURPOSE:		Requires that super classes set their state and state string
//******************************************************************************

void 
StorageCollection::SetYourState(){

	int						state, savedState = m_state;
	StorageElementBase		*pStorageElementBase;
	
	if( StatusReporterInterface::CanChangeStateToGood() ){
		m_state = SSAPI_OBJECT_STATE_GOOD;
		m_stateString = CTS_SSAPI_OBJECT_STATE_NAME_GOOD;

		for( U32 i = 0; i < m_children.Count(); i++ ){
			pStorageElementBase	= (StorageElementBase *)GetChild( i );
			if( (state = pStorageElementBase->GetState() ) > m_state ){
				m_stateString = pStorageElementBase->GetStateString();
				m_state = state;
			}
		}

		if( m_state != savedState )
			FireEventObjectModifed();
	}
}


//******************************************************************************
// ReportYourUnderlyingDevices:
//
// PURPOSE:		Gives a chance to every Storage Element to report devices that
//				make it up.
//
// FUNTIONALITY: Derived classes must populate the specified vector with IDs of
//				devices that they contain of. Memory allocated will be freed
//				by the caller, so derived classes must not deallocate anything.
//
//				Basically, simply go thru the child list and collect what they
//				report into a list.
//******************************************************************************

void  
StorageCollection::ReportYourUnderlyingDevices( Container &devices ){

	CoolVector			tempContainer;
	StorageElementBase	*pElement;
	U32					index;
	DesignatorId		*pId;

	devices.RemoveAll();

	for( index = 0 ; index < GetChildCount(); index++ ){

		// request a list for every child
		pElement = (StorageElementBase *)GetChild( index );
		pElement->ReportYourUnderlyingDevices( tempContainer );

		// move ids into the main vector
		while( tempContainer.Count() ){
			tempContainer.GetAt( (CONTAINER_ELEMENT &)pId, 0 );
			tempContainer.RemoveAt( 0 );
			devices.Add( (CONTAINER_ELEMENT) pId );
		}
	}
}
