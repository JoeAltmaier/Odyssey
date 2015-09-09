//******************************************************************************
// FIlE:		Raid2SsapiErrorConvertor.h
//
// PURPOSE:		Converts Raid Master error codes to the SSAPI exception codes
//******************************************************************************

#ifndef __RAID_2_SSAPI_ERR_CONVERTOR_H__
#define	__RAID_2_SSAPI_ERR_CONVERTOR_H__

#include "CtEvent.h"
#include "CtTypes.h"


class Raid2SsapiErrorConvertor {

public:

//******************************************************************************
// Raid2SsapiErrorConvertor:
//
// PURPOSE:		Default constructor
//******************************************************************************

Raid2SsapiErrorConvertor() {}


//******************************************************************************
// GetSsapiException:
//
// PURPOSE:		Performs a lookup of exception code by raid error code
//******************************************************************************

U32 GetSsapiException( U32 raidErrorCode );
U32 GetSsapiPartitionException( U32 partitionErrorCode );

};

#endif // __RAID_2_SSAPI_ERR_CONVERTOR_H__
