//************************************************************************
// FILE:		RedundantDataPath.cpp
//
// PURPOSE:		Implements class used to represent data paths between
//				a host's HBAs and NACs' ports.
//************************************************************************


#include "DdmSSAPI.h"
#include "ConnectionManager.h"
#include "RedundantDataPath.h"
#include "UpstreamConnection.h"
#include "HostManager.h"
#include "DeviceManager.h"
		
//************************************************************************
// RedundantDataPath:
//
// PURPOSE:		The default constructor
//************************************************************************

RedundantDataPath::RedundantDataPath( ListenManager *pListenManager, ObjectManager *pManager )
:DataPath( pListenManager, SSAPI_OBJECT_CLASS_TYPE_REDUNDANT_DATA_PATH, pManager ){

}


//************************************************************************
// ~RedundantDataPath:
//
// PURPOSE:		The destructor
//************************************************************************

RedundantDataPath::~RedundantDataPath(){
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
RedundantDataPath::BuildYourValueSet(){

	return DataPath::BuildYourValueSet();
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
RedundantDataPath::BuildYourselfFromYourValueSet(){

	return DataPath::BuildYourselfFromYourValueSet();
}


//************************************************************************
// IsAValidConfiguration:
//
// PURPOSE:		Checks the connections assigned to this data path and
//				responds if this is a valid set of connections
//
// OUTPUT:		If the configuration is invalid, 'errorString' will
//				contain the localized string that explains the problem
//************************************************************************

bool 
RedundantDataPath::IsAValidConfiguration( U32 &errorString ){

	// the connection # check
	if( (GetChildCount() > 2) || (GetChildCount() < 1 ) ){
		errorString = CTS_SSAPI_CM_INVALID_CONN_COUNT_FOR_RDP;
		return false;
	}

	// must belong to the same host
	HostManager *pHM = (HostManager *)GetObjectManager( GetManager(), SSAPI_MANAGER_CLASS_TYPE_HOST_MANAGER );
	if( !pHM->DoConnectionsBelongToSameHost( m_children ) ){
		errorString = CTS_SSAPI_CM_CONN_MUST_BELONG_2_SAME_HOST;
		return false;
	}

	// must map to a single primary/fail-over IOP pair
	if( GetChildCount() == 2 ){
		DeviceManager		*pDM = (DeviceManager *)GetObjectManager(GetManager(), SSAPI_MANAGER_CLASS_TYPE_DEVICE_MANAGER );
		ConnectionManager	*pCM = (ConnectionManager *)GetManager();
		DesignatorId		id1 = GetChildIdAt( 0 ), id2 = GetChildIdAt( 1 );

		id1 = ((ConnectionBase *)pCM->GetManagedObject( &id1 ))->GetGeminiPortId();
		id2 = ((ConnectionBase *)pCM->GetManagedObject( &id2 ))->GetGeminiPortId();
		if( !pDM->AreThesePortsOnPartneredNacs( id1, id2 ) ){
			errorString = CTS_SSAPI_CM_CONN_MUST_BE_ON_PARTNER_NACS;
			return false;
		}
	}

	// we made it to here? Gee, it's good then!!!
	return true;
}





