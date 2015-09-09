//************************************************************************
// FILE:		PowerSupply.cpp
//
// PURPOSE:		Implements an abstract class PowerSupply  that serves  as
//				a base class for all power supply- type objects
//************************************************************************


#include "PowerSupply.h"


//************************************************************************
// PowerSupply:
//
// PURPOSE:		Default constructor
//************************************************************************

PowerSupply::PowerSupply( ListenManager *pListenManager, U32 objectClassType, int number, int numInEvc )
			:Device( pListenManager, objectClassType ){

	m_number		= number;
	m_numberInEvcRow= numInEvc;
	m_location		= m_number;
}


//************************************************************************
// ~PowerSupply:
//
// PURPOSE:		The destructor
//************************************************************************

PowerSupply::~PowerSupply(){
}



