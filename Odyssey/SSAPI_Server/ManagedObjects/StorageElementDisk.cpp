//************************************************************************
// FILE:		StorageElementDisk.cpp
//
// PURPOSE:		Implements the object whose instances will represent 
//				internal disk storage elements
//************************************************************************


#include "StorageElementDisk.h"
#include "DeviceManager.h"
#include "DdmSSAPI.h"
#include "PathDescriptor.h"
#include "StorageIdInfo.h"


extern void WriteWWNToUnicodeString( char* pWWN, U32 wwnSize, UnicodeString &us );


//************************************************************************
// StorageElementDisk:
//
// PURPOSE:		Default constructor
//************************************************************************

StorageElementDisk::StorageElementDisk( ListenManager *pListenManager, U32 classType, ObjectManager *pManager )
:StorageElement( pListenManager, classType, pManager ){
}


//************************************************************************
// ~StorageElementDisk:
//
// PURPOSE:		Default destructor
//************************************************************************

StorageElementDisk::~StorageElementDisk(){
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
StorageElementDisk::BuildYourValueSet(){

	StorageElement::BuildYourValueSet();

	return true;
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
StorageElementDisk::BuildYourselfFromYourValueSet(){

	StorageElement::BuildYourselfFromYourValueSet();

	return true;
}


//************************************************************************
// BuildYourselfFromPtsRow
// 
// PURPOSE:		Populates data members based on the info in the row
//************************************************************************

void 
StorageElementDisk::BuildYourselfFromPtsRow( DiskDescriptor *pRow ){

	char	*p = new char[ sizeof(pRow->SerialNumber) + 1 ];

	m_status				= pRow->CurrentStatus;
	m_ridDescriptor			= pRow->rid;
	memcpy( &m_inquiry, &pRow->InqData, sizeof(INQUIRY) );

	memset( p, 0, sizeof(pRow->SerialNumber) + 1 );
	memcpy( p, pRow->SerialNumber, sizeof(pRow->SerialNumber) );
	m_serialNumber = StringClass( p );
	m_wwName = StringClass( pRow->WWNName );

	delete p;
}


//************************************************************************
// BuildYourselfFromPathRow:
//
// PURPOSE:		Populates data members from the path descriptor
//************************************************************************

void 
StorageElementDisk::BuildYourselfFromPathRow( PathDescriptor *pPath ){

	// we only need the active paths
	if( m_ridDescriptor != pPath->ridActiveDesc )
		return;

	m_targetId			= pPath->FCTargetID;
	m_targetLUN			= pPath->FCTargetLUN;
	m_instanceNumber	= pPath->FCInstance;
}


//******************************************************************************
// SetYourState:
//
// PURPOSE:		Requires that super classes set their state and state string
//******************************************************************************

void 
StorageElementDisk::SetYourState(){

	switch( m_status ){
		case DriveInvalid:
		case DriveNotSpinning:
		case DriveRemoved:
		case DriveNotPresent:
		case DriveHardFailure:

			m_state		= SSAPI_OBJECT_STATE_DEAD;
			m_stateString = CTS_SSAPI_OBJECT_STATE_NAME_DEAD;
			break;

		case DriveSpinningUp:
			m_state		= SSAPI_OBJECT_STATE_WARNING;
			m_stateString = CTS_SSAPI_OBJECT_STATE_NAME_WARNING;
			break;

		case DriveReady:
			m_state		= SSAPI_OBJECT_STATE_GOOD;
			m_stateString = CTS_SSAPI_OBJECT_STATE_NAME_GOOD;
			break;
	}
}


//******************************************************************************
// PopulateYourIdInfo:
//
// PURPOSE:		Requests that the element populates it id info object
//
// RETURN:		true:	the element has the id info and has populated it
//				false:	the element has no id info
//******************************************************************************

bool 
StorageElementDisk::PopulateYourIdInfo( StorageIdInfo& idInfo ) {

	INQUIRY			buff;	// for 0-termination
	UnicodeString	s;
	char			*pTemp;
	DiskDescriptor	*pDummy = NULL;	// for sizeof()

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

	pTemp = new(tZERO) char[ sizeof(pDummy->WWNName) ];
	m_wwName.CString( pTemp, sizeof(pDummy->WWNName) );
	WriteWWNToUnicodeString( pTemp, sizeof(pDummy->WWNName), s );
	idInfo.SetWWName( s );
	delete [] pTemp;

	return true;
}