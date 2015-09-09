//************************************************************************
// FILE:		HDDDevice.h
//
// PURPOSE:		Defines class HDDDevice that is used to represent all HDD
//				devices in the O2K
//************************************************************************

#ifndef __HDD_DEVICE_H__
#define	__HDD_DEVICE_H__

#include "Device.h"
#include "UnicodeString.h"
#include "LockableInterface.h"
#include "AssetInterface.h"

#ifdef WIN32
#pragma pack(4)
#endif

struct PathDescriptor;

class HDDDevice : public Device, public LockableInterface{

	int				m_bayNumber;
	int				m_targetId;
	StringClass		m_serialNumber;			// to be transfered as UnicodeString
	int				m_status;
	I64				m_capacity;
	AssetInterface	m_assetInfo;
	RowId			m_ridStatusRecord;		// for internal purposes (to find PHSs)
	RowId			m_ridPerformanceRecord;	// for internal purposes (to find PHSs)


	friend class DeviceManager;
public:


//************************************************************************
// HDDDevice:
//
// PURPOSE:		Default constructor
//************************************************************************

HDDDevice( ListenManager *pListenManager );


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new HDDDevice( GetListenManager() ); }


//************************************************************************
// ~HDDDevice
// 
// PURPOSE:		The destructor
//************************************************************************

virtual ~HDDDevice();


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
// Assignment operator overloaded
//************************************************************************

const ValueSet& operator=(const ValueSet& obj ){ *(ValueSet *)this = obj; return obj; }


//************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		REsponsible for reading an object's datamember from a PTS
//				row into crresponding data members
//************************************************************************

virtual bool BuildYourselfFromPtsRow( void *pRow );


//************************************************************************
// BuildYourselfFromPathRow:
//
// PURPOSE:		Populates some data members based on the contents of the
//				row passed.
//************************************************************************

void BuildYourselfFromPathRow( PathDescriptor *pRow );


//************************************************************************
// IsYourDevice:
//
// PURPOSE:		Determines if a gven device belongs to this device
//				in the logical hierachy. 
//
// RETURN:		true:		yes
//				false:		no
//************************************************************************

virtual bool IsYourDevice( Device &device );


//************************************************************************
// Accessors:
//************************************************************************

int GetTargetId() const { return m_targetId; }
RowId& GetRidStatus() { return m_ridStatusRecord; }
RowId& GetRidPerformance() { return m_ridPerformanceRecord; }

//************************************************************************
// HasYouPhsVectorChanged:
//
// PURPOSE:		Determines if the PHS id vector is to be rebuilt
//************************************************************************

virtual bool HasYouPhsVectorChanged( Device *pOldDevice ) { 

	HDDDevice	*pOld = (HDDDevice *)pOldDevice;

	if( (m_ridStatusRecord != pOld->GetRidStatus() )
		||
		(m_ridPerformanceRecord != pOld->GetRidPerformance() ) )

		return true;

	return false;
}


//******************************************************************************
// GetPhsDataRowId:
//
// PURPOSE:		Returns a row id of the PHS data objects that apply to the element
//
// NOTE:		The caller must de-allocate the pointer in the container
//				(instances of RowId);
//******************************************************************************

virtual void GetPhsDataRowId( Container &container ){
	container.Add( (CONTAINER_ELEMENT) new RowId(m_ridStatusRecord) );
	container.Add( (CONTAINER_ELEMENT) new RowId(m_ridPerformanceRecord) );
}


//************************************************************************
// Lock:
//
// PURPOSE:		Declares an API method 
//************************************************************************

virtual bool Lock( SsapiResponder *pResponder );


//************************************************************************
// UnLock:
//
// PURPOSE:		Declares an API method
//************************************************************************

virtual bool UnLock( SsapiResponder *pResponder );



private:

//************************************************************************
// SetPhsRowIds:
//
// PURPOSE:		Provided for the DeviceManager to set row ids when
//				appropriate.
//
// RETURN:		true:		at least one row id was modified
//				false:		row ids were as requested already
//************************************************************************

bool SetPhsRowIds( RowId &ridStatus, RowId &ridPerformance, bool postEvent = false );


protected:
//************************************************************************
// HandleObjectAddedEvent:
//
// PURPOSE:		Called by the manager to inform the object that a new 
//				object has been added to the system. The object may be
//				interested to know if this new object is its child/parent
//				and update its vectors as needed.
//************************************************************************

virtual void HandleObjectAddedEvent( ValueSet *pObj, bool postEvent = true );


//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		Called before a device object is serialized to allow 
//				all the sub classes to set what their overall state is
//				the state should be put into 'm_state' data member
//************************************************************************

virtual void ComposeYourOverallState();


};

#endif	//__HDD_DEVICE_H__