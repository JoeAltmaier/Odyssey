//************************************************************************
// FILE:		PowerSupply.h
//
// PURPOSE:		Defines an abstract class PowerSupply  that serves  as
//				a base class for all power supply- type objects
//************************************************************************


#ifndef __POWER_SUPPLY_H__
#define __POWER_SUPPLY_H__


#include "Device.h"


#ifdef WIN32
#pragma pack(4)
#endif


class PowerSupply : public Device{

protected:

	int			m_number;			// internal, used for accessing PTS row
	int			m_numberInEvcRow;	// internal, used for PHS data hook up

public:

//************************************************************************
// PowerSupply:
//
// PURPOSE:		Default constructor
//************************************************************************

PowerSupply( ListenManager *pListenManager, U32 objectClassType, int number, int numInEvc );


//************************************************************************
// ~PowerSupply:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~PowerSupply();


//************************************************************************
//************************************************************************
//************************************************************************

};


#endif	// __POWER_SUPPLY_H__