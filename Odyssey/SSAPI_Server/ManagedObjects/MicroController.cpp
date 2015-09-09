//************************************************************************
// FILE:		MicroController.cpp
//
// PURPOSE:		Implements the object used to represent microcontrollers 
//				on ConvergeNet's hardware (AVR's, EEPROM's)
//************************************************************************


#include "MicroController.h"


//************************************************************************
// BuildYourValueSet:
//
// PURPOSE:		Populates the its value set with its data members
//************************************************************************

bool 
MicroController::BuildYourValueSet(){

	AddU32( m_name, SSAPI_MICROCONTROLLER_FID_NAME );
	AddString( &m_hwVersion, SSAPI_MICROCONTROLLER_FID_HW_VERSION );
	AddString( &m_fwVersion, SSAPI_MICROCONTROLLER_FID_FW_VERSION );

	return true;
}


//************************************************************************
// BuildYourselfFromYourValueSet:
//
// PURPOSE:		Populates data members from the value set
//************************************************************************

bool 
MicroController::BuildYourselfFromYourValueSet(){

	GetU32( SSAPI_MICROCONTROLLER_FID_NAME, &m_name );
	GetString( SSAPI_MICROCONTROLLER_FID_HW_VERSION, &m_hwVersion );
	GetString( SSAPI_MICROCONTROLLER_FID_FW_VERSION, &m_fwVersion );

	return true;
}


//************************************************************************
// MicroController:
//
// PURPOSE:		The constructor
//************************************************************************

MicroController::MicroController(	LocalizedString name, 
									UnicodeString hwVersion, 
									UnicodeString fwVersion ){

	m_name		= name;
	m_hwVersion	= hwVersion;
	m_fwVersion	= fwVersion;
}