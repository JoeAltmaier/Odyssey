//************************************************************************
// FILE:		SSAPIObjectStates.h
//
// PURPOSE:		Defines numeric values for the generalized object state
//				This will be used by our clients to map object state to
//				some representation (color, etc)
//************************************************************************

#ifndef __SSAPI_OBJECT_STATES_H__
#define	__SSAPI_OBJECT_STATES_H__

enum SSAPI_OBJECT_STATE{
	SSAPI_OBJECT_STATE_GOOD		= 0,	
	SSAPI_OBJECT_STATE_WARNING,
	SSAPI_OBJECT_STATE_DEGRADED,
	SSAPI_OBJECT_STATE_DEAD,
	SSAPI_OBJECT_STATE_UNKNOWN,
	SSAPI_OBJECT_STATE_DIAG
	

};

#endif // __SSAPI_OBJECT_STATES_H__