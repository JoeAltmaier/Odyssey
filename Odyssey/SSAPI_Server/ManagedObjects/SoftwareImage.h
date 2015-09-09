//************************************************************************
// FILE:		SoftwareImage.h
//
// PURPOSE:		Defines the object whose instances will represent
//				software images associated with devices
//************************************************************************

#ifndef __SOFTWARE_IMAGE_H__
#define	__SOFTWARE_IMAGE_H__


#include "ManagedObject.h"
#include "SsapiTypes.h"

class ImageDesc;

class SoftwareImage : public ManagedObject{

	DesignatorId		m_descriptorId;
	DesignatorId		m_deviceId;
	bool				m_isAccepted;
	bool				m_isCurrent;
	bool				m_isPrimary;
	LocalizedDateTime	m_lastBoot;
	int					m_slotNumber;

public:

//************************************************************************
// SoftwareImage:
//

//************************************************************************

SoftwareImage( ListenManager *pLManager, ObjectManager *pManager );


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new SoftwareImage( GetListenManager(), GetManager() ); }


virtual ~SoftwareImage();


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
// operator=:
//************************************************************************

virtual const ValueSet& operator=(const ValueSet& obj ){ return *((ValueSet *)this) = obj; }


//************************************************************************
// Public accessors:
//************************************************************************

int GetSlotNumber() const { return m_slotNumber; }


//************************************************************************
// BuildYourselfFromImageDesc:
//
// PURPOSE:		Populates data member based on the information contained
//				in the image descriptor
//************************************************************************

void BuildYourselfFromImageDesc(ImageDesc		&imageDesc, 
								int				slotNumber, 
								DesignatorId	descriptorId, 
								DesignatorId	deviceId );


//************************************************************************
// Public accessors:
//************************************************************************

bool GetIsPrimary() const { return m_isPrimary; }
bool GetIsCurrent() const { return m_isCurrent; }
DesignatorId& GetDescriptorId() { return m_descriptorId; }
DesignatorId& GetDeviceId()		{ return m_deviceId; }

void SetIsPrimary( bool isPrimary, bool postEvent = false );
void SetIsAccepted( bool isAccespted, bool postEvent = false );

};

#endif	// __SOFTWARE_IMAGE_H__