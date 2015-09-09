//************************************************************************
// FILE:		AssetInterface.cpp
// 
// PURPOSE:		Implements that the asset information available for some
//				of the device objects in the SSAPI
//************************************************************************

#include "AssetInterface.h"
#include "ValueSet.h"
#include "SSAPI_Codes.h"

//************************************************************************
// BuildYourValueSet:
//
// PURPOSE:		Populates the seed value set with its data members
//************************************************************************

bool 
AssetInterface::BuildYourValueSet( ValueSet &seed ){

	ValueSet		*pVs = new ValueSet;
	
	if( m_hasSerialNumber )
		pVs->AddString( &m_serialNumber, SSAPI_ASSET_FID_SERIAL_NUMBER );

	if( m_hasVersionNumber )
		pVs->AddString( &m_versionNumber, SSAPI_ASSET_FID_VERSION_NUMBER );

	if( m_hasProductionDate )
		pVs->AddInt64( (void *)&m_productionDate, SSAPI_ASSET_FID_PRODUCTION_DATE );

	seed.AddValue( pVs, SSAPI_OBJECT_FID_ASSET_TAG_OBJECT );

	delete pVs;
	return true;
}


//************************************************************************
// BuildYourselfFromYourValueSet:
//
// PURPOSE:		Populates its data members with values from the seed value 
//				set
//************************************************************************

bool 
AssetInterface::BuildYourselfFromYourValueSet( ValueSet &seed ){

	ValueSet		*pVs = (ValueSet *)seed.GetValue( SSAPI_OBJECT_FID_ASSET_TAG_OBJECT );

	if( pVs ){
		m_hasSerialNumber = pVs->GetString( SSAPI_ASSET_FID_SERIAL_NUMBER, &m_serialNumber )? true : false;
		m_hasVersionNumber = pVs->GetString( SSAPI_ASSET_FID_VERSION_NUMBER, &m_versionNumber ) ? true : false;
		m_hasProductionDate = pVs->GetInt64( SSAPI_ASSET_FID_PRODUCTION_DATE, &m_productionDate )? true : false;
	}

	return true;
}


//************************************************************************
// ClearInfo:
//
// PURPOSE:		Clears information possesed by the asset interface
//************************************************************************

void 
AssetInterface::ClearInfo(){

	m_hasSerialNumber		= false;
	m_hasVersionNumber		= false;
	m_hasProductionDate		= false;
}