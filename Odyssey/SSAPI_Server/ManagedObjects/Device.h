//************************************************************************
// FILE:		Device.h
//
// PURPOSE:		Declares an abstract class Device that will serve as a 
//				base class for all device objects in the O2K
//************************************************************************

#ifndef __DEVICE_MO_H__
#define	__DEVICE_MO_H__

#include "ManagedObject.h"
#include "UnicodeString.h"
#include "SSAPITypes.h"
#include "SSAPIObjectStates.h"
#include "StatusReporterInterface.h"
#include "SList.h"

class Device :  public ManagedObject, public StatusReporterInterface {

	

protected:
	bool				m_isLocked;
	bool				m_isPowered;
	int					m_serviceState;			// 1 - in service, 0 - out of service
	LocalizedString		m_name;
	SList				m_microControllerVector;	// contains ptrs to MicroController objects
	int					m_location;

//************************************************************************
// Device:
//
// PURPOSE:			Default constructor
//************************************************************************

Device( ListenManager *pListenManager, U32 objectClassType );



public:


//************************************************************************
// ~Device:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~Device();


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
// BuildYourselfFromPtsRow:
//
// PURPOSE:		REsponsible for reading an object's datamember from a PTS
//				row into crresponding data members
//************************************************************************

virtual bool BuildYourselfFromPtsRow( void *pRow ) = 0;


//************************************************************************
// HandleObjectAddedEvent:
//
// PURPOSE:		Called by the manager to inform the object that a new 
//				object has been added to the system. The object may be
//				interested to know if this new object is its child/parent
//				and update its vectors as needed.
//************************************************************************

void HandlePhsObjectAddedEvent( DesignatorId id, bool postEvent = true );


//************************************************************************
// HandleObjectAddedEvent:
//
// PURPOSE:		Called by the manager to inform the object that a new 
//				object has been added to the system. The object may be
//				interested to know if this new object is its child/parent
//				and update its vectors as needed.
//************************************************************************

virtual void HandleObjectAddedEvent( ValueSet *pObj, bool postEvent = true ) = 0;



//************************************************************************
// IsYourDevice:
//
// PURPOSE:		Determines if a gven device belongs to this device
//				in the logical hierachy. 
//
// RETURN:		true:		yes
//				false:		no
//************************************************************************

virtual bool IsYourDevice( Device &device ) = 0;


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

virtual bool ApplyNecessaryStatusRollupRules();


//************************************************************************
// HasYouPhsVectorChanged:
//
// PURPOSE:		Determines if the PHS id vector is to be rebuilt
//************************************************************************

virtual bool HasYouPhsVectorChanged( Device *pOldDevice ) { return false; }


//******************************************************************************
// GetPhsDataRowId:
//
// PURPOSE:		Returns a row id of the PHS data objects that apply to the element
//
// NOTE:		The caller must de-allocate the pointer in the container
//				(instances of RowId);
//******************************************************************************

virtual void GetPhsDataRowId( Container &container ){
	container.Add( (CONTAINER_ELEMENT) new RowId(m_id.GetRowId()) );
}


//************************************************************************
// Lock:
//
// PURPOSE:		Declares an API method 
//************************************************************************

virtual bool Lock( SsapiResponder *pResponder ){ return false; }


//************************************************************************
// UnLock:
//
// PURPOSE:		Declares an API method
//************************************************************************

virtual bool UnLock( SsapiResponder *pResponder ) {return false; }


//************************************************************************
// SetServiceState:
//
// PURPOSE:		Declares an API method 
//************************************************************************

virtual bool SetServiceState(  SsapiResponder *pResponder, U32 newState ){ return false; }


//************************************************************************
// PowerOn:
//
// PURPOSE:		Declares an API method 
//************************************************************************

virtual bool PowerOn( SsapiResponder *pResponder ){ return false; }


//************************************************************************
// PowerOff:
//
// PURPOSE:		Declares an API method
//************************************************************************

virtual bool PowerOff( SsapiResponder *pResponder ){ return false; }


//************************************************************************
// IsLocked:
//
// PURPOSE:		An accessor
//************************************************************************

bool IsLocked() const { return m_isLocked; }
void SetIsLocked( bool s ) { m_isLocked = s; }


//************************************************************************
// IsPowered:
//
// PURPOSE:		An accessor
//************************************************************************

virtual bool IsPowered(){ return true; }



//************************************************************************
// GetServiceState:
//
// PURPOSE:		Declares an API method
//************************************************************************

U32 GetServiceState() const { return m_serviceState; }

protected:


//************************************************************************
// IsThisClassTypeInThisArray:
//
// PURPOSE:		Determines if the given class type is in the array
//				of class types specified
//************************************************************************

bool IsThisClassTypeInThisArray( U32 classType, const U32 array[], U32 arraySize );


//************************************************************************
// RemoveMicroControllers:		
//
// PURPOSE:		Removes all MicroControllers and frees up memory.
//************************************************************************

void RemoveMicroControllers();


};

#endif	// __DEVICE_MO_H__