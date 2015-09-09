//******************************************************************************
// File:		DescriptorCollector.cpp
//
// PURPOSE:		A backet to be used to colect descriptors of new items
//******************************************************************************


#include "DescriptorCollector.h"
#include "SsapiAssert.h"

//******************************************************************************
// DescriptorCollector:
//
// PURPOSE:		Default constructor
//******************************************************************************

DescriptorCollector::DescriptorCollector(){

	m_pBacket	= new CoolVector;
}


//******************************************************************************
// ~DescriptorCollector:
//
// PURPOSE:		The destructor
//******************************************************************************

DescriptorCollector::~DescriptorCollector(){

	void	*p;

	while( m_pBacket->Count() ){
		m_pBacket->GetAt( (CONTAINER_ELEMENT&)p, 0 );
		m_pBacket->RemoveAt(0);
		delete p;
	}

	delete m_pBacket;

}


//******************************************************************************
// Add:
//
// PURPOSE:		Adds a descriptor
//******************************************************************************

bool 
DescriptorCollector::Add( void *pDescriptor, U32 size ){

	void *p = new char[ size ];

	memcpy( p, pDescriptor, size );

	return m_pBacket->Add( (CONTAINER_ELEMENT)p) ? true : false;

}


//******************************************************************************
// Get:
//
// PURPOSE:		Fetches the descriptor with the rid requested
//******************************************************************************

bool 
DescriptorCollector::Get( const RowId &rid, void* &pDescriptor ){

	void	*p;
	U32		i;

	for( i = 0; i < m_pBacket->Count(); i++ ){
		m_pBacket->GetAt( (CONTAINER_ELEMENT &)p, i );
		if( *((RowId *)p) == rid ){
			pDescriptor = p;
			return true;
		}
	}

	return false;
}

bool 
DescriptorCollector::GetAt( U32 position, void *&pDescriptor ){

	void *p = NULL;

	if( m_pBacket->GetAt( (CONTAINER_ELEMENT &)p, position ) ){
		pDescriptor = p;
		return true;
	}

	return false;
}


//******************************************************************************
// Delete:
//
// PURPOSE:		Deletes the descriptor with the 'rid' requested
//******************************************************************************

bool 
DescriptorCollector::Delete( RowId rid ){

	void	*p;
	U32		i;

	for( i = 0; i < m_pBacket->Count(); i++ ){
		m_pBacket->GetAt( (CONTAINER_ELEMENT &)p, i );
		if( *((RowId *)p) == rid ){
			m_pBacket->RemoveAt( i );
			delete p;
			return true;
		}
	}

	return false;
}

