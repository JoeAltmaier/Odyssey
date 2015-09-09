//************************************************************************
// FILE:		StatusReporterInterface.cpp
//
// PURPOSE:		Implements generic SSAPI status reporting interface.\
//				This abstract class is to be inherited from by all 
//				objects wishing to be SSAPI status reporting complient.
//************************************************************************

#include "StatusReporterInterface.h"
#include "SList.h"
#include "ValueSet.h"
#include "CtEvent.h"
#include "ManagedObject.h"

//************************************************************************
// StatusReporterInterface:
//
// PURPOSE:		Default constructor
//************************************************************************

StatusReporterInterface::StatusReporterInterface(){

	m_state			= 0;
	m_stateString	= 0;
	m_isRolledUp	= false;

}


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
//
// NOTE:		The current rules are:
//					if parent is DEAD, the object is UNKNOWN
//					if child is not GOOD, the object is WARNING
//************************************************************************

bool 
StatusReporterInterface::ApplyNecessaryStatusRollupRules(	ManagedObject	*pObject,
															const Container &children, 
															const Container &parents ){

	ManagedObject					*pParent, *pChild;
	U32								index;
	int								newState, oldState;
	bool							canChangeFlag = true;


	// first check parents
	newState		= SSAPI_OBJECT_STATE_UNKNOWN;
	for( index = 0; index < parents.Count(); index++ ){
		if ( pObject->GetParent( index ) == NULL )
			pObject= pObject;
		pParent = pObject->GetParent( index );
		pParent->BuildYourValueSet();
		pParent->GetInt( SSAPI_OBJECT_FID_STATE, &oldState );
		pParent->Clear();
		if( (oldState == SSAPI_OBJECT_STATE_DEAD) || (oldState == CTS_SSAPI_OBJECT_STATE_NAME_UNKNOWN) ){
			if( newState != m_state ){
				m_state			= newState;
				m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_UNKNOWN;
				m_isRolledUp	= true;
				return true;
			}
			else
				canChangeFlag = false;
		}
	}

	// second, check children
	newState		= SSAPI_OBJECT_STATE_WARNING;
	for( index = 0; index < children.Count(); index++ ) {
		pChild = pObject->GetChild( index );
		pChild->BuildYourValueSet();
		pChild->GetInt( SSAPI_OBJECT_FID_STATE, &oldState );
		pChild->Clear();
		if( oldState != SSAPI_OBJECT_STATE_GOOD ){
			if( (m_state != newState) && (m_state == SSAPI_OBJECT_STATE_GOOD) ){
				m_state			= newState;
				m_stateString	= CTS_SSAPI_OBJECT_STATE_NAME_WARNING;	
				m_isRolledUp	= true;
				return true;
			}
			else
				canChangeFlag	= false;
		}
	}
	
	if( canChangeFlag && m_isRolledUp ){
		m_isRolledUp = false;
		m_state		 = SSAPI_OBJECT_STATE_GOOD;
		m_stateString= CTS_SSAPI_OBJECT_STATE_NAME_GOOD;
		return true;
	}

	return false;
}



//************************************************************************
// BuildYourValueSet:
//
// PURPOSE:		Populates object's value set with its members
//************************************************************************

bool 
StatusReporterInterface::BuildYourValueSet( ValueSet &vs ){

	int		rc = 1;

	rc &= vs.AddInt( m_state, SSAPI_OBJECT_FID_STATE );
	rc &= vs.AddU32( m_stateString, SSAPI_OBJECT_FID_STATE_STRING );

	return rc? true : false;
}


//************************************************************************
// BuildYourselfFromYourValueSet:
//
// PURPOSE:		Populates its data members from object's value set
//************************************************************************

bool 
StatusReporterInterface::BuildYourselfFromYourValueSet( ValueSet &vs ){

	int		rc = 1;

	rc &= vs.GetInt( SSAPI_OBJECT_FID_STATE, &m_state );
	rc &= vs.GetU32( SSAPI_OBJECT_FID_STATE_STRING, &m_stateString );

	return rc? true : false;
}

