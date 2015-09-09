//************************************************************************
// FILE:		SoftwareDescriptor.h
//
// PURPOSE:		Implements the object whose instances will be used to 
//				represent descriptors of the images contained inside
//				an O2K box.
//************************************************************************

#include "SoftwareDescriptor.h"
#include "ImageIterator.h"


//************************************************************************
// SoftwareDescriptor:
//
// PURPOSE:		Default constructor
//************************************************************************

SoftwareDescriptor::SoftwareDescriptor( ListenManager *pLM, ObjectManager *pManager, U32 classType )
:ManagedObject( pLM, classType, pManager ){

	ASSERT( pManager );
}


//************************************************************************
// ~SoftwareDescriptor:
//
// PURPOSE:		The destructor
//************************************************************************

SoftwareDescriptor::~SoftwareDescriptor(){
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
SoftwareDescriptor::BuildYourValueSet(){

	ManagedObject::BuildYourValueSet();

	AddU32( m_majorVersion, SSAPI_IOP_SW_IMAGE_DESCRIPTOR_FID_MAJOR_VERSION );
	AddU32( m_minorVersion, SSAPI_IOP_SW_IMAGE_DESCRIPTOR_FID_MINOR_VERSION );
	AddU32( m_referenceCount, SSAPI_IOP_SW_IMAGE_DESCRIPTOR_FID_REFERENCE_COUNT );
	AddInt64( m_loadedOn, SSAPI_IOP_SW_IMAGE_DESCRIPTOR_FID_LOADED_ON );
	AddInt( m_isDefault? 1 : 0, SSAPI_IOP_SW_IMAGE_DESCRIPTOR_FID_IS_DEFAULT );

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
SoftwareDescriptor::BuildYourselfFromYourValueSet(){

	int temp;

	ManagedObject::BuildYourselfFromYourValueSet();

	GetU32( SSAPI_IOP_SW_IMAGE_DESCRIPTOR_FID_MAJOR_VERSION, &m_majorVersion );
	GetU32( SSAPI_IOP_SW_IMAGE_DESCRIPTOR_FID_MINOR_VERSION, &m_minorVersion);
	GetU32( SSAPI_IOP_SW_IMAGE_DESCRIPTOR_FID_REFERENCE_COUNT, &m_referenceCount );
	GetInt64( SSAPI_IOP_SW_IMAGE_DESCRIPTOR_FID_LOADED_ON, &m_loadedOn );
	GetInt( SSAPI_IOP_SW_IMAGE_DESCRIPTOR_FID_IS_DEFAULT, &temp );
	m_isDefault = temp? true : false;

	return true;
}


//************************************************************************
// BuildYourselfFromImageObject:
//
// PURPOSE:		Populates data members based on the info in the Image
//				object given
//************************************************************************

void 
SoftwareDescriptor::BuildYourselfFromImageObject( Image &image ){

	m_majorVersion		= image.GetMajorVersion();
	m_minorVersion		= image.GetMinorVersion();
	m_referenceCount	= image.GetIopCount();
	m_isDefault			= image.IsDefault()? true : false;
	image.GetTimeCreated( &m_loadedOn );
	m_id				= DesignatorId( (RowId &)image.GetKey(), GetClassType() );
}


//************************************************************************
// SetIsDefault:
//
//************************************************************************

void 
SoftwareDescriptor::SetIsDefault( bool isDefault, bool postEvent ){

	if( m_isDefault != isDefault ) {
		m_isDefault = isDefault;
		if( postEvent )
			FireEventObjectModifed();
	}
}