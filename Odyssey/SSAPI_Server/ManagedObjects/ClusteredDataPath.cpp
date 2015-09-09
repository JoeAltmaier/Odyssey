//************************************************************************
// FILE:		ClusteredDataPath.cpp
//
// PURPOSE:		Implements the object used to represent clustered data paths
//				from a server cluster to or NACs
//************************************************************************

#include "ClusteredDataPath.h"
#include "HostManager.h"
#include "DeviceManager.h"
#include "ConnectionManager.h"
#include "Connection.h"

#define CLEAN_UP_CONTAINERS(a) 	FreeMemoryForTheContainerWithIds( hosts );	\
								FreeMemoryForTheContainerWithIds( connections ); \
								errorString = a;

//************************************************************************
// ClusteredDataPath:
//
// PURPOSE:		Default constructor
//************************************************************************

ClusteredDataPath::ClusteredDataPath( ListenManager *pLM, ObjectManager *pManager )
:DataPath( pLM, SSAPI_OBJECT_CLASS_TYPE_CLUSTERED_DATA_PATH, pManager ){
}


//************************************************************************
// ~ClusteredDataPath();
//
// PURPOSE:		The destructor.
//************************************************************************

ClusteredDataPath::~ClusteredDataPath(){
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
ClusteredDataPath::BuildYourValueSet(){

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
ClusteredDataPath::BuildYourselfFromYourValueSet(){

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
//
// FUNCTIONALITY:
//				All Connections from each host must map to same 
//				primary/fail-over pair
//				The algorithm: 
//				1.	Create a copy of the children vector (name it connection vector)
//				2.	Build the vector of Host ids which is of the same size as connection
//					vector, and Ith id in one vector is for Ith id in the other
//				3.	Go thru the host vector vector, finding all connections that belong 
//					to the same host. Check each pair if they are on partnered NACs.
//************************************************************************

bool 
ClusteredDataPath::IsAValidConfiguration( U32 &errorString ){
	
	CoolVector			connections, hosts, positions;
	DesignatorId		*pId, connId, hostId, port1, port2;
	U32					i, hostNum;
	HostManager			*pHM = (HostManager *)GetObjectManager( GetManager(), SSAPI_MANAGER_CLASS_TYPE_HOST_MANAGER );
	DeviceManager		*pDM = (DeviceManager *)GetObjectManager( GetManager(), SSAPI_MANAGER_CLASS_TYPE_DEVICE_MANAGER );
	ConnectionManager	*pCM = (ConnectionManager *)GetManager();

	if( !GetChildCount() ){
		errorString = CTS_SSAPI_CM_CONN_MUST_BE_PRESENT;
		return false;
	}

	//1 && 2
	for( i = 0; i < GetChildCount(); i++ ){
		connId = GetChildIdAt( i );
		if( !pHM->GetHostIdByConnectionId( connId, hostId ) ){
			CLEAN_UP_CONTAINERS(CTS_SSAPI_CM_CONN_NOT_ASSIGNED_TO_HOST)
			return false;
		}
		connections.AddAt( (CONTAINER_ELEMENT) new DesignatorId( connId ), i );
		hosts.AddAt( (CONTAINER_ELEMENT) new DesignatorId( hostId ), i );
	}

	// 3
	for( positions.RemoveAll() ; hosts.Count(); positions.RemoveAll() ){
		// get the host id to look for
		positions.Add( 0 );
		hosts.GetAt( (CONTAINER_ELEMENT &)pId, 0 );
		hostId = *pId;

		// find positions of all host ids that are == to the one at position 0
		for( hostNum = 1; hostNum < hosts.Count(); hostNum++ ){
			hosts.GetAt( (CONTAINER_ELEMENT &)pId, hostNum );
			if( *pId == hostId )
				positions.AddAt( hostNum, positions.Count() );
		}

		// now use the positions to check if the connections are 
		// on the partnered NACs
		if( positions.Count() == 2 ){
			connections.GetAt( (CONTAINER_ELEMENT &)pId, 0 );
			port1 = ((ConnectionBase *)pCM->GetManagedObject( pId ))->GetGeminiPortId();
			positions.GetAt( i, 1 );
			connections.GetAt( (CONTAINER_ELEMENT &)pId, i );
			port2 = ((ConnectionBase *)pCM->GetManagedObject( pId ))->GetGeminiPortId();

			if( !pDM->AreThesePortsOnPartneredNacs( port1, port2 ) ){
				CLEAN_UP_CONTAINERS(CTS_SSAPI_CM_CONN_FROM_HOST_MUST_BE_ON_PARTNER_NACS);
				return false;
			}
		}
		else if( positions.Count() > 2 ){
			CLEAN_UP_CONTAINERS( CTS_SSAPI_CM_ONLY2_CONN_PER_HOST );
			return false;
		}

		// OK, connections for this host are fine, so purge 'em from the 
		// corresponding vectors
		while( positions.Count() ){
			positions.GetAt( i, 0 );
			positions.RemoveAt(0);

			hosts.GetAt( (CONTAINER_ELEMENT &)pId, i );
			delete pId;
			hosts.RemoveAt( i );

			connections.GetAt( (CONTAINER_ELEMENT &)pId, i );
			delete pId;
			connections.RemoveAt( i );
		}
	}

	// if we made here -> all is cool and all vectors have been freed
	return true;
}
