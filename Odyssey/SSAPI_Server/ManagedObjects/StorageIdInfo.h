//************************************************************************
// FILE:		StorageIdInfo.h
//
// PURPOSE:		Define the oibject that carries all the information
//				used to indentify a storage device.
//************************************************************************

#ifndef __STORAGE_ID_INFO__
#define	__STORAGE_ID_INFO__


#include "ValueSet.h"
#include "SsapiTypes.h"
#include "CtTypes.h"
#include "SsapiAssert.h"
#include "UnicodeString.h"

class StorageIdInfo : public ValueSet {

	UnicodeString			m_vendor;
	UnicodeString			m_product;
	UnicodeString			m_revision;
	UnicodeString			m_serialNumber;
	UnicodeString			m_wwName;
	U32						m_targetLUN;
	U32						m_targetId;

public:

//************************************************************************
// StorageIdInfo:
// 
// PURPOSE:		Default constructor
//************************************************************************

StorageIdInfo(){}


//************************************************************************
// BuildYourValueSet:
//
// PURPOSE:		Populates the internal value set with the data members
//************************************************************************

void BuildYourValueSet();


//************************************************************************
// BuildYourselfFromYourValueSet:
//
// PURPOSE:		Populates data members out of the local value set
//************************************************************************

void BuildYourselfFromYourValueSet();


//************************************************************************
// Public Accessors
//************************************************************************

void SetVendor( UnicodeString& vendor ) { m_vendor = vendor; }
void SetProduct( UnicodeString& product ) { m_product = product; }
void SetRevision( UnicodeString& revision ) { m_revision = revision; }
void SetSerialNumber( UnicodeString& sNumber ) { m_serialNumber = sNumber; }
void SetTargetId( U32 id ) { m_targetId = id; }
void SetTargetLUN( U32 lun ) { m_targetLUN = lun; }
void SetWWName( UnicodeString& wwName ) { m_wwName = wwName; }

U32  GetTargetLUN() const { return m_targetLUN; }
U32	 GetTargetId() const { return m_targetId; }


};
#endif	// __STORAGE_ID_INFO__