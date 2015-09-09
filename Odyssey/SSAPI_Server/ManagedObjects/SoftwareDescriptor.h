//************************************************************************
// FILE:		SoftwareDescriptor.h
//
// PURPOSE:		Defines the object whose instances will be used to 
//				represent descriptors of the images contained inside
//				an O2K box.
//************************************************************************

#ifndef __SOFTWARE_DESCRIPTOR_H__
#define	__SOFTWARE_DESCRIPTOR_H__

#include "ManagedObject.h"
#include "SsapiTypes.h"


class Image;

class SoftwareDescriptor : public ManagedObject {

	U32					m_majorVersion;
	U32					m_minorVersion;
	LocalizedDateTime	m_loadedOn;
	U32					m_referenceCount;
	bool				m_isDefault;

protected:

//************************************************************************
// SoftwareDescriptor:
//
// PURPOSE:		Default constructor
//************************************************************************

SoftwareDescriptor( ListenManager *pLM, ObjectManager *pManager, U32 classType );


public:

//************************************************************************
// ~SoftwareDescriptor:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~SoftwareDescriptor();


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
//
//************************************************************************

virtual const ValueSet& operator=(const ValueSet& obj ){ return *((ValueSet *)this) = obj; }


//************************************************************************
// BuildYourselfFromImageObject:
//
// PURPOSE:		Populates data members based on the info in the Image
//				object given
//************************************************************************

void BuildYourselfFromImageObject( Image &image );


//************************************************************************
// SetIsDefault:
//
//************************************************************************

void SetIsDefault( bool isDefault, bool postEvent = false );

bool GetIsDefault() const { return m_isDefault; }

};

#endif	// __SOFTWARE_DESCRIPTOR_H__