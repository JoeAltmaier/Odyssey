//************************************************************************
// FILE:		MicroController.h
//
// PURPOSE:		Defines the object used to represent microcontrollers 
//				on ConvergeNet's hardware (AVR's, EEPROM's)
//************************************************************************

#ifndef	__MICRO_CONTROLLER_H__
#define	__MICRO_CONTROLLER_H__

#include "SsapiTypes.h"
#include "UnicodeString.h"
#include "ValueSet.h"

#ifdef WIN32
#pragma pack(4)
#endif

class MicroController : public ValueSet {

protected:
	
	LocalizedString			m_name;
	UnicodeString			m_hwVersion;
	UnicodeString			m_fwVersion;

public:


//************************************************************************
// MicroController:
//
// PURPOSE:		The constructor
//************************************************************************

MicroController(	LocalizedString name	= 0, 
					UnicodeString hwVersion	= StringClass(""), 
					UnicodeString swVersion	= StringClass("") );


//************************************************************************
// ~MicroController:
//
// PURPOSE:		The destructor
//************************************************************************

~MicroController(){}


//************************************************************************
// BuildYourValueSet:
//
// PURPOSE:		Populates the its value set with its data members
//************************************************************************

bool BuildYourValueSet();


//************************************************************************
// BuildYourselfFromYourValueSet:
//
// PURPOSE:		Populates data members from the value set
//************************************************************************

bool BuildYourselfFromYourValueSet();


//************************************************************************
// operator=
//************************************************************************


const ValueSet& operator=(const ValueSet& obj ){ return *this = obj; }

};

#endif	// __MICRO_CONTROLLER_H__


