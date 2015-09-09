//************************************************************************
// FILE:		AssetInterface.h
// 
// PURPOSE:		DEfines that the asset information available for some
//				of the device objects in the SSAPI
//************************************************************************

#ifndef __ASSET_INTERFACE_H__
#define	__ASSET_INTERFACE_H__

#include "SsapiTypes.h"
#include "UnicodeString.h"

class ValueSet;

#ifdef WIN32
#pragma pack(4)
#endif

class AssetInterface {

	UnicodeString		m_serialNumber;
	UnicodeString		m_versionNumber;
	LocalizedDateTime	m_productionDate;
	bool				m_hasSerialNumber;
	bool				m_hasVersionNumber;
	bool				m_hasProductionDate;

public:


//************************************************************************
// AssetInterface:
//
// PURPOSE:		Default constructor
//************************************************************************

AssetInterface() { ClearInfo(); }


//************************************************************************
// BuildYourValueSet:
//
// PURPOSE:		Populates the seed value set with its data members
//************************************************************************

bool BuildYourValueSet( ValueSet &seed );


//************************************************************************
// BuildYourselfFromYourValueSet:
//
// PURPOSE:		Populates its data members with values from the seed value 
//				set
//************************************************************************

bool BuildYourselfFromYourValueSet( ValueSet &seed );


//************************************************************************
// ClearInfo:
//
// PURPOSE:		Clears information possesed by the asset interface
//************************************************************************

void ClearInfo();


//************************************************************************
// Public Accessors:
//************************************************************************

UnicodeString GetSerialNumber()	const	{ return m_serialNumber;}
UnicodeString GetVersionNumber() const	{ return m_versionNumber; }
LocalizedDateTime& GetProductionDate()	{ return m_productionDate; }

bool HasSerialNumber() const	{ return m_hasSerialNumber; }
bool HasVersionNumber() const	{ return m_hasVersionNumber; }
bool HasProductionDate() const	{ return m_hasProductionDate; }

void SetProductionDate( LocalizedDateTime& d )	{ memcpy( &m_productionDate, &d, sizeof(d) ); m_hasProductionDate = true; }
void SetVersionNumber( UnicodeString& s )		{ m_versionNumber = s; m_hasVersionNumber = true; }
void SetSerialNumber( UnicodeString& s )		{ m_serialNumber = s; m_hasSerialNumber = true; }

};

#endif	// __ASSET_INTERFACE_H__