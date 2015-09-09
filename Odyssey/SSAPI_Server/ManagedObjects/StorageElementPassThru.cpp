//************************************************************************
// FILE:		StorageElementPassThru.cpp
//
// PURPOSE:		Defines an abstract base class for all PASS_THROUGH type
//				of devices in the O2K
//************************************************************************

#include "StorageElementPassThru.h"
#include "StorageIdInfo.h"
#include "DeviceManager.h"
#include "StorageManager.h"
#include "DeviceDescriptor.h"
#include "PathDescriptor.h"


//******************************************************************************
// PopulateYourIdInfo:
//
// PURPOSE:		Requests that the element populates it id info object
//
// RETURN:		true:	the element has the id info and has populated it
//				false:	the element has no id info
//******************************************************************************

bool 
StorageElementPassThru::PopulateYourIdInfo( StorageIdInfo& idInfo ) {

	INQUIRY			buff;	// for 0-termination
	UnicodeString	s;

	idInfo.SetSerialNumber( m_serialNumber );
	idInfo.SetTargetId( m_targetId );
	idInfo.SetTargetLUN( m_targetLUN );

	memset( &buff, 0, sizeof(buff) );
	memcpy( &buff, m_inquiry.VendorId, sizeof(m_inquiry.VendorId) );
	s = StringClass( (char *)&buff );
	idInfo.SetVendor( s );

	memset( &buff, 0, sizeof(buff) );
	memcpy( &buff, m_inquiry.ProductId, sizeof(m_inquiry.ProductId) );
	s = StringClass( (char *)&buff );
	idInfo.SetProduct( s );

	memset( &buff, 0, sizeof(buff) );
	memcpy( &buff, m_inquiry.ProductRevision, sizeof( m_inquiry.ProductRevision) );
	s = StringClass( (char *)&buff );
	idInfo.SetRevision( s );

	return true;
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
//******************************************************************************

void 
StorageElementPassThru::ReportYourUnderlyingDevices( Container &devices ){

	DesignatorId	id;
	DeviceManager	*pDManager = (DeviceManager *)GetObjectManager( GetManager(), SSAPI_MANAGER_CLASS_TYPE_DEVICE_MANAGER );
	

	id = pDManager->GetPortByInstanceNumber( m_instanceNumber );
	if( id == DesignatorId() ){
		ASSERT( 0 );
		return;
	}
	devices.Add( (CONTAINER_ELEMENT) new DesignatorId( id ) );
}


//************************************************************************
// GetCoreTableMask:
//
// PURPOSE:		The methods returns the integer mask of the core table
//				withing the StorageManager. The core table for a storage 
//				element is the table which defines the very existance
//				of the storage element.
//				Before, we only had one such table -- the StorageRollCall
//				table. Now, there is another one - the DeviceDescriptor.
//************************************************************************

U32 
StorageElementPassThru::GetCoreTableMask( ){

	return SSAPI_SM_DEVICE_TABLE;
}


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		Populates data members based on the contents of the PTS 
//				row supplied.
//************************************************************************

void 
StorageElementPassThru::BuildYourselfFromPtsRow( DeviceDescriptor *pRow ){

	char	*p = new char[ sizeof(pRow->SerialNumber) + 1 ];

	m_id			= DesignatorId( pRow->rid, GetClassType() );
	m_isUsed		= pRow->fUsed? true : false;
	m_capacity		= 0;
	m_ridName		= pRow->ridName;
	m_ridDescriptor	= pRow->rid;

	m_internalState	= pRow->CurrentStatus;
	memcpy( &m_inquiry, &pRow->InqData, sizeof(INQUIRY) );

	memset( p, 0, sizeof(pRow->SerialNumber) + 1 );
	memcpy( p, pRow->SerialNumber, sizeof(pRow->SerialNumber) );
	m_serialNumber = StringClass( p );

	delete p;
}


//************************************************************************
// BuildyourselfFromPathRow:
//
// PURPOSE:		Populates some of data members. Filters out inactive
//				paths, only taking data from the sactive one (one is in 
//				1st release, after that --- could be more )
//************************************************************************

void 
StorageElementPassThru::BuildYourselfFromPathRow( PathDescriptor *pPath ){

	// only look at active paths
	if( m_ridDescriptor != pPath->ridActiveDesc )
		return;

	m_instanceNumber= pPath->FCInstance;
	m_targetId		= pPath->FCTargetID;
	m_targetLUN		= pPath->FCTargetLUN;
}
