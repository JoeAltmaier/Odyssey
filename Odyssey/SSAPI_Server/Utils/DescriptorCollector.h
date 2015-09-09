//******************************************************************************
// File:		DescriptorCollector
//
// PURPOSE:		A backet to be used to colect descriptors of new items
//******************************************************************************

#ifndef __DESCRIPTOR_COLLECTOR_H__
#define	__DESCRIPTOR_COLLECTOR_H__

#include "CoolVector.h"
#include "DesignatorId.h"
#include "CtTypes.h"

#ifdef WIN32
#pragma pack(4)
#endif

class DescriptorCollector {
	
	CoolVector				*m_pBacket;

public:

//******************************************************************************
// DescriptorCollector:
//
// PURPOSE:		Default constructor
//******************************************************************************

DescriptorCollector();


//******************************************************************************
// ~DescriptorCollector:
//
// PURPOSE:		The destructor
//******************************************************************************

~DescriptorCollector();


//******************************************************************************
// Add:
//
// PURPOSE:		Adds a descriptor
//******************************************************************************

bool Add( void *pDescriptor, U32 size );


//******************************************************************************
// Get:
//
// PURPOSE:		Fetches the descriptor with the rid requested
//******************************************************************************

bool Get( const RowId &rid, void* &pDescriptor );
bool GetAt( U32 position, void *&pDescriptor );


//******************************************************************************
// Delete:
//
// PURPOSE:		Deletes the descriptor with the 'rid' requested
//******************************************************************************

bool Delete( RowId rid );

};


#endif // __DESCRIPTOR_COLLECTOR_H__