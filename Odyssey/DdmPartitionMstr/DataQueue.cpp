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
// File: DataQueue.cpp
// 
// Description:
//	Provides an object for storing any of our local data
// 
//
/*************************************************************************/

#include "DataQueue.h"


//************************************************************************
//	CONSTRUCTOR
//************************************************************************
DataQueue::DataQueue(DdmServices *pParentDdm, TableServices *pTableServices)	
				: DdmServices( pParentDdm )
{
	m_pParentDdm = pParentDdm;
	m_pTableServices = pTableServices;
	m_pHead = NULL;
	m_pTail = NULL;	
}

//************************************************************************
//	DESTRUCTOR
//
//************************************************************************
DataQueue::~DataQueue()
{
}


//************************************************************************
//	Add Data to data queue
//
//************************************************************************
STATUS DataQueue
::Add(
	  U32							type, 
	  rowID							*pRowId, 
	  PARTITION_STATE_IDENTIFIER	*pStateIdentifier,
	  void							*pData, 
	  U32							sizeofData, 
	  BOOL							addToTail)
{
	assert(sizeofData);
	
	// Check if our local copy already has the data, if it does
	// then dont add again..
	BOOL stateProcessed = CheckIfStateAlreadyProcessed(
				type,
				pStateIdentifier,
				NULL);
	if (stateProcessed){
		return stateProcessed;
	}



	STATUS			status = 0;

	DataObject *pObj = new DataObject(
								type, 
								pRowId, 
								pStateIdentifier,
								pData, 
								sizeofData);

	// check if first node
	if ((m_pHead == NULL) && (m_pTail == NULL)){
		// add node
		m_pHead = pObj;
		m_pTail = pObj;
		pObj->pNext = NULL;
	} else {
		if (addToTail) {
			// add node to tail
			(m_pTail)->pNext = pObj;
			pObj->pNext = NULL;
			m_pTail = pObj;
		} else {
			// add node to head
			pObj->pNext = m_pHead;
			m_pHead = pObj;
		}
	}
	return status;
}


//************************************************************************
//	Get Data
//
//************************************************************************
STATUS DataQueue
::Get(U32 type, rowID *pRowId, void **ppDataRet, U32 sizeofData)
{
	STATUS		status = 0;
	DataObject	*pTemp = NULL;
	BOOL		found = false;

	assert(pRowId);

	pTemp = GetDataObject(type, pRowId);
	if (pTemp){
		*ppDataRet = pTemp->pData;
		return true;
	} else {
		return false;
	}
}



//************************************************************************
//	Modify Data 
//
//************************************************************************
STATUS DataQueue
::Modify(
	  U32							type, 
	  rowID							*pRowId, 
	  PARTITION_STATE_IDENTIFIER	*pStateIdentifier,
	  void							*pData, 
	  U32							sizeofData, 
	  BOOL							addToTail)
{

	DataObject	*pTemp = NULL;

	pTemp = GetDataObject(type, pRowId);
	if (pTemp){
		pTemp->stateIdentifier = *pStateIdentifier;
		memcpy(pTemp->pData, pData, sizeofData);
		return true;
	} else {
		return false;
	}
}


//************************************************************************
//	Traverse Data
//
//************************************************************************
STATUS DataQueue
::Traverse(
		U32							type,
		rowID						*pRowId, 
		void						**ppDataRet,
		rowID						**ppRowIdRet,
		PARTITION_STATE_IDENTIFIER	**ppStateIdentifierRet)
{
	STATUS		status = 0;
	DataObject	*pTemp = NULL;
	DataObject	*pNextEntry = NULL;
	BOOL		found = false;

	if (m_pHead == NULL){
		// head is null, so no entries left
		*ppDataRet = NULL;
		*ppRowIdRet = NULL;
		if(ppStateIdentifierRet)
			*ppStateIdentifierRet = NULL;
		return !OK;
	}

	pTemp = m_pHead;
	while (pTemp != NULL){
		if (pTemp->type == type) {
			if (pRowId){
				if (pTemp->rowId == *pRowId){
					//found a match, return next entry of same type
					if (pTemp->pNext){
						pNextEntry = pTemp->pNext;
						while (pNextEntry){
							if (pNextEntry->type == type){
								*ppDataRet = pNextEntry->pData;
								*ppRowIdRet = &pNextEntry->rowId;
								if (ppStateIdentifierRet)
									*ppStateIdentifierRet = &pNextEntry->stateIdentifier;
								found = true;
								break;
							}
							pNextEntry = pNextEntry->pNext;
						}
					} 
					if (!found) {
						// end of list
						*ppDataRet = NULL;
						*ppRowIdRet = NULL;
						if(ppStateIdentifierRet)
							*ppStateIdentifierRet = NULL;
					}
					break;
				}
			} else {
				// return first entry
				*ppDataRet = pTemp->pData;
				*ppRowIdRet = &pTemp->rowId;
				if (ppStateIdentifierRet)
					*ppStateIdentifierRet = &pTemp->stateIdentifier;
				break;
			}
			pTemp = pTemp->pNext;
		} else {
			pTemp = pTemp->pNext;
		}
	} 
	return status;
}



//************************************************************************
//	Remove Data
//
//************************************************************************
STATUS DataQueue
::Remove(
		U32				type,
		rowID			*pRowId)
{
	assert (pRowId);

	STATUS			status = 0;	
	DataObject		*pTemp	= NULL;

	if (m_pHead == NULL){
		return !OK;
	}

	// check first entry
	pTemp = m_pHead;
	if (pTemp->type == type) {
		if (pTemp->rowId == *pRowId){		
			m_pHead = pTemp->pNext;
			if (m_pHead == NULL){
				// if it happens to be last entry
				m_pTail = NULL;
			}
			delete pTemp;
			return status;
		}
	}

	// check remaining entries
	pTemp					= m_pHead;
	DataObject *pNextNode	= pTemp->pNext;

	while (pNextNode != NULL){
		if (pNextNode->type == type) {
			if (pNextNode->rowId == *pRowId){
				pTemp->pNext = pNextNode->pNext;
				if (pTemp->pNext == NULL){
					m_pTail = pTemp;
				}
				delete pNextNode;
				return status;
			}
		}
		pTemp = pNextNode;
		pNextNode = pTemp->pNext;
	} 
	return status;
}

//************************************************************************
//	Checks if a particular state of our state machine is already
//	processed or not.
//
//************************************************************************
STATUS DataQueue::
CheckIfStateAlreadyProcessed(
			U32							type,
			PARTITION_STATE_IDENTIFIER	*pStateToBeChecked,
			void						*pBuffer)
{
	void						*pRowData = NULL;
	rowID						*pRowIdRet = NULL;
	PARTITION_STATE_IDENTIFIER	*pStateIdentifier = NULL;
	BOOL						insertDone = false;
	DataObject					*pDataObject = NULL;


	Traverse(
			type,
			NULL,
			(void **)&pRowData,
			&pRowIdRet,
			&pStateIdentifier);

	while(pRowData){
		if (pStateIdentifier->cmdRowId == pStateToBeChecked->cmdRowId){
			// if same opcode
			if (pStateToBeChecked->opcode == pStateIdentifier->opcode){
				// if same or less state then index has to be same
				if (pStateToBeChecked->state <= pStateIdentifier->state){
					if (pStateToBeChecked->index == pStateIdentifier->index){
						insertDone = true;
					}
				}
			}
		}

		if (insertDone){
			if (pBuffer){
					pDataObject = GetDataObject(type, pRowIdRet);
					memcpy(pBuffer, pRowData, pDataObject->sizeofData);
			}

			break;
		} else {
			// Check if the next entry exists, if present get it
			Traverse( 
				type,
				pRowIdRet,				
				(void **)&pRowData,
				&pRowIdRet,
				&pStateIdentifier);
		}
	}
	return insertDone;
}


//************************************************************************
//	PRIVATE:
//	GetDataObject
//
//************************************************************************
DataQueue::DataObject *DataQueue::
GetDataObject(
		U32				type,
		rowID			*pRowId)
{
	assert (pRowId);
	STATUS		status = 0;
	DataObject	*pTemp = NULL;

	if (m_pHead == NULL){
		// head is null, so no entries left
		return pTemp;
	}

	pTemp = m_pHead;
	while (pTemp != NULL){
		if (pTemp->type == type) {
			if (RowId(pTemp->rowId) == RowId(*pRowId)){
				return pTemp;
			}		
		}
		pTemp = pTemp->pNext;
	} 
	return NULL;

}





//************************************************************************
//		Insert a row only if the state has not been processed
//		
//
//************************************************************************
STATUS DataQueue
::CheckAndInsertRow(
			U32							type,
			PARTITION_STATE_IDENTIFIER	*pStateIdentifier,
			String64					tableName,
			void						*pBuffer,
			U32							sizeofData,
			rowID						*pNewRowIdRet,
			pTSCallback_t				cb,
			void						*pOriginalContext)
{
	STATUS							status;


	// if state was already processed, pBuffer will contain
	// the last data in PTS (for that type)
	BOOL stateProcessed = CheckIfStateAlreadyProcessed(
				type,
				pStateIdentifier,
				pBuffer);
	if (stateProcessed){
		status = (m_pParentDdm->*cb)(pOriginalContext, OK);
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
//		Modify a row only if the state has not been processed
//		
//
//************************************************************************
STATUS DataQueue
::CheckAndModifyRow(
			U32							type,
			PARTITION_STATE_IDENTIFIER	*pStateIdentifier,
			String64					tableName,
			rowID						*pRowToModify,
			void						*pBuffer,
			U32							sizeofData,
			rowID						*pNewRowIdRet,
			pTSCallback_t				cb,
			void						*pOriginalContext)
{
	STATUS							status;

	// if state was already processed, pUtil should have UDT_INSERTED state or greater
	BOOL stateProcessed = CheckIfStateAlreadyProcessed(
				type,
				pStateIdentifier,
				pBuffer);
	if (stateProcessed){
		status = (m_pParentDdm->*cb)(pOriginalContext, OK);
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
//		Set the state identifier
//		
//
//************************************************************************
void DataQueue
::SetStateIdentifier(
			PARTITION_STATE_IDENTIFIER	*pStateIdentifier,
			U32							opcode,
			rowID						*pRowId,
			U32							state,
			U32							index)
{
	assert(pRowId);
	pStateIdentifier->opcode = opcode;
	pStateIdentifier->cmdRowId = *pRowId;
	pStateIdentifier->state = state;
	pStateIdentifier->index = index;
}


