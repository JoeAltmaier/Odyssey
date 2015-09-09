//************************************************************************
// FILE:		StorageIdInfo.cpp
//
// PURPOSE:		Implements the oibject that carries all the information
//				used to indentify a storage device.
//************************************************************************

#include "StorageIdInfo.h"



//************************************************************************
// BuildYourValueSet:
//
// PURPOSE:		Populates the internal value set with the data members
//************************************************************************

void 
StorageIdInfo::BuildYourValueSet(){

	AddString( &m_vendor, SSAPI_STORAGE_ID_INFO_FID_VENDOR );
	AddString( &m_product,SSAPI_STORAGE_ID_INFO_FID_PRODUCT  );
	AddString( &m_revision, SSAPI_STORAGE_ID_INFO_FID_REVISION );
	AddString( &m_serialNumber, SSAPI_STORAGE_ID_INFO_FID_SERIAL_NUMBER );
	AddString( &m_wwName, SSAPI_STORAGE_ID_INFO_FID_WWNAME );
	AddU32( m_targetLUN, SSAPI_STORAGE_ID_INFO_FID_TARGET_LUN );
	AddU32( m_targetId, SSAPI_STORAGE_ID_INFO_FID_TARGET_ID );
}


//************************************************************************
// BuildYourselfFromYourValueSet:
//
// PURPOSE:		Populates data members out of the local value set
//************************************************************************

void 
StorageIdInfo::BuildYourselfFromYourValueSet(){

	GetString( SSAPI_STORAGE_ID_INFO_FID_VENDOR, &m_vendor );
	GetString( SSAPI_STORAGE_ID_INFO_FID_PRODUCT, &m_product );
	GetString( SSAPI_STORAGE_ID_INFO_FID_REVISION, &m_revision );
	GetString( SSAPI_STORAGE_ID_INFO_FID_SERIAL_NUMBER, &m_serialNumber );
	GetString( SSAPI_STORAGE_ID_INFO_FID_WWNAME, &m_wwName );
	GetU32( SSAPI_STORAGE_ID_INFO_FID_TARGET_LUN, &m_targetLUN );
	GetU32( SSAPI_STORAGE_ID_INFO_FID_TARGET_ID, &m_targetId );
}

