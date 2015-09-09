//************************************************************************
// FILE:		StatusReporterInterface.h
//
// PURPOSE:		Defines generic SSAPI status reporting interface.\
//				This abstract class is to be inherited from by all 
//				objects wishing to be SSAPI status reporting complient.
//************************************************************************

#ifndef __STATUS_REPORTER_INTERFACE_H__
#define __STATUS_REPORTER_INTERFACE_H__

#include "CtTypes.h"
#include "SSAPITypes.h"
#include "SSAPIObjectStates.h"


class ManagedObject;
class Container;
class ValueSet;

#ifdef WIN32
#pragma pack(4)
#endif

class StatusReporterInterface{

protected:

	int						m_state;			// see SsapiObjectStates.h
	U32						m_stateString;
	bool					m_isRolledUp;


//************************************************************************
// ApplyNecessaryStatusRollupRules:
//
// PURPOSE:		Applies applicabale rollup rules.
//
// RECEIVE:		children:		vector with child objects
//				parents:		vector with parent objects
//
// RETURN:		true:			object's state's been changed
//				false:			object's state's remained unchanged
//************************************************************************

bool ApplyNecessaryStatusRollupRules(	ManagedObject	*pObject,
										const Container &children, 
										const Container &parents );


//************************************************************************
// BuildYourValueSet:
//
// PURPOSE:		Populates object's value set with its members
//************************************************************************

bool BuildYourValueSet( ValueSet &vs );


//************************************************************************
// BuildYourselfFromYourValueSet:
//
// PURPOSE:		Populates its data members from object's value set
//************************************************************************

bool BuildYourselfFromYourValueSet( ValueSet &vs );


public:

//************************************************************************
// StatusReporterInterface:
//
// PURPOSE:		Default constructor
//************************************************************************

StatusReporterInterface();


//************************************************************************
// Acessors
//
//
//************************************************************************

int GetState() const		{ return m_state; }

U32 GetStateString() const	{ return m_stateString; }


//************************************************************************
// ApplyNecessaryStatusRollupRules:
//
// PURPOSE:		Applies applicabale rollup rules.
//				The object that implements this method is expected to call
//				the protected method with the same name and specify 
//				necessary parms.
//
// RETURN:		true:			object's state's been changed
//				false:			object's state's remained unchanged
//************************************************************************

virtual bool ApplyNecessaryStatusRollupRules() = 0;


//************************************************************************
// 
//************************************************************************

bool CanChangeStateToGood() const { return !m_isRolledUp; }


//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		Called before an object is serialized to allow 
//				all the sub classes to set what their overall state is
//				the state should be put into 'm_state' data member
//************************************************************************

virtual void ComposeYourOverallState() = 0;


};
#endif // __STATUS_REPORTER_INTERFACE_H__
