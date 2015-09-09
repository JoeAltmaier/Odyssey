//************************************************************************
// FILE:		DeviceCollection.cpp
//
// PURPOSE:		Implements class DeviceCollection that will contain
//				somehow associated device objects used to represent
//				devices in the O2K product. The state of a device collection
//				is defined as the worst state of any of its elements
//************************************************************************

#include "DeviceCollection.h"

//************************************************************************
// ComposeYourOverallState:
//
// PURPOSE:		Called before a device object is serialized to allow 
//				all the sub classes to set what their overall state is
//				the state should be put into 'm_state' data member
//************************************************************************

void 
DeviceCollection::ComposeYourOverallState(){

	if( StatusReporterInterface::CanChangeStateToGood() ){
		m_state = SSAPI_OBJECT_STATE_GOOD;
		m_stateString = CTS_SSAPI_OBJECT_STATE_NAME_GOOD;
	}
/*
	int						state, savedState = m_state;
	Device					*pDevice;
	
	if( StatusReporterInterface::CanChangeStateToGood() ){
		m_state = SSAPI_OBJECT_STATE_GOOD;
		m_stateString = CTS_SSAPI_OBJECT_STATE_NAME_GOOD;

		for( U32 i = 0; i < m_children.Count(); i++ ){
			pDevice	= (Device *)GetChild( i );
			if( (state = pDevice->GetState() ) > m_state ){
				m_stateString = pDevice->GetStateString();
				m_state = state;
			}
		}

		if( m_state != savedState )
			FireEventObjectModifed();
	}
*/
}

