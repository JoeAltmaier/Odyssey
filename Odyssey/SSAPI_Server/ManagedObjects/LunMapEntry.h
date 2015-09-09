//************************************************************************
// FILE:		LunMapEntry.h
//
// PURPOSE:		Defines the object that is used to represent mapped 
//				LUNs that we [can] export to the outside FC world
//************************************************************************

#ifndef __LUN_MAP_ENTRY_H__
#define	__LUN_MAP_ENTRY_H__

#include "ManagedObject.h"
#include "ExportTable.h"
#include "ExportTableUserInfoTable.h"
#include "ReadTable.h"
#include "StorageRollCallTable.h"
#include "StatusReporterInterface.h"
#include "HostConnectionDescriptorTable.h"
#include "StorageIdInfo.h"

class DescriptorCollector;
class DdmSSAPI;
class VCCreateCommand;
struct StsData;

#ifdef WIN32
#pragma pack(4)
#endif


class LunMapEntry : public ManagedObject, public StatusReporterInterface {

	bool						m_isCached;
	bool						m_isExported;
	bool						m_isVerified;					// what the hell is this?!
	CTProtocolType				m_protocolType;
	DesignatorId				m_storageElementId;
	DesignatorId				m_hostConnectionId;				// if exported --> to whom
	UnicodeString				m_name;
	UnicodeString				m_description;
	StorageIdInfo				m_idInfo;
	RowId						m_ridUserInfo;					// internal
	RowId						m_ridName;
	RowId						m_ridDescription;
	bool						m_isIniting;					// internal, init only
	RowId						m_ridHandle;					// handle to send to the VCM
	RowId						m_ridStatus;
	RowId						m_ridPerformance;
	VDN							m_nextVd;						// internal

public:

//************************************************************************
// LunMapEntry:
//
// PURPOSE:		Default constructor
//************************************************************************

LunMapEntry( ListenManager *pListenManager, DdmServices* );


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new LunMapEntry( GetListenManager(), GetManager() ); }


//************************************************************************
// ~LunMapEntry:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~LunMapEntry();


//************************************************************************
// operator=:
//
//************************************************************************

virtual const ValueSet& operator=(const ValueSet& obj ){ return *((ValueSet *)this) = obj; }


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
// Accessors:
//************************************************************************

UnicodeString	GetName() const { return m_name; }
UnicodeString	GetDescription() const { return m_description; }
rowID			GetUserInfoRowId() const { return m_ridUserInfo.GetRowID(); }
DesignatorId	GetStorageElementId() const { return m_storageElementId; }
DesignatorId	GetHostConnectionId() const { return m_hostConnectionId; }
U32				GetTargetId() const	{ return m_idInfo.GetTargetId(); }
U32				GetTargetLun() const	{ return m_idInfo.GetTargetLUN(); }
RowId			GetRidName() const { return m_ridName; }
RowId			GetRidDescription() const { return m_ridDescription; }
RowId			GetRidUserInfo() const { return m_ridUserInfo; }
RowId			GetRidHandle() const { return m_ridHandle; }
bool			GetIsExported() const { return m_isExported; }
RowId&			GetRidPerformance() { return m_ridPerformance; }
RowId&			GetRidStatus() { return m_ridStatus; }
VDN				GetNextVd() const { return m_nextVd; }

void			SetRidName( RowId &rid ) { m_ridName = rid; }
void			SetRidDescription( RowId &rid ) { m_ridDescription = rid; }
void			SetRidUserInfo( RowId &rid ) { m_ridUserInfo = rid; }
void			SetName( UnicodeString &name );
void			SetDescription( UnicodeString &description );
void			SetIsExported( bool isExported );
void			SetRidHandle( RowId ridHandle ) { m_ridHandle = ridHandle; }
void			SetIsQuiesced( bool isQuiesced, bool postEvent = true );


//************************************************************************
// BuildYourselfFromExportTableRow:
//
// PURPOSE:		Build data members based on the information in the PTS row
//************************************************************************

void BuildYourselfFromExportTableRow( ExportTableEntry	*pRow );


//************************************************************************
// BuildYourselfFromExportTableRow:
//
// PURPOSE:		Build data members based on the information in the PTS row
//************************************************************************

void BuildYourselfFromExportTableRow(ExportTableEntry		*pRow,
									 DescriptorCollector	&descriptorCollector,
									 DescriptorCollector	&idInfoCollector,
									 DdmSSAPI				&ddmSsapi);


//************************************************************************
// BuildYourIdInfo:
//
// PURPOSE:		Populates Lun id info based on the PTS record
//************************************************************************

void BuildYourIdInfo( StsData *pRow, bool postEvent = false );


//************************************************************************
// WriteYourselfIntoExportTableRow:
//
// PURPOSE:		Populates the PTS row based on the data members
//************************************************************************

void WriteYourselfIntoExportTableRow( ExportTableEntry *pRow );


//************************************************************************
// ModifyObject:
//
// PURPOSE:		Modifes contents of the object
//
// NOTE:		Must be overridden by objects that can be modified
//************************************************************************

virtual bool ModifyObject( ValueSet &objectValues, SsapiResponder *pResponder );


//************************************************************************
// DeleteObject:
//
// PURPOSE:		Deletes the object from the system
//
// NOTE:		Must be overridden by objects that can be deleted
//************************************************************************

virtual bool DeleteObject( ValueSet &objectValues, SsapiResponder *pResponder );


//************************************************************************
// SetYourState:
//
// PURPOSE:		Determines the objects state
//************************************************************************

void SetYourState( CTReadyState fcState );


//************************************************************************
// ApplyNecessaryStatusRollupRules:
//
// PURPOSE:		Applies applicabale rollup rules.
//				The object that implements this method is expected to call
//				the protected method with the same name and specify 
//				necessary parms.
//
// RETURN:		true:			object's state's been changed
//				false:			object's state's remained unchanged
//************************************************************************

virtual bool ApplyNecessaryStatusRollupRules(){ return false; }


//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		Called before an object is serialized to allow 
//				all the sub classes to set what their overall state is
//				the state should be put into 'm_state' data member
//************************************************************************

virtual void ComposeYourOverallState();


//************************************************************************
// PopulateVCMCreateCommand:
//
// PURPOSE:		Populates record to the VCM's Create command
//************************************************************************

void PopulateVCMCreateCommand( VCCreateCommand &cmd );


//************************************************************************
// WriteYourselfIntoPtsRow:
//
// PURPOSE:		Populates ExportEntry. Provided for debugging and testing
//************************************************************************

void WriteYourselfIntoPtsRow( ExportTableEntry &row );


//******************************************************************************
// GetPhsDataRowId:
//
// PURPOSE:		Returns a row id of the PHS data objects that apply to the element
//
// NOTE:		The caller must de-allocate the pointer in the container
//				(instances of RowId);
//******************************************************************************

void GetPhsDataRowId( Container &container ){
	container.Add( (CONTAINER_ELEMENT) new RowId(m_ridStatus) );
	container.Add( (CONTAINER_ELEMENT) new RowId(m_ridPerformance) );
}

};



#endif // __LUN_MAP_ENTRY_H__