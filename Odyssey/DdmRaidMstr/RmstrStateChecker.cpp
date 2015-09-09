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
//	Implementation to determine if a particular state has already been
//	processed or not.
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RmstrStateChecker.cpp $
// 
// 2     8/24/99 5:13p Dpatel
// added failover code, create array changes for "array name"
// 
// 1     8/20/99 3:02p Dpatel
// Initial creation.
// 
//
/*************************************************************************/

#include "DdmRaidMgmt.h"


//************************************************************************
//	CheckIfStateAlreadyProcessed
//		Checks all the existing entries in PTS for a state that
//		matches or exceeds the state defined in pStateIdentifier.
//		If the state is already processed, then the existing data from
//		PTS is copied to pBuffer.
//		4 types of tables are handled,
//			RAID_ARRAY_DESCRIPTOR_TABLE
//			RAID_MEMBER_DESCRIPTOR_TABLE
//			RAID_SPARE_DESCRIPTOR_TABLE
//			RAID_UTILITY_DESCRIPTOR_TABLE
//
//************************************************************************
BOOL DdmRAIDMstr
::CheckIfStateAlreadyProcessed(
				U32					type,
				STATE_IDENTIFIER	*pStateIdentifier,
				void				*pBuffer)
{
	void						*pRowData = NULL;
	RAID_ARRAY_DESCRIPTOR		*pADTRecord = NULL;
	RAID_SPARE_DESCRIPTOR		*pSpare = NULL;
	RAID_ARRAY_MEMBER			*pMember = NULL;
	RAID_ARRAY_UTILITY			*pUtility = NULL;
	U32							rowSize = 0;
	rowID						cmdRowId;
	U32							opcode = 0;
	U32							state = 0;
	BOOL						insertDone = false;
	U32							index = 0;
	rowID						currentRowId;


	TraverseRmstrData(
			type,
			NULL,
			(void **)&pRowData);
	while(pRowData){
		switch(type){
			case RAID_ARRAY:
				pADTRecord = (RAID_ARRAY_DESCRIPTOR *)pRowData;
				currentRowId = pADTRecord->thisRID;
				rowSize = sizeof(RAID_ARRAY_DESCRIPTOR);

				cmdRowId = pADTRecord->stateIdentifier.cmdRowId;
				opcode = pADTRecord->stateIdentifier.cmdOpcode;
				state = pADTRecord->stateIdentifier.cmdState;
				index = pADTRecord->stateIdentifier.index;
				break;

			case RAID_MEMBER:
				pMember = (RAID_ARRAY_MEMBER *)pRowData;
				rowSize = sizeof(RAID_ARRAY_MEMBER);
				currentRowId = pMember->thisRID;

				cmdRowId = pMember->stateIdentifier.cmdRowId;
				opcode = pMember->stateIdentifier.cmdOpcode;
				state = pMember->stateIdentifier.cmdState;
				index = pMember->stateIdentifier.index;
				break;

			case RAID_SPARE:
				pSpare = (RAID_SPARE_DESCRIPTOR *)pRowData;
				rowSize = sizeof(RAID_SPARE_DESCRIPTOR);
				currentRowId = pSpare->thisRID;

				cmdRowId = pSpare->stateIdentifier.cmdRowId;
				opcode = pSpare->stateIdentifier.cmdOpcode;
				state = pSpare->stateIdentifier.cmdState;
				index = pSpare->stateIdentifier.index;
				break;

			case RAID_UTILITY:
				pUtility = (RAID_ARRAY_UTILITY *)pRowData;
				rowSize = sizeof(RAID_ARRAY_UTILITY);
				currentRowId = pUtility->thisRID;

				cmdRowId = pUtility->stateIdentifier.cmdRowId;
				opcode = pUtility->stateIdentifier.cmdOpcode;
				state = pUtility->stateIdentifier.cmdState;
				index = pUtility->stateIdentifier.index;
				break;

			default:
				assert(0);
		}

		// Check if the same cmd
		if (memcmp(&pStateIdentifier->cmdRowId, &cmdRowId, sizeof(rowID)) == 0){
			// if same opcode
			if (opcode == pStateIdentifier->cmdOpcode){
				// if same state or greate state
				if (state >= pStateIdentifier->cmdState){
					if (index >= pStateIdentifier->index){
						insertDone = true;
						memcpy(pBuffer, pRowData, rowSize);
					}
				}
			}
		}

		if (insertDone){
			break;
		} else {
			// Check if the next entry exists, if present get it
			TraverseRmstrData(
				type,
				&currentRowId,
				(void **)&pRowData);
		}
	}
	return insertDone;
}



//************************************************************************
//		Modify a row only if the state has not been processed
//		
//
//************************************************************************
STATUS DdmRAIDMstr
::CheckAndModifyRow(
			U32					type,
			STATE_IDENTIFIER	*pStateIdentifier,
			String64			tableName,
			rowID				*pRowToModify,
			void				*pBuffer,
			U32					sizeofData,
			rowID				*pNewRowIdRet,
			pTSCallback_t		cb,
			void				*pOriginalContext)
{
	STATUS							status;

	// if state was already processed, pUtil should have UDT_INSERTED state or greater
	BOOL stateProcessed = CheckIfStateAlreadyProcessed(
				type,
				pStateIdentifier,
				pBuffer);
	if (stateProcessed){
		status = (this->*cb)(pOriginalContext, OK);
	} else {
		status = m_pTableServices->TableServiceModifyRow(
						tableName,
						pRowToModify,	// row id to modify
						pBuffer,
						sizeofData,
						pRowToModify,
						cb,
						pOriginalContext);
	}
	return status;
}




//************************************************************************
//		Insert a row only if the state has not been processed
//		
//
//************************************************************************
STATUS DdmRAIDMstr
::CheckAndInsertRow(
			U32					type,
			STATE_IDENTIFIER	*pStateIdentifier,
			String64			tableName,
			void				*pBuffer,
			U32					sizeofData,
			rowID				*pNewRowIdRet,
			pTSCallback_t		cb,
			void				*pOriginalContext)
{
	STATUS							status;


	// if state was already processed, pBuffer will contain
	// the last data in PTS (for that type)
	BOOL stateProcessed = CheckIfStateAlreadyProcessed(
				type,
				pStateIdentifier,
				pBuffer);
	if (stateProcessed){
		status = (this->*cb)(pOriginalContext, OK);
	} else {
		status = m_pTableServices->TableServiceInsertRow(
					tableName,
					pBuffer,
					sizeofData,
					(rowID *)pBuffer,		// to fill the row id
					cb,
					pOriginalContext);
	}
	return status;
}


//************************************************************************
//	CheckIfStateAlreadyProcessed
//
//************************************************************************
void DdmRAIDMstr
::SetStateIdentifier(
		STATE_IDENTIFIER	*pStateIdentifier,
		U32					opcode,
		rowID				*pCmdRowId,
		U32					state,
		U32					index)
{
	pStateIdentifier->cmdOpcode = opcode;
	pStateIdentifier->cmdRowId = *pCmdRowId;
	pStateIdentifier->cmdState = state;
	pStateIdentifier->index = index;
}



//************************************************************************
//		Simulate Failover
//		
//
//************************************************************************
BOOL DdmRAIDMstr
::SimulateFailover(CONTEXT *pCmdContext)
{
	// if failover was not in progress
	if (m_failoverInProgress == false){
		StopCommandProcessing(START_FAILOVER_SIMULATION, pCmdContext->cmdHandle);
		if (pCmdContext) {
			delete pCmdContext;
			pCmdContext = NULL;
		}
		return true;
	}
	return false;
}
