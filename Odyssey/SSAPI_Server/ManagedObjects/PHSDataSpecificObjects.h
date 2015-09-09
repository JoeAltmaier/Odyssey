//************************************************************************
// FILE:		PHSDataSpecificObjects.h
//
// PURPOSE:		Defines specific PHSData classes
//
// CLASSES:		PHSDataTemperature		-- in F
//				PHSDataTime				-- in microseconds ellapsed
//				PHSDataThoughput		-- in bytes, value is I64
//				PHSDataVoltage			-- in millivotls
//				PHSDataCurrent			-- in milliamps
//************************************************************************

#ifndef __H_PHS_DATA_SPECIFIC_MO_H__
#define	__H_PHS_DATA_SPECIFIC_MO_H__

#include "PHSDataInt64.h"
#include "PHSDataInt.h"



class PHSDataCurrent : public PHSDataInt{

public:

//************************************************************************
// PHSDataCurrent:
//
// PURPOPSE:		Default constructor
//************************************************************************

PHSDataCurrent( ListenManager *pListenManager, DesignatorId id, VALUE_TYPE valueType )
	:PHSDataInt( pListenManager, id, valueType, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_CURRENT ){}


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new PHSDataCurrent(	GetListenManager(),
																	GetDesignatorId(),
																	m_valueType ); }

};



class PHSDataVoltage : public PHSDataInt{

public:

//************************************************************************
// PHSDataVoltage:
//
// PURPOPSE:		Default constructor
//************************************************************************

PHSDataVoltage( ListenManager *pListenManager, DesignatorId id, VALUE_TYPE valueType )
	:PHSDataInt( pListenManager, id, valueType, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_VOLTAGE ){}


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new PHSDataVoltage(	GetListenManager(),
																	GetDesignatorId(),
																	m_valueType ); }

};



class PHSDataTemperature : public PHSDataInt{

public:

//************************************************************************
// PHSDataTemperature:
//
// PURPOPSE:		Default constructor
//************************************************************************

PHSDataTemperature( ListenManager *pListenManager, DesignatorId id, VALUE_TYPE valueType = SNAPSHOT )
	:PHSDataInt( pListenManager, id, valueType, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_TEMPERATURE ){}


void SetValueInCelcius( int valueC )			{ m_value	= ((valueC+32)*9)/5; }


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new PHSDataTemperature(	GetListenManager(),
																		GetDesignatorId(),
																		m_valueType ); }

};



class PHSDataTime : public PHSDataInt64{

public:

//************************************************************************
// PHSDataTime:
//
// PURPOSE:		Default constructor
//************************************************************************

PHSDataTime( ListenManager *pListenManager, DesignatorId id, VALUE_TYPE valueType )
	:PHSDataInt64( pListenManager, id, valueType, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_TIME ){}


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new PHSDataTime(GetListenManager(),
																GetDesignatorId(),
																m_valueType ); }

};


class PHSDataThroughput : public PHSDataInt64{

public:

//************************************************************************
// PHSDataThroughput:
//
// PURPOSE:		Default constructor
//************************************************************************

PHSDataThroughput( ListenManager *pListenManager, DesignatorId id, VALUE_TYPE valueType )
	:PHSDataInt64( pListenManager, id, valueType, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT ){}


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new PHSDataThroughput(	GetListenManager(),
																		GetDesignatorId(),
																		m_valueType ); }
};




#endif // __H_PHS_DATA_SPECIFIC_MO_H__