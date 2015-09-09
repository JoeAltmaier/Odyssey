//************************************************************************
// FILE:		LunMapEntry.cpp
//
// PURPOSE:		Defines the object that is used to represent mapped 
//				LUNs that we [can] export to the outside FC world
//************************************************************************

#include "LunMapEntry.h"
#include "StringResourceManager.h"
#include "LunMapManager.h"
#include "DescriptorCollector.h"
#include "DdmSSAPI.h"
#include "DdmVCMCommands.h"
#include "StsData.h"

// TRACE Facility hook-up
#include "Trace_Index.h"
#ifdef TRACE_INDEX
#undef TRACE_INDEX
#endif
#define TRACE_INDEX TRACE_SSAPI_MANAGERS

//************************************************************************
// LunMapEntry:
//
// PURPOSE:		Default constructor
//************************************************************************

LunMapEntry::LunMapEntry(	ListenManager			*pListenManager, 
							DdmServices				*pParent )
:ManagedObject( pListenManager, SSAPI_OBJECT_CLASS_TYPE_LUN_MAP_ENTRY ) {

	pParentDdmSvs = pParent;
	m_isIniting = false;

	m_state			= SSAPI_OBJECT_STATE_GOOD;
	m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_GOOD;
}


//************************************************************************
// ~LunMapEntry:
//
// PURPOSE:		The destructor
//************************************************************************

LunMapEntry::~LunMapEntry(){

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
LunMapEntry::BuildYourValueSet(){
	
	int			temp;
	
	ManagedObject::BuildYourValueSet();

	temp = m_isVerified? 1 : 0;
	AddInt( temp, SSAPI_LUN_MAP_ENTRY_FID_VERIFIED );

	temp = m_isExported? 1 : 0;
	AddInt( temp, SSAPI_LUN_MAP_ENTRY_FID_EXPORTED );

	temp = m_isCached? 1 : 0;
	AddInt( temp, SSAPI_LUN_MAP_ENTRY_FID_USES_CACHE );
	
	m_idInfo.BuildYourValueSet();
	AddValue( &m_idInfo, SSAPI_LUN_MAP_ENTRY_FID_ID_INFO_OBJECT );
	AddString( &m_name, SSAPI_LUN_MAP_ENTRY_FID_NAME );
	AddString( &m_description, SSAPI_LUN_MAP_ENTRY_FID_DESCRIPTION );
	AddInt( m_protocolType, SSAPI_LUN_MAP_ENTRY_FID_PROTOCOL_TYPE );
	AddGenericValue( (char *)&m_hostConnectionId, sizeof(DesignatorId), SSAPI_LUN_MAP_ENTRY_FID_HC_ID );
	AddGenericValue( (char *)&m_storageElementId, sizeof(DesignatorId), SSAPI_LUN_MAP_ENTRY_FID_STORAGE_ELEMENT_ID );

	StatusReporterInterface::BuildYourValueSet( *this );

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
LunMapEntry::BuildYourselfFromYourValueSet(){

	int			temp;
	ValueSet	*pVs;
	
	ManagedObject::BuildYourselfFromYourValueSet();

	GetInt( SSAPI_LUN_MAP_ENTRY_FID_VERIFIED, &temp );
	m_isVerified = temp? true : false;

	GetInt( SSAPI_LUN_MAP_ENTRY_FID_EXPORTED, &temp );
	m_isExported = temp? true : false;

	GetInt( SSAPI_LUN_MAP_ENTRY_FID_USES_CACHE, &temp );
	m_isCached = temp? true : false;
	
	pVs = (ValueSet *)GetValue( SSAPI_LUN_MAP_ENTRY_FID_ID_INFO_OBJECT );
	if( pVs ){
		((ValueSet &)m_idInfo) = *pVs;
		m_idInfo.BuildYourselfFromYourValueSet();
	}

	GetString( SSAPI_LUN_MAP_ENTRY_FID_NAME, &m_name );
	GetString( SSAPI_LUN_MAP_ENTRY_FID_DESCRIPTION, &m_description );
	GetInt( SSAPI_LUN_MAP_ENTRY_FID_PROTOCOL_TYPE, &temp );
	m_protocolType = (CTProtocolType)temp;

	GetGenericValue( (char *)&m_hostConnectionId, sizeof(DesignatorId), SSAPI_LUN_MAP_ENTRY_FID_HC_ID );
	GetGenericValue( (char *)&m_storageElementId, sizeof(DesignatorId), SSAPI_LUN_MAP_ENTRY_FID_STORAGE_ELEMENT_ID );

	StatusReporterInterface::BuildYourselfFromYourValueSet( *this );

	return true;
}


//************************************************************************
// BuildYourselfFromExportTableRow:
//
// PURPOSE:		Build data members based on the information in the PTS row
//************************************************************************

void 
LunMapEntry::BuildYourselfFromExportTableRow( ExportTableEntry	*pRow ){

	m_id					= DesignatorId( RowId(pRow->rid), SSAPI_OBJECT_CLASS_TYPE_LUN_MAP_ENTRY );
	m_ridUserInfo			= pRow->ridUserInfo;
	m_isCached				= (pRow->attribMask & EXPORT_ENTRY_ATTRIBUTE_CACHED)? true : false;
	
	switch( pRow->ReadyState ){
		case StateConfiguredAndExported:
		case StateConfiguredAndActive:
			m_isExported = true;
			break;

		default:
			m_isExported = false;
			break;
	}
	
	m_isVerified			= (pRow->attribMask & EXPORT_ENTRY_ATTRIBUTE_VERIFIED)? true : false;
	m_protocolType			= pRow->ProtocolType;
	m_nextVd				= pRow->vdNext;
	m_idInfo.SetTargetLUN( pRow->ExportedLUN );
	m_idInfo.SetTargetId( pRow->TargetId );

	// determine the state
	SetYourState(pRow->ReadyState);

	m_ridStatus			= pRow->ridStatusRec;
	m_ridPerformance	= pRow->ridPerformanceRec;
	
	// get storage element id
	if( GetManager() ){
		GetObjectManager( GetManager(), SSAPI_MANAGER_CLASS_TYPE_STORAGE_MANAGER )
			->GetDesignatorIdByRowId( pRow->ridSRC, m_storageElementId );

		// get host connection id
		GetObjectManager(GetManager(), SSAPI_MANAGER_CLASS_TYPE_CONNECTION_MANAGER)
			->GetDesignatorIdByRowId( pRow->ridHC, m_hostConnectionId );
	}
	

}



//************************************************************************
// BuildYourselfFromExportTableRow:
//
// PURPOSE:		Build data members based on the information in the PTS row
//************************************************************************

void 
LunMapEntry::BuildYourselfFromExportTableRow(ExportTableEntry		*pRow,
											 DescriptorCollector	&descriptorCollector,
											 DescriptorCollector	&idInfoCollector,
											 DdmSSAPI				&ddmSsapi){

	ExportTableUserInfoRecord		*pUserInfo;
	StsData							*pIdInfoRow;
	U32								i;
	bool							found;

	BuildYourselfFromExportTableRow( pRow );

	if( descriptorCollector.Get( (RowId &)pRow->ridUserInfo, (void *&)pUserInfo ) ){
		m_ridUserInfo		= pRow->ridUserInfo;
		m_ridName			= pUserInfo->ridName;
		m_ridDescription	= pUserInfo->ridDescription;
	}
	else{
		m_ridUserInfo		= RowId();
		m_ridName			= RowId();
		m_ridDescription	= RowId();
	}

	// get storage element id
	ddmSsapi.GetObjectManager( SSAPI_MANAGER_CLASS_TYPE_STORAGE_MANAGER )
		->GetDesignatorIdByRowId( pRow->ridSRC, m_storageElementId );

	// get host connection id
	ddmSsapi.GetObjectManager(SSAPI_MANAGER_CLASS_TYPE_CONNECTION_MANAGER)
		->GetDesignatorIdByRowId( pRow->ridHC, m_hostConnectionId );

	// find the id info record
	for( i = 0, found = false; ; i++ ){
		if( !idInfoCollector.GetAt( i, (void *&)pIdInfoRow ) )
			break;
		
		if( pIdInfoRow->vdSTS == m_nextVd ){
			BuildYourIdInfo( pIdInfoRow );
			found = true;
			break;
		}
	}
	ASSERT( found != false );
}
	

//************************************************************************
// WriteYourselfIntoExportTableRow:
//
// PURPOSE:		Populates the PTS row based on the data members
//************************************************************************

void 
LunMapEntry::WriteYourselfIntoExportTableRow( ExportTableEntry *pRow ){

	pRow->ridUserInfo		= m_ridUserInfo;
}


//************************************************************************
// ModifyObject:
//
// PURPOSE:		Modifes contents of the object
//
// NOTE:		Must be overridden by objects that can be modified
//************************************************************************

bool 
LunMapEntry::ModifyObject( ValueSet &objectValues, SsapiResponder *pResponder ){

	return ((LunMapManager *)GetManager())->ModifyLunMap( objectValues, pResponder );
}


//************************************************************************
// DeleteObject:
//
// PURPOSE:		Deletes the object from the system
//
// NOTE:		Must be overridden by objects that can be deleted
//************************************************************************

bool 
LunMapEntry::DeleteObject( ValueSet &objectValues, SsapiResponder *pResponder ){

	DesignatorId				id;

	objectValues.GetGenericValue( (char *)&id, sizeof(id), SSAPI_OBJECT_FID_ID );
	
	return ((LunMapManager *)GetManager())->DeleteLunMap( id, pResponder );
}


//************************************************************************
// SetYourState:
//
// PURPOSE:		Determines the objects state
//************************************************************************

void 
LunMapEntry::SetYourState( CTReadyState fcState ){

	switch( fcState ){
		case StateConfiguring:
		case StateConfigured:
		case StateConfiguredNotExported:
		case StateConfiguredAndExporting:
		case StateConfiguredAndExported:
		case StateConfiguredAndActive:

			m_state			= SSAPI_OBJECT_STATE_GOOD; 
			m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_GOOD;
			break;

		case StateQuiesced:
			break;

		case StateInvalid:
			ASSERT(0);
		case StateOffLine:
			m_state			= SSAPI_OBJECT_STATE_DEAD; 
			m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_DEAD;
			break;

		default:
			ASSERT(0);
			break;
	}
}

//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		Called before an object is serialized to allow 
//				all the sub classes to set what their overall state is
//				the state should be put into 'm_state' data member
//************************************************************************

void 
LunMapEntry::ComposeYourOverallState(){
	
}


//************************************************************************
//
//************************************************************************

void
LunMapEntry::SetName( UnicodeString &name ){

	if( name != m_name ){
		m_name = name;
		FireEventObjectModifed();
	}
}


//************************************************************************
//
//************************************************************************

void 
LunMapEntry::SetDescription( UnicodeString &description ){

	if( description != m_description ){
		m_description = description;
		FireEventObjectModifed();
	}
}


//************************************************************************
// PopulateVCMCreateCommand:
//
// PURPOSE:		Populates record to the VCM's Create command
//************************************************************************

void 
LunMapEntry::PopulateVCMCreateCommand( VCCreateCommand &cmd ){

	memset( &cmd, 0, sizeof(cmd) );
	cmd.ridSRCElement	= GetStorageElementId().GetRowId().GetRowID();
	cmd.ridHCDR			= GetHostConnectionId().GetRowId().GetRowID();
	cmd.ridUserInfo		= m_ridUserInfo.GetRowID();
	cmd.TargetID		= GetTargetId();
	cmd.LUN				= GetTargetLun();
	cmd.fExport			= GetIsExported();
}


//************************************************************************
// WriteYourselfIntoPtsRow:
//
// PURPOSE:		Populates ExportEntry. Provided for debugging and testing
//************************************************************************

void 
LunMapEntry::WriteYourselfIntoPtsRow( ExportTableEntry &row ){
	
	memset( &row, 0, sizeof(row) );

	row.version			= EXPORT_TABLE_VERSION;
	row.size			= sizeof(row);
	row.ExportedLUN		= m_idInfo.GetTargetLUN();
	row.ProtocolType	= ProtocolFibreChannel;
	row.ridHC			= m_hostConnectionId.GetRowId().GetRowID();
	row.ridSRC			= m_storageElementId.GetRowId().GetRowID();
	row.TargetId		= m_idInfo.GetTargetId();
	row.ReadyState		= m_isExported?StateConfiguredAndActive:StateConfiguredNotExported;
	row.attribMask		= m_isCached? EXPORT_ENTRY_ATTRIBUTE_CACHED : 0;
}


//************************************************************************
// SetIsExported:
//
// PURPOSE:		Changes the 'm_isExported' attrib and posts the event
//************************************************************************

void
LunMapEntry::SetIsExported( bool isExported ){
	
	if( m_isExported != isExported ){
		m_isExported = isExported;
		FireEventObjectModifed();
	}
}


//************************************************************************
// SetIsQuiesced:
//
// PURPOSE:		Makes sure the state of entry is properly reflected.
//				Posts an event if requested
//************************************************************************

void
LunMapEntry::SetIsQuiesced( bool isQuiesced, bool postEvent ){

	if( isQuiesced ){
		if( m_state == SSAPI_OBJECT_STATE_GOOD ){
			m_state			= SSAPI_OBJECT_STATE_WARNING;
			m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_QUIESCED;
			if( postEvent )
				FireEventObjectModifed();
		}
	}
	else{
		m_state			= SSAPI_OBJECT_STATE_GOOD;
		m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_GOOD;
		if( postEvent )
			FireEventObjectModifed();
	}
}


//************************************************************************
// BuildYourIdInfo:
//
// PURPOSE:		Populates Lun id info based on the PTS record
//************************************************************************

void 
LunMapEntry::BuildYourIdInfo( StsData *pRow, bool postEvent ){

	INQUIRY			buff;	// for 0-termination
	UnicodeString	s;

	memset( &buff, 0, sizeof(buff) );
	memcpy( &buff, pRow->InqData.VendorId, sizeof(pRow->InqData.VendorId) );
	s = StringClass( (char *)&buff );
	m_idInfo.SetVendor( s );

	memset( &buff, 0, sizeof(buff) );
	memcpy( &buff, pRow->InqData.ProductId, sizeof(pRow->InqData.ProductId) );
	s = StringClass( (char *)&buff );
	m_idInfo.SetProduct( s );

	memset( &buff, 0, sizeof(buff) );
	memcpy( &buff, pRow->InqData.ProductRevision, sizeof( pRow->InqData.ProductRevision) );
	s = StringClass( (char *)&buff );
	m_idInfo.SetRevision( s );

	if( postEvent )
		FireEventObjectModifed();
}