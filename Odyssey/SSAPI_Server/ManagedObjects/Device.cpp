//************************************************************************
// FILE:		Device.cpp
//
// PURPOSE:		Implements an abstract class Device that will serve as a 
//				base class for all device objects in the O2K
//************************************************************************

#include "Device.h"
#include "MicroController.h"


//************************************************************************
// Device:
//
// PURPOSE:			Default constructor
//************************************************************************

Device::Device( ListenManager *pListenManager, U32 objectClassType )
	  :ManagedObject( pListenManager, objectClassType ){
	
	m_manager	= DesignatorId( RowId(), SSAPI_MANAGER_CLASS_TYPE_DEVICE_MANAGER );
	m_isLocked	= true; 
	m_isPowered = true;
	m_serviceState = 1;
}


//************************************************************************
// ~Device:
//
// PURPOSE:		The destructor
//************************************************************************

Device::~Device(){
	
	RemoveMicroControllers();
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
Device::BuildYourValueSet(){

	ValueSet			*pVs = new ValueSet;
	U32					index;
	MicroController		*pMC;

	ManagedObject::BuildYourValueSet();

	ComposeYourOverallState();

	AddInt( m_name,  SSAPI_DEVICE_FID_NAME );
	AddInt( m_location, SSAPI_DEVICE_FID_LOCATION );

	for( index = 0; index < m_microControllerVector.Count(); index++ ){
		m_microControllerVector.GetAt( (CONTAINER_ELEMENT &)pMC, index );
		pMC->BuildYourValueSet();
		pVs->AddValue( pMC, index );
	}
	AddValue( pVs, SSAPI_DEVICE_FID_MICROCONTROLLER_VECTOR );

	StatusReporterInterface::BuildYourValueSet( *this );

	delete pVs;
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
Device::BuildYourselfFromYourValueSet(){

	ValueSet			*pVs, *pMcVs;
	U32					index;
	MicroController		*pMC;

	ManagedObject::BuildYourselfFromYourValueSet();

	GetInt( SSAPI_DEVICE_FID_NAME, (int *)&m_name );
	GetInt( SSAPI_DEVICE_FID_LOCATION, &m_location );

	// remove old micro controllers
	RemoveMicroControllers();

	// get new microcontrollers, if any
	pVs = (ValueSet *)GetValue( SSAPI_DEVICE_FID_MICROCONTROLLER_VECTOR );
	if( pVs )
		for( index = 0; index < pVs->GetCount(); index++ ){
			pMC		= new MicroController();
			pMcVs	= (ValueSet *)pVs->GetValue( index );
			*pMC	= *pMcVs;
			pMC->BuildYourselfFromYourValueSet();
			m_microControllerVector.Add( (CONTAINER_ELEMENT) pMC );
		}
	
	StatusReporterInterface::BuildYourselfFromYourValueSet( *this );

	return true;
}


//************************************************************************
// HandleObjectAddedEvent:
//
// PURPOSE:		Called by the manager to inform the object that a new 
//				object has been added to the system. The object may be
//				interested to know if this new object is its child/parent
//				and update its vectors as needed.
//************************************************************************

void 
Device::HandlePhsObjectAddedEvent( DesignatorId id, bool postEvent ){

	ValueSet		*pVs = new ValueSet;
	
	pVs->AddGenericValue( (char *)&id, sizeof(id), SSAPI_OBJECT_FID_ID);

	HandleObjectAddedEvent( pVs, postEvent );

	delete pVs;
}


//************************************************************************
// IsThisClassTypeInThisArray:
//
// PURPOSE:		Determines if the given class type is in the array
//				of class types specified
//************************************************************************

bool 
Device::IsThisClassTypeInThisArray( U32 classType, const U32 array[], U32 arraySize ){
	
	for( U32 index = 0; index < arraySize; index++ )
		if( array[ index ] == classType )
			return true;

	return false;
}


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

bool 
Device::ApplyNecessaryStatusRollupRules(){

	bool				rc;

	ComposeYourOverallState();

	rc = StatusReporterInterface::ApplyNecessaryStatusRollupRules( this, m_children, m_parents );

	if( rc )
		FireEventObjectModifed();

	return rc;
}


//************************************************************************
// RemoveMicroControllers:		
//
// PURPOSE:		Removes all MicroControllers and frees up memory.
//************************************************************************

void 
Device::RemoveMicroControllers(){

	MicroController		*pMC;

	while( m_microControllerVector.Count() ){
		m_microControllerVector.GetAt( (CONTAINER_ELEMENT &)pMC, 0 );
		m_microControllerVector.RemoveAt( 0 );
		delete pMC;
	}
}