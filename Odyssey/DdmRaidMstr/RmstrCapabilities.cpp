/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// (c) Copyright 1999 ConvergeNet Technologies, Inc.
//     All Rights Reserved.
//
// File: RmstrCapabilities.cpp
// 
// Description:
// Implementation for the RmstrCapabilities class. It will allow the reading
//	and writing of RMSTR capabilities.
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RmstrCapabilities.cpp $
// 
// 3     8/14/99 1:37p Dpatel
// Added event logging..
// 
// 2     8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64. added simulation raid
// ddm.
//
/*************************************************************************/
#include "RmstrCapabilities.h"


//************************************************************************
//	CONSTRUCTOR
//		Initialize our table services object so that we can access
//		our table services.
//
//	pADTRecord		- the array to start the regenerate on
//
//************************************************************************
RmstrCapabilities::RmstrCapabilities(DdmServices *pParentDdm){
	m_pTableServices = new TableServices(pParentDdm);
}


//************************************************************************
//	DESTRUCTOR
//		Delete the table services object that we allocated.
//
//************************************************************************
RmstrCapabilities::~RmstrCapabilities(){
	if (m_pTableServices)
		delete m_pTableServices;
}

//************************************************************************
//	GetRmstrCapability
//		Will read the capability according to the capability code and will
//		set the pFound and pCapability descriptor. Will call "cb" on
//		completion with pOriginalContext.
//
//		code		- RMSTR_CAPABILITY_RAID0, RMSTR_CAPABILITY_RAID1, etc
//		pFound		- true (capability found) else not found
//		pCapabilityDescriptor	- the capability record
//		cb						- callback on read complete
//		pOriginalContext		- any client context data
//
//************************************************************************
void RmstrCapabilities::
GetRmstrCapability(
		RMSTR_CAPABILITY_CODE		code,
		U32							*pFound,
		RMSTR_CAPABILITY_DESCRIPTOR *pCapabilityDescriptor,
		pTSCallback_t				cb,
		void						*pOriginalContext)
{
	CAPABILITY_CONTEXT				*pContext = new CAPABILITY_CONTEXT;

	pContext->pCallback = cb;
	pContext->pParentContext = pOriginalContext;
	pContext->value2 = code;
	pContext->pData1 = pCapabilityDescriptor;
	pContext->pData2 = pFound;

	m_pTableServices->TableServiceEnumTable(
						RMSTR_CAPABILITY_TABLE,			// tableName
						&pContext->pData,				// table data returned
						&pContext->value1,				// data returned size
						&pContext->value,				// number of rows returned here
						pContext,						// context
						(pTSCallback_t)&RmstrCapabilities::GetRmstrCapabilityReply);
}

//************************************************************************
//	GetRmstrCapabilityReply
//		Will read the capability according to the capability code and will
//		set the pFound and pCapability descriptor. Will call "cb" on
//		completion with pOriginalContext.
//
//************************************************************************
STATUS RmstrCapabilities::
GetRmstrCapabilityReply (void *_pContext, STATUS status)
{
	CAPABILITY_CONTEXT			*pContext = (CAPABILITY_CONTEXT *)_pContext;
	STATUS						rc = OK;
	void						*pCapabilityTableData = NULL;
	RMSTR_CAPABILITY_DESCRIPTOR	*pCapabilityRecord = NULL;	
	U32							i=0;
	U32							numberOfRows = 0;
	RMSTR_CAPABILITY_CODE		capCode;
	U32							*pFound = NULL;
	
	
	numberOfRows = pContext->value;
	capCode = (RMSTR_CAPABILITY_CODE)pContext->value2;
	pFound = (U32 *)pContext->pData2;

	*pFound = false;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		rc = 1;
	} else {
			pCapabilityTableData = pContext->pData;
			for (i=0; i < numberOfRows; i++){					
					pCapabilityRecord = (RMSTR_CAPABILITY_DESCRIPTOR *)
						((char *)pCapabilityTableData+(sizeof(RMSTR_CAPABILITY_DESCRIPTOR)*i));
					if (pCapabilityRecord){
						// check against code
						if (pCapabilityRecord->capabilityCode == capCode){
							memcpy(
								pContext->pData1,
								pCapabilityRecord,
								sizeof(RMSTR_CAPABILITY_DESCRIPTOR));
							*pFound = true;
							break;
						}
					}
			}
			if (numberOfRows){
				if (pContext->pData){
					delete pContext->pData;
					pContext->pData = NULL;
				}
			}
			pContext->value = 0;
			pContext->value1 = 0;

	}
	pTSCallback_t		cb = pContext->pCallback;
	(this->*cb)(
			pContext->pParentContext,	
			status);		
	pContext->pData1 = NULL;
	pContext->pData2 = NULL;
	delete pContext;
	return status;
}