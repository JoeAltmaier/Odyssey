//************************************************************************
// FILE:		SoftwareImage.cpp
//
// PURPOSE:		Implements the object whose instances will represent
//				software images associated with devices
//************************************************************************

#include "SoftwareImage.h"
#include "ImageIterator.h"

//************************************************************************
// SoftwareImage:
//

//************************************************************************

SoftwareImage::SoftwareImage( ListenManager *pLManager, ObjectManager *pManager )
:ManagedObject( pLManager, SSAPI_OBJECT_CLASS_TYPE_SOFTWARE_IMAGE, pManager ){
}


//************************************************************************
//
//************************************************************************

SoftwareImage::~SoftwareImage(){
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
SoftwareImage::BuildYourValueSet(){
	
	ManagedObject::BuildYourValueSet();
	
	AddGenericValue( (char *)&m_descriptorId, sizeof(m_descriptorId), SSAPI_SOFTWARE_IMAGE_FID_DESCRIPTOR_OBJECT_ID );
	AddGenericValue( (char *)&m_deviceId, sizeof(m_deviceId), SSAPI_SOFTWARE_IMAGE_FID_DEVICE_OBJECT_ID );
	AddInt64( m_lastBoot, SSAPI_SOFTWARE_IMAGE_FID_LAST_BOOTED_TIMESTAMP );
	AddInt( m_isAccepted? 1 : 0, SSAPI_SOFTWARE_IMAGE_FID_IS_ACCEPTED );
	AddInt( m_isCurrent? 1 : 0, SSAPI_SOFTWARE_IMAGE_FID_IS_CURRENT );
	AddInt( m_isPrimary? 1 : 0, SSAPI_SOFTWARE_IMAGE_FID_IS_PRIMARY );

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
SoftwareImage::BuildYourselfFromYourValueSet(){
	
	int				temp;

	ManagedObject::BuildYourselfFromYourValueSet();

	GetGenericValue( (char *)&m_descriptorId, sizeof(m_descriptorId), SSAPI_SOFTWARE_IMAGE_FID_DESCRIPTOR_OBJECT_ID );
	GetGenericValue( (char *)&m_deviceId, sizeof(m_deviceId), SSAPI_SOFTWARE_IMAGE_FID_DEVICE_OBJECT_ID );
	GetInt64( SSAPI_SOFTWARE_IMAGE_FID_LAST_BOOTED_TIMESTAMP, &m_lastBoot );

	GetInt( SSAPI_SOFTWARE_IMAGE_FID_IS_ACCEPTED, &temp );
	m_isAccepted = temp? true : false;

	GetInt( SSAPI_SOFTWARE_IMAGE_FID_IS_CURRENT, &temp );
	m_isCurrent = temp? true : false;

	GetInt( SSAPI_SOFTWARE_IMAGE_FID_IS_PRIMARY, &temp );
	m_isPrimary = temp? true : false;

	return true;
}


//************************************************************************
// BuildYourselfFromImageDesc:
//
// PURPOSE:		Populates data member based on the information contained
//				in the image descriptor
//************************************************************************

void 
SoftwareImage::BuildYourselfFromImageDesc(	ImageDesc		&imageDesc, 
											int				slotNumber, 
											DesignatorId	descriptorId, 
											DesignatorId	deviceId ){

	m_slotNumber	= slotNumber;
	m_descriptorId	= descriptorId;
	m_deviceId		= deviceId;
	m_isAccepted	= imageDesc.IsAccepted()? true : false;
	m_isCurrent		= imageDesc.IsCurrent()? true : false;
	m_isPrimary		= imageDesc.IsPrimary()? true : false;
	m_id			= DesignatorId( imageDesc.GetKey(), SSAPI_OBJECT_CLASS_TYPE_SOFTWARE_IMAGE );
	imageDesc.TimeBooted( &m_lastBoot );
}


//************************************************************************
//
//************************************************************************

void 
SoftwareImage::SetIsPrimary( bool isPrimary, bool postEvent ){

	if( m_isPrimary != isPrimary ){
		m_isPrimary = isPrimary;
		if( postEvent )
			this->FireEventObjectModifed();
	}
}


void 
SoftwareImage::SetIsAccepted( bool isAccepted, bool postEvent ){

	if( m_isAccepted != isAccepted ){
		m_isAccepted = isAccepted;
		if( postEvent )
			this->FireEventObjectModifed();
	}
}