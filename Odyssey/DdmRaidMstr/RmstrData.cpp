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
// File: RmstrData.cpp
// 
// Description:
// Data for display
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RmstrData.cpp $
// 
// 18    9/07/99 1:47p Dpatel
// Checked array and utility data during init to check PTS data
// consistency.
// 
// 17    9/03/99 10:01a Dpatel
// Remitting alarms...
// 
// 16    9/01/99 6:38p Dpatel
// added logging and alarm code..
// 
// 15    8/20/99 3:03p Dpatel
// added simulation for failover and failover code (CheckAnd...() methods)
// 
// 14    8/16/99 7:05p Dpatel
// Changes for alarms + using rowID * as handle instead of void*
// 
// 13    8/12/99 1:56p Dpatel
// Added Array offline event processing code.
// 
// 12    8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64. added simulation raid
// ddm.
// 
// 11    8/05/99 11:07a Dpatel
// internal delete array, fake raid ddm, hot copy auto break, removed
// array name code..
// 
// 10    8/02/99 3:01p Dpatel
// changes to create array, array FT processing...
// 
// 9     7/30/99 6:47p Dpatel
// Removed INSUFFICIENT CAP and INVALID STORAGE_ELEMENT
// 
// 8     7/30/99 6:40p Dpatel
// Change preferred member, processing member down and stop util as
// internal cmds..
// 
// 7     7/28/99 6:35p Dpatel
// Added capability code, table services, add/remove members, preferred
// member and source member, hot copy etc...
// 
// 6     7/23/99 5:47p Dpatel
// Added internal cmds, hotcopy, changed commit spare etc.
// 
// 5     7/17/99 1:20p Dpatel
// Queued up commands.
// 
// 4     7/16/99 10:28a Dpatel
// Added DownMember and Commit Spare code. Also removed the reads for
// validation.
// 
// 3     7/09/99 5:26p Dpatel
// 
// 2     6/28/99 5:16p Dpatel
// Implemented new methods, changed headers.
// 
//
// 06/11/99 Dipam Patel: Create file
//
/*************************************************************************/

#include "DdmRaidMgmt.h"

#pragma	pack(4)



char *dispCommandName[] =
{
	"INVALID CMD",
	"RMSTR_CMND_CREATE_ARRAY",
	"RMSTR_CMND_DELETE_ARRAY",
	"RMSTR_CMND_CREATE_SPARE",
	"RMSTR_CMND_DELETE_SPARE",
	"RMSTR_CMND_START_UTIL",
	"RMSTR_CMND_ABORT_UTIL",
	"RMSTR_CMND_CHANGE_UTIL_PRIORITY",
	"RMSTR_CMND_DOWN_A_MEMBER",
	"RMSTR_CMND_ADD_MEMBERS",
	"RMSTR_CMND_REMOVE_MEMBERS",
	"RMSTR_CMND_CHANGE_SOURCE_MEMBER",
	"RMSTR_CMND_CHANGE_PREFERRED_MEMBER",
	"RMSTR_CMND_LAST_VALID",
	"RMSTR_INTERNAL_CMND_START_UTIL",
	"RMSTR_INTERNAL_CMND_DELETE_ARRAY",
	"RMSTR_INTERNAL_CMND_COMMIT_SPARE",
	"RMSTR_INTERNAL_CMND_PROCESS_MEMBER_DOWN_EVENT",
	"RMSTR_INTERNAL_CMND_PROCESS_ARRAY_OFFLINE_EVENT",
	"RMSTR_INTERNAL_CMND_PROCESS_STOP_UTIL_EVENT",
	"RMSTR_INTERNAL_CMND_CHANGE_SOURCE_MEMBER"
};


char *dispUtilityName[] =
{
	"INVALID_UTIL",
	"RAID_UTIL_VERIFY",
	"RAID_UTIL_REGENERATE",
	"RAID_UTIL_LUN_HOTCOPY",
	"RAID_UTIL_MEMBER_HOTCOPY",
	"RAID_UTIL_BKGD_INIT",
	"RAID_UTIL_EXPAND",
	"LAST_UTIL_NAME"	
};


char *dispEventName[] =
{
	"INVALID_UTIL",
	"RMSTR_EVT_ARRAY_ADDED",
	"RMSTR_EVT_ARRAY_DELETED",
	"RMSTR_EVT_UTIL_STARTED",
	"RMSTR_EVT_UTIL_ABORTED",
	"RMSTR_EVT_UTIL_STOPPED",
	"RMSTR_EVT_UTIL_PRIORITY_CHANGED",
	"RMSTR_EVT_UTIL_PERCENT_COMPLETE",  // [ Not reported to System Log]
	"RMSTR_EVT_SPARE_ADDED",
	"RMSTR_EVT_SPARE_DELETED",
	"RMSTR_EVT_MEMBER_DOWN",
	"RMSTR_EVT_ARRAY_CRITICAL",
	"RMSTR_EVT_ARRAY_OFFLINE", 
	"RMSTR_EVT_ARRAY_FAULT_TOLERANT",
	"RMSTR_EVT_SPARE_ACTIVATED",
	"RMSTR_EVT_FLAKY_DRIVE",	
	"RMSTR_EVT_MEMBER_ADDED",
	"RMSTR_EVT_MEMBER_REMOVED",
	"RMSTR_EVT_SOURCE_MEMBER_CHANGED",
	"RMSTR_EVT_PREFERRED_MEMBER_CHANGED"
};



char *dispErrorName[] =
{
	"INVALID_UTIL",
	"RMSTR_ERR_RMSTR_NOT_INITIALIZED",
	"RMSTR_ERR_INSUFFICIENT_MEMBER_CAPACITY",
	"RMSTR_ERR_INSUFFICIENT_SPARE_CAPACITY",
	"RMSTR_ERR_STORAGE_ELEMENT_FAILED",
	"RMSTR_ERR_STORAGE_ELEMENT_IN_USE",
	"RMSTR_ERR_OS_PARTITION_EXISTS",
	"RMSTR_ERR_INVALID_COMMAND",
	"RMSTR_ERR_MEMBER_ALREADY_DOWN",
	"RMSTR_ERR_UTIL_RUNNING",
	"RMSTR_ERR_UTIL_NOT_SUPPORTED",
	"RMSTR_ERR_UTIL_ALREADY_RUNNING",	
	"RMSTR_ERR_UTIL_ABORT_NOT_ALLOWED",
	"RMSTR_ERR_UTIL_NOT_RUNNING",
	"RMSTR_ERR_INVALID_PRIORITY_LEVEL",


	"RMSTR_ERR_MAX_SPARES_ALREADY_CREATED",
	"RMSTR_ERR_SPARE_DOES_NOT_EXIST",

	"RMSTR_ERR_ARRAY_STATE_OFFLINE",
	"RMSTR_ERR_ARRAY_STATE_NOT_CRITICAL",
	"RMSTR_ERR_ARRAY_STATE_CRITICAL",
	"RMSTR_ERR_ARRAY_OFFLINE",
	"RMSTR_ERR_ARRAY_CRITICAL",
	"RMSTR_ERR_NAME_ALREADY_EXISTS",
	"RMSTR_ERR_MAX_ARRAY_MEMBERS",
	"RMSTR_ERR_INVALID_RAID_LEVEL"
};





enum {
	INITIALIZE_RMSTR_DATA_ADT_ENUMERATED = 200,
	INITIALIZE_RMSTR_DATA_MDT_ENUMERATED,
	INITIALIZE_RMSTR_DATA_SDT_ENUMERATED,
	INITIALIZE_RMSTR_DATA_UDT_ENUMERATED
};


//************************************************************************
//	InitializeRmstrData
//		Read the ADT Table and store all entries in our local data
//		Read the MDT Table and store all entries in our local data
//		Read the SDT Table and store all entries in our local data
//		Read the UDT Table and store all entries in our local data
//
//************************************************************************
void DdmRAIDMstr::
InitializeRmstrData()
{
	CONTEXT		*pContext= new CONTEXT;

	pContext->state		= INITIALIZE_RMSTR_DATA_ADT_ENUMERATED;

	m_pTableServices->TableServiceEnumTable(
				RAID_ARRAY_DESCRIPTOR_TABLE,	// tableName
				&pContext->pData,				// table data returned
				&pContext->value1,				// data returned size
				&pContext->value,				// number of rows returned here
				pContext,						// context
				(pTSCallback_t)&DdmRAIDMstr::InitializeRmstrDataReply);
}



//************************************************************************
//	InitializeRmstrDataReply
//		Process the enum replies for different tables.
//
//************************************************************************
STATUS DdmRAIDMstr::
InitializeRmstrDataReply(void *_pContext, STATUS status)
{
	CONTEXT							*pContext = NULL;	
	
	STATUS							rc;
	BOOL							initializeComplete = false;
	U32								numberOfRows = 0;
	void							*pADTTableData = NULL;
	void							*pMDTTableData = NULL;
	void							*pSDTTableData = NULL;
	void							*pUDTTableData = NULL;
	U32								i = 0;
	RAID_ARRAY_DESCRIPTOR			*pADTRecord = NULL;
	RAID_ARRAY_MEMBER				*pMember = NULL;
	RAID_SPARE_DESCRIPTOR			*pSpare = NULL;
	RAID_ARRAY_UTILITY				*pUtility = NULL;


	rc						= RMSTR_SUCCESS;
	pContext				= (CONTEXT *)_pContext;

	numberOfRows = pContext->value;
	if (status != OS_DETAIL_STATUS_SUCCESS){
		initializeComplete = true;
	} else {
		switch(pContext->state){
		case INITIALIZE_RMSTR_DATA_ADT_ENUMERATED:
			// pValidationContext->pData = enum data		
			// pData2 = NULL
			// pData3 = NULL
			// read the data that was enumerated
			pADTTableData = pContext->pData;
			for (i=0; i < numberOfRows; i++){
					pADTRecord = (RAID_ARRAY_DESCRIPTOR *)
						((char *)pADTTableData+(sizeof(RAID_ARRAY_DESCRIPTOR)*i));
					if (pADTRecord){
						AddRmstrData(
								RAID_ARRAY,
								&pADTRecord->thisRID,
								pADTRecord);
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

			pContext->state		= INITIALIZE_RMSTR_DATA_MDT_ENUMERATED;
			m_pTableServices->TableServiceEnumTable(
				RAID_MEMBER_DESCRIPTOR_TABLE,	// tableName
				&pContext->pData,				// table data returned
				&pContext->value1,				// data returned size
				&pContext->value,				// number of rows returned here
				pContext,						// context
				(pTSCallback_t)&DdmRAIDMstr::InitializeRmstrDataReply);

			break;

		case INITIALIZE_RMSTR_DATA_MDT_ENUMERATED:			
			pMDTTableData = pContext->pData;
			// read the data that was enumerated
			for (i=0; i < numberOfRows; i++){
					pMember = (RAID_ARRAY_MEMBER *)
						((char *)pMDTTableData+(sizeof(RAID_ARRAY_MEMBER)*i));
					if (pMember){
						AddRmstrData(
								RAID_MEMBER,
								&pMember->thisRID,
								pMember);
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

			pContext->state		= INITIALIZE_RMSTR_DATA_SDT_ENUMERATED;
			m_pTableServices->TableServiceEnumTable(
				RAID_SPARE_DESCRIPTOR_TABLE,	// tableName
				&pContext->pData,				// table data returned
				&pContext->value1,				// data returned size
				&pContext->value,				// number of rows returned here
				pContext,						// context
				(pTSCallback_t)&DdmRAIDMstr::InitializeRmstrDataReply);
			break;

		case INITIALIZE_RMSTR_DATA_SDT_ENUMERATED:
			pSDTTableData = pContext->pData;
			// read the data that was enumerated
			for (i=0; i < numberOfRows; i++){
					pSpare = (RAID_SPARE_DESCRIPTOR *)
						((char *)pSDTTableData+(sizeof(RAID_SPARE_DESCRIPTOR)*i));
					if (pSpare){
						AddRmstrData(
								RAID_SPARE,
								&pSpare->thisRID,
								pSpare);
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
			pContext->state		= INITIALIZE_RMSTR_DATA_UDT_ENUMERATED;
			m_pTableServices->TableServiceEnumTable(
				RAID_UTIL_DESCRIPTOR_TABLE,		// tableName
				&pContext->pData,				// table data returned
				&pContext->value1,				// data returned size
				&pContext->value,				// number of rows returned here
				pContext,						// context
				(pTSCallback_t)&DdmRAIDMstr::InitializeRmstrDataReply);
			break;

		case INITIALIZE_RMSTR_DATA_UDT_ENUMERATED:
			pUDTTableData = pContext->pData;
			// read the data that was enumerated
			for (i=0; i < numberOfRows; i++){
					pUtility = (RAID_ARRAY_UTILITY *)
						((char *)pUDTTableData+(sizeof(RAID_ARRAY_UTILITY)*i));
					if (pUtility){
						AddRmstrData(
								RAID_UTILITY,
								&pUtility->thisRID,
								pUtility);
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
			initializeComplete = true;
			break;

		default:
			assert(0);
		}
	}
	if (initializeComplete){
		delete pContext;
		pContext = NULL;
#ifdef RMSTR_RAID_DDM_TEST
		m_isQuiesced = false;
		RunOutstandingCommand();		
#else		
		// Check Data consistency, i.e verify that all tables
		// are in order. Also need to check PTS data with 
		// Reserved sector data if and when its available.
		CheckRmstrDataConsistency();
		// Now initialize the command queues
		InitializeCommandQueues();
#endif		
	}
	return status;
}


//************************************************************************
//	CheckRmstrDataConsistency
//
//************************************************************************
void DdmRAIDMstr::
CheckRmstrDataConsistency()
{
	CheckArrayData();
	CheckUtilityData();
}


//************************************************************************
//	CheckArrayData
//		Check for any down members to start regenerate or to commit spare
//		Resolve: Check for any offline arrays whose processing needs to be done
//
//************************************************************************
void DdmRAIDMstr::
CheckArrayData()
{
	RAID_ARRAY_DESCRIPTOR		*pADTRecord = NULL;
	RAID_ARRAY_MEMBER			*pMember = NULL;
	BOOL						isArrayOffline = false;
	U32							i = 0;

	TraverseRmstrData(
		RAID_ARRAY,
		NULL,
		(void **)&pADTRecord);
	while (pADTRecord){
		// First check if array is not offline
		isArrayOffline = CheckIfArrayOffline(pADTRecord);
		if (isArrayOffline){
			FindFirstDownMember(pADTRecord,(void **)&pMember);
			// Resolve: reason
			StartInternalProcessArrayOfflineEvent(
				&pADTRecord->thisRID, 
				&pMember->thisRID,
				0);
		} else {
			// Check all members of the array and find any down member,		
			// if down member was a good member that went down, start down member
			// processing (which will take care of committing spares if available)
			for (i=0; i < pADTRecord->numberMembers; i++){
				pMember = NULL;
				GetRmstrData(
					RAID_MEMBER,
					&pADTRecord->members[i],
					(void **)&pMember);
				if (pMember){
					if (pMember->memberHealth == RAID_STATUS_DOWN){
						// Resolve: Check if member used to be a spare that was committed
						//if (pMember->previouslyASpare){
						if (0){
							// down as spare was recently committed
							StartInternalProcessMemberDownEvent(
								&pADTRecord->thisRID,
								&pMember->thisRID,
								0);
						} else {
							StartInternalRegenerate(pADTRecord);
						}
					}
				}
			}
		}
		// check other arrays
		TraverseRmstrData(
			RAID_ARRAY,
			&pADTRecord->thisRID,
			(void **)&pADTRecord);
	}
}


//************************************************************************
//	CheckUtilityData
//
//************************************************************************
void DdmRAIDMstr::
CheckUtilityData()
{
	RAID_ARRAY_UTILITY	 *pUtility = NULL;

	TraverseRmstrData(
		RAID_UTILITY,
		NULL,
		(void **)&pUtility);
	while (pUtility){
		switch (pUtility->status){
		case RAID_UTIL_ABORTED:
		case RAID_UTIL_ABORTED_IOERROR:
		case RAID_UTIL_COMPLETE:
			// if util was aborted for any reason, it should not be
			// in our tables. Resolve: miscompare cnt
			StartInternalProcessStopUtilEvent(
						&pUtility->thisRID,
						0, //pUtility->MiscompareCnt,
						pUtility->status);
		}
		// check other utils
		TraverseRmstrData(
			RAID_UTILITY,
			&pUtility->thisRID,
			(void **)&pUtility);
	}
}



//************************************************************************
//	AddRmstrData
//		Add the pData to the appropriate queue for the type. There
//		are 5 separate queues maintained
//
//	type		- RAID_ARRAY/MEMBER/SPARE/UTIL or RAID_CMND
//	pRowId		- Row Id in one of the tables
//	pData		- Row Data
//
//************************************************************************
void DdmRAIDMstr::
AddRmstrData(U32 type, rowID *pRowId, void *pData)
{
	RMSTR_DATA		*pRmstrData = new(tZERO) RMSTR_DATA;
	U32				sizeofData = 0;
	RMSTR_DATA		*pHeadAddress = NULL;
	RMSTR_DATA		*pTailAddress = NULL;
	RMSTR_DATA		**ppHead = NULL;
	RMSTR_DATA		**ppTail = NULL;
	
	BOOL			rowAlreadyExists = false;

	rowAlreadyExists = 
		CheckIfDataAlreadyInserted(type, pRowId);
	if (rowAlreadyExists){
	} else {
		pRmstrData->type = type,
		memcpy(&pRmstrData->rowId, pRowId, sizeof(rowID));

		PrepareRmstrData(
				type, 
				pRmstrData, 
				&sizeofData, 
				&pHeadAddress,
				&pTailAddress);

		ppHead = (RMSTR_DATA **)pHeadAddress;
		ppTail = (RMSTR_DATA **)pTailAddress;

		memcpy(pRmstrData->pRowData, pData, sizeofData);
		// check if first node
		if ((*ppHead == NULL) && (*ppTail == NULL)){
			// add node
			*ppHead = pRmstrData;
			*ppTail = pRmstrData;
			pRmstrData->pNext = NULL;
		} else {
			// add node to tail
			(*ppTail)->pNext = pRmstrData;
			pRmstrData->pNext = NULL;
			*ppTail = pRmstrData;
		}
	}
}

//************************************************************************
//	AddRmstrDataToHead
//		Add the pData (at Head) to the appropriate queue for the type. There
//		are 5 separate queues maintained
//
//	type		- RAID_ARRAY/MEMBER/SPARE/UTIL or RAID_CMND
//	pRowId		- Row Id in one of the tables
//	pData		- Row Data
//
//************************************************************************
void DdmRAIDMstr::
AddRmstrDataToHead(U32 type, rowID *pRowId, void *pData)
{
	RMSTR_DATA		*pRmstrData = new(tZERO) RMSTR_DATA;
	U32				sizeofData = 0;
	RMSTR_DATA		*pHeadAddress = NULL;
	RMSTR_DATA		*pTailAddress = NULL;
	RMSTR_DATA		**ppHead = NULL;
	RMSTR_DATA		**ppTail = NULL;
	

	pRmstrData->type = type,
	memcpy(&pRmstrData->rowId, pRowId, sizeof(rowID));

	// Get the pointer, size etc. to correct type of Head /Tail
	PrepareRmstrData(
			type, 
			pRmstrData, 
			&sizeofData, 
			&pHeadAddress,
			&pTailAddress);

	// we need to chg the head and tail so get the address of head/tail (ptr to ptr)
	ppHead = (RMSTR_DATA **)pHeadAddress;
	ppTail = (RMSTR_DATA **)pTailAddress;

	memcpy(pRmstrData->pRowData, pData, sizeofData);

	// check if first node
	if ((*ppHead == NULL) && (*ppTail == NULL)){
		// add node
		*ppHead = pRmstrData;
		*ppTail = pRmstrData;
		pRmstrData->pNext = NULL;
	} else {
		// add node to head
		pRmstrData->pNext = *ppHead;
		*ppHead = pRmstrData;
	}
}


//************************************************************************
//	ModifyRmstrData
//		Modify an existing record by removing and adding
//
//	type		- RAID_ARRAY/MEMBER/SPARE/UTIL or RAID_CMND
//	pRowId		- Row Id in one of the tables
//	pData		- Row Data
//
//************************************************************************
void DdmRAIDMstr::
ModifyRmstrData(
			U32		type,
			rowID	*pRowId,
			void	*pData)
{
	RemoveRmstrData(type, pRowId);
	AddRmstrData(type,pRowId,pData);
}

//************************************************************************
//	GetRmstrData
//		Retrieve an existing data record by row id and type
//
//	type			- RAID_ARRAY/MEMBER/SPARE/UTIL or RAID_CMND
//	pRowId			- Row Id in one of the tables
//	ppRmstrDataRet	- Row Data returned at this address
//
//************************************************************************
void DdmRAIDMstr::
GetRmstrData(U32 type, rowID *pRowId, void **ppRmstrDataRet)
{
	RMSTR_DATA				*pTemp = NULL;

	RMSTR_DATA				*pDataHeadAddress = NULL;
	RMSTR_DATA				*pDataTailAddress = NULL;
	RMSTR_DATA				**ppHead = NULL;
	RMSTR_DATA				**ppTail = NULL;
	RMSTR_DATA				*pDataHead = NULL;
	RMSTR_DATA				*pDataTail = NULL;

	U32						sizeofData = 0;

	PrepareRmstrData(
			type, 
			NULL, 
			&sizeofData, 
			&pDataHeadAddress,
			&pDataTailAddress);

	// first get the address of head/tail (&m_pArrayData etc)
	ppHead = (RMSTR_DATA **)pDataHeadAddress;
	ppTail = (RMSTR_DATA **)pDataTailAddress;

	// Get the ptr to the head tail (m_pArrayData etc)
	pDataHead = *ppHead;
	pDataTail = *ppTail;

	*ppRmstrDataRet = NULL;
	pTemp = pDataHead;
	while (pTemp != NULL){
		if (memcmp(&pTemp->rowId, pRowId, sizeof(rowID)) == 0){
			//found a match
			*ppRmstrDataRet = pTemp->pRowData;
			break;
		}
		pTemp = pTemp->pNext;
	}
}

//************************************************************************
//	TraverseRmstrData
//		Traverse the rmstr data starting at a particular row id
//
//	type		- RAID_ARRAY/MEMBER/SPARE/UTIL or RAID_CMND
//	pRowID		- whose next entry will be returned(
//						(NULL to get first entry)
//	ppRmstrDataRet	- Row Data returned at this address, set to NULL if 
//						end of list reached.
//
//************************************************************************
void DdmRAIDMstr::
TraverseRmstrData(
		U32			type,
		rowID		*pRowId,
		void		**ppRmstrDataRet)
{
	RMSTR_DATA				*pTemp = NULL;
	RMSTR_DATA				*pDataHeadAddress = NULL;
	RMSTR_DATA				*pDataTailAddress = NULL;
	RMSTR_DATA				**ppHead = NULL;
	RMSTR_DATA				**ppTail = NULL;
	RMSTR_DATA				*pDataHead = NULL;
	RMSTR_DATA				*pDataTail = NULL;

	U32						sizeofData = 0;

	PrepareRmstrData(
			type, 
			NULL, 
			&sizeofData, 
			&pDataHeadAddress,
			&pDataTailAddress);

	ppHead = (RMSTR_DATA **)pDataHeadAddress;
	ppTail = (RMSTR_DATA **)pDataTailAddress;

	pDataHead = *ppHead;
	pDataTail = *ppTail;

	pTemp = pDataHead;
	if (pTemp == NULL){
		// head is null, so no entries left
		*ppRmstrDataRet = NULL;
		return;
	}

	if (pRowId) {
		while (pTemp != NULL){
			if (memcmp(&pTemp->rowId, pRowId, sizeof(rowID)) == 0){
				//found a match, return next entry
				if (pTemp->pNext){
					*ppRmstrDataRet = pTemp->pNext->pRowData;
				} else {
					// end of list
					*ppRmstrDataRet = NULL;
				}
				break;
			}
			pTemp = pTemp->pNext;
		}
	} else {
		// return first entry
		if (pTemp){
			*ppRmstrDataRet = pTemp->pRowData;
		} else {
			*ppRmstrDataRet = NULL;
		}
	}
}


//************************************************************************
//	RemoveRmstrData
//		Remove an existing data record by row id and type
//
//	type		- RAID_ARRAY/MEMBER/SPARE/UTIL or RAID_CMND
//	pRowID		- row id to remove
//
//************************************************************************
void DdmRAIDMstr
::RemoveRmstrData(U32 type, rowID *pRowId)
{
	RMSTR_DATA			*pTemp = NULL;

	U32				sizeofData = 0;
	RMSTR_DATA		*pHeadAddress = NULL;
	RMSTR_DATA		*pTailAddress = NULL;
	RMSTR_DATA		**ppDataHead = NULL;
	RMSTR_DATA		**ppDataTail = NULL;
	

	PrepareRmstrData(
			type, 
			NULL, 
			&sizeofData, 
			&pHeadAddress,
			&pTailAddress);

	ppDataHead = (RMSTR_DATA **)pHeadAddress;
	ppDataTail = (RMSTR_DATA **)pTailAddress;


	if (*ppDataHead == NULL){
		return;
	}

	// check first entry
	if (memcmp(&(*ppDataHead)->rowId, pRowId, sizeof(rowID)) == 0){
		if ((*ppDataHead)->pRowData){
			delete (*ppDataHead)->pRowData;
		}
		pTemp = *ppDataHead;
		*ppDataHead = (*ppDataHead)->pNext;
		if (*ppDataHead == NULL){
			// if it happens to be last entry
			*ppDataTail = NULL;
		}
		delete pTemp;
		return;
	}
	pTemp					= *ppDataHead;
	RMSTR_DATA *pNextNode	= pTemp->pNext;
	while (pTemp->pNext != NULL){
		if (memcmp(&(pNextNode->rowId),pRowId, sizeof(rowID)) == 0){
			//found a match
			pTemp->pNext = pNextNode->pNext;
			if (pTemp->pNext == NULL){
				*ppDataTail = pTemp;
			}
			if (pNextNode->pRowData){
				delete pNextNode->pRowData;
			}
			delete pNextNode;
			return;
		}
		pTemp = pNextNode;
		pNextNode = pTemp->pNext;
	}
}



//************************************************************************
//	PrepareRmstrData
//		Get the address of the pointer to head and tail for each
//		data type. Also set the size of the data record.
//
//	type		- RAID_ARRAY/MEMBER/SPARE/UTIL or RAID_CMND
//	pRmstrData	- if non NULL, allocates the data record (pRowData)
//	pSizeofData	- sizeof data is set at this address
//	ppHead		- address of data head ptr is returned here (&m_pArrayData)
//	ppTail		- address of data tail ptr is returned here (&m_pArrayDataTail)
//
//************************************************************************
void DdmRAIDMstr::
PrepareRmstrData(
		U32				type, 
		RMSTR_DATA		*pRmstrData, 
		U32				*pSizeofData,
		RMSTR_DATA		**ppHead, 
		RMSTR_DATA		**ppTail)
{
	switch(type) {
	case RAID_ARRAY:
		if (pRmstrData)
			pRmstrData->pRowData = new(tZERO) RAID_ARRAY_DESCRIPTOR;
		*pSizeofData = sizeof(RAID_ARRAY_DESCRIPTOR);
		*ppHead = (RMSTR_DATA *)&m_pArrayData;
		*ppTail = (RMSTR_DATA *)&m_pArrayDataTail;
		break;
	case RAID_MEMBER:
		if (pRmstrData)
			pRmstrData->pRowData = new(tZERO) RAID_ARRAY_MEMBER;
		*pSizeofData = sizeof(RAID_ARRAY_MEMBER);
		*ppHead = (RMSTR_DATA *)&m_pMemberData;
		*ppTail = (RMSTR_DATA *)&m_pMemberDataTail;
		break;
	case RAID_SPARE:
		if (pRmstrData)
			pRmstrData->pRowData = new(tZERO) RAID_SPARE_DESCRIPTOR;
		*pSizeofData = sizeof(RAID_SPARE_DESCRIPTOR);
		*ppHead = (RMSTR_DATA *)&m_pSpareData;
		*ppTail = (RMSTR_DATA *)&m_pSpareDataTail;
		break;
	case RAID_UTILITY:
		if (pRmstrData)
			pRmstrData->pRowData = new(tZERO) RAID_ARRAY_UTILITY;
		*pSizeofData = sizeof(RAID_ARRAY_UTILITY);
		*ppHead = (RMSTR_DATA *)&m_pUtilityData;
		*ppTail = (RMSTR_DATA *)&m_pUtilityDataTail;
		break;
	case RAID_CMND:
		if (pRmstrData)
			pRmstrData->pRowData = new(tZERO) RMSTR_QUEUED_CMND;
		*pSizeofData = sizeof(RMSTR_QUEUED_CMND);
		*ppHead = (RMSTR_DATA *)&m_pCmdData;
		*ppTail = (RMSTR_DATA *)&m_pCmdDataTail;
		break;
	case RAID_ALARM:
		if (pRmstrData)
			pRmstrData->pRowData = new(tZERO) ALARM_CONTEXT;
		*pSizeofData = sizeof(ALARM_CONTEXT);
		*ppHead = (RMSTR_DATA *)&m_pAlarmData;
		*ppTail = (RMSTR_DATA *)&m_pAlarmDataTail;
		break;

	default:
		assert(0);
	}
}


//************************************************************************
//	CleanRmstrData
//		Remove all the allocated data, from our internal queues.
//
//************************************************************************
void DdmRAIDMstr
::CleanRmstrData()
{
	CleanData(RAID_ARRAY);
	CleanData(RAID_MEMBER);
	CleanData(RAID_SPARE);
	CleanData(RAID_UTILITY);
	CleanData(RAID_CMND);
	CleanData(RAID_ALARM);
}


//************************************************************************
//	CleanData
//		Remove all the allocated data, from our internal queues
//		for a particular type
//
//************************************************************************
void DdmRAIDMstr
::CleanData(U32 type)
{
	void						*pRowData = NULL;
	rowID						rowId;
	RAID_ARRAY_DESCRIPTOR		*pADTRecord = NULL;
	RAID_SPARE_DESCRIPTOR		*pSpare = NULL;
	RAID_ARRAY_MEMBER			*pMember = NULL;
	RAID_ARRAY_UTILITY			*pUtility = NULL;
	RMSTR_QUEUED_CMND			*pRmstrQueuedCmnd = NULL;
	ALARM_CONTEXT				*pAlarmContext = NULL;

	TraverseRmstrData(
			type,
			NULL,
			(void **)&pRowData);
	while(pRowData){
		switch(type){
			case RAID_ARRAY:
				pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pRowData;
				rowId = pADTRecord->thisRID;
				break;
			case RAID_MEMBER:
				pMember = (RAID_ARRAY_MEMBER *)pRowData;
				rowId = pMember->thisRID;
				break;
			case RAID_SPARE:
				pSpare = (RAID_SPARE_DESCRIPTOR *)pRowData;
				rowId = pSpare->thisRID;
				break;
			case RAID_UTILITY:
				pUtility = (RAID_ARRAY_UTILITY *)pRowData;
				rowId = pUtility->thisRID;
				break;
			case RAID_CMND:
				pRmstrQueuedCmnd = (RMSTR_QUEUED_CMND *)pRowData;
				rowId = pRmstrQueuedCmnd->rowId;
				break;
			case RAID_ALARM:
				pAlarmContext = (ALARM_CONTEXT *)pRowData;
				rowId = pAlarmContext->alarmSourceRowId;
				break;
			default:
				assert(0);
		}
		// Now delete the entry 
		RemoveRmstrData(
			type,
			&rowId);

		// Check if the next entry exists, if present get it
		TraverseRmstrData(
			type,
			NULL,
			(void **)&pRowData);

	}
}




BOOL DdmRAIDMstr
::CheckIfDataAlreadyInserted(
				U32					type,
				rowID				*pRowId)
{
	void						*pRowData = NULL;
	RAID_ARRAY_DESCRIPTOR		*pADTRecord = NULL;
	RAID_SPARE_DESCRIPTOR		*pSpare = NULL;
	RAID_ARRAY_MEMBER			*pMember = NULL;
	RAID_ARRAY_UTILITY			*pUtility = NULL;
	rowID						rowId;
	BOOL						alreadyExists = false;

	TraverseRmstrData(
			type,
			NULL,
			(void **)&pRowData);
	while(pRowData){
		switch(type){
			case RAID_ARRAY:
				pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pRowData;
				rowId = pADTRecord->thisRID;
				break;

			case RAID_MEMBER:
				pMember = (RAID_ARRAY_MEMBER *)pRowData;
				rowId = pMember->thisRID;
				break;

			case RAID_SPARE:
				pSpare = (RAID_SPARE_DESCRIPTOR *)pRowData;
				rowId = pSpare->thisRID;
				break;

			case RAID_UTILITY:
				pUtility = (RAID_ARRAY_UTILITY *)pRowData;
				rowId = pUtility->thisRID;
				break;

			case RAID_CMND:
				// no check required
				return alreadyExists;

			case RAID_ALARM:
				// no check required
				return alreadyExists;

			default:
				assert(0);
		}

		// Check if the same row id exists
		if (memcmp(&rowId, pRowId, sizeof(rowID)) == 0){
			alreadyExists = true;
		}

		if (alreadyExists){
			break;
		} else {
			// Check if the next entry exists, if present get it
			TraverseRmstrData(
				type,
				&rowId,
				(void **)&pRowData);
		}
	}
	return alreadyExists;
}





