//************************************************************************
// FILE:		RedundantDataPath.h
//
// PURPOSE:		Dedines class used to represent redundant data paths between
//				a host's  and NAC's port.
//************************************************************************

#ifndef __REDUNDANT_DATA_PATH_H__
#define	__REDUNDANT_DATA_PATH_H__

#include "DataPath.h"
#include "HostConnectionDescriptorTable.h"

#ifdef WIN32
#pragma pack(4)
#endif

class RedundantDataPath : public DataPath{
		
	
public:

//************************************************************************
// RedundantDataPath:
//
// PURPOSE:		The default constructor
//************************************************************************

RedundantDataPath( ListenManager *pListenManager, ObjectManager *pManager );


//************************************************************************
// CreateInstance:
//
// PURPOSE:		Creates an instance of the same time as it is.
//				The best attempt is made to clone data members that
//				are not a part of the object's value set. The ones that
//				are will not be copied - they can copied manually thru 
//				value set's functionality.
//************************************************************************

virtual ManagedObject* CreateInstance(){ return new RedundantDataPath( GetListenManager(), GetManager() ); }


//************************************************************************
// ~RedundantDataPath:
//
// PURPOSE:		The destructor
//************************************************************************

virtual ~RedundantDataPath();


//************************************************************************
// BuildYourValueSet:
//
// PURPOSE:		The method is responsible for adding all data to be 
//				transfered to a client into its value set. All derived 
//				objects must override this method if they have data members
//				they want client proxies to have. 
//
// NOTE:		If a derived object overrides this this method, it MUST call
//				it in its base class BEFORE doing any work!.
//
// RETURN:		true:		success
//************************************************************************

virtual bool BuildYourValueSet();


//************************************************************************
// BuildYourselfFromYourValueSet:
//
// PURPOSE:		Populates data members based on the underlying value set
//
// NOTE:		All subclasses that override this method must call to it
//				somewhere in the overriding method
//
// RETURN:		true:		success
//************************************************************************

virtual bool BuildYourselfFromYourValueSet();


//************************************************************************
// IsAValidConfiguration:
//
// PURPOSE:		Checks the connections assigned to this data path and
//				responds if this is a valid set of connections
//
// OUTPUT:		If the configuration is invalid, 'errorString' will
//				contain the localized string that explains the problem
//************************************************************************

virtual bool IsAValidConfiguration( U32 &errorString );


//************************************************************************
// GetPathType:
//
// PURPOSE:		Returns an integer to be stored in the PTS row so that
//				the right type of object could be built next time.
//
// NOTE:		declared pure virtual to enforce implementation of the 
//				method
//************************************************************************

virtual U32 GetPathType() { return ePATH_TYPE_REDUNDANT; }
};

#endif	// __REDUNDANT_DATA_PATH_H__
