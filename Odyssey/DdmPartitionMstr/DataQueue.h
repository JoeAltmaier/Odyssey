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
// File: DataQueue.h
// 
// Description:
// The header for the Data Queue class
// 
// $Log: /Gemini/Odyssey/DdmPartitionMstr/DataQueue.h $
// 
// 1     9/15/99 4:00p Dpatel
// Initial creation
// 
//
/*************************************************************************/

#ifndef __DataQueue_h
#define __DataQueue_h

#include "CtTypes.h"
#include "TableMsgs.h"
#include "DdmOsServices.h"
#include "PtsCommon.h"
#include "ReadTable.h"
#include "Listen.h"
#include "Fields.h"
#include "Rows.h"
#include "Listen.h"
#include "Table.h"


#include "PartitionTable.h"
#include "TableServices.h"


#pragma	pack(4)



class DataQueue: DdmServices {
public:
	DataQueue(DdmServices *, TableServices *);
	~DataQueue();

	STATUS Add(
			U32							type,
			rowID						*pRowId,
			PARTITION_STATE_IDENTIFIER	*pStateIdentifier,
			void						*pData,
			U32							sizeofData,
			BOOL						addToTail = true);

	STATUS Modify(
			U32							type, 
			rowID						*pRowId, 
			PARTITION_STATE_IDENTIFIER	*pStateIdentifier,
			void						*pData, 
			U32							sizeofData, 
			BOOL						addToTail = true);

	STATUS Get(
			U32							type,
			rowID						*pRowId,
			void						**pDataRet,
			U32							sizeofData);

	STATUS Traverse(
			U32							type,
			rowID						*pRowId, 
			void						**ppDataRet,
			rowID						**pRowIdRet,
			PARTITION_STATE_IDENTIFIER	**ppStateIdentifierRet = NULL);

	STATUS Remove(
			U32					type,
			rowID				*pRowId);

	// CHECK AND PERFORM OPERATIONS
	void SetStateIdentifier(
			PARTITION_STATE_IDENTIFIER	*pStateIdentifier,
			U32							opcode,
			rowID						*pRowId,
			U32							state,
			U32							index);


	STATUS CheckIfStateAlreadyProcessed(
			U32							type,
			PARTITION_STATE_IDENTIFIER	*pStateIdentifier,
			void						*pBuffer = NULL);

	STATUS CheckAndInsertRow(
			U32							type,
			PARTITION_STATE_IDENTIFIER	*pStateIdentifier,
			String64					tableName,
			void						*pBuffer,
			U32							sizeofData,
			rowID						*pNewRowIdRet,
			pTSCallback_t				cb,
			void						*pOriginalContext);
	STATUS CheckAndModifyRow(
			U32							type,
			PARTITION_STATE_IDENTIFIER	*pStateIdentifier,
			String64					tableName,
			rowID						*pRowToModify,
			void						*pBuffer,
			U32							sizeofData,
			rowID						*pNewRowIdRet,
			pTSCallback_t				cb,
			void						*pOriginalContext);

private:
	DdmServices			*m_pParentDdm;
	TableServices		*m_pTableServices;

	class DataObject{
	public:
		U32					type;	// any type of data which user specifies
		rowID				rowId;	// rowid to use as key
		PARTITION_STATE_IDENTIFIER	stateIdentifier;
		void				*pData;		// actual data ptr
		U32					sizeofData;	// size of dat
		DataObject			*pNext;		// ptr to next Data Object

		DataObject(
				U32							_type, 
				rowID						*pRowId, 
				PARTITION_STATE_IDENTIFIER	*pStateIdentifier,
				void						*_pData, 
				U32							_sizeofData)
		{
			type = _type;
			rowId = *pRowId;
			if (pStateIdentifier){
				memcpy(&stateIdentifier, pStateIdentifier, sizeof(PARTITION_STATE_IDENTIFIER));
			} else {
				memset(&stateIdentifier, 0, sizeof(PARTITION_STATE_IDENTIFIER));
			}
			pData = new char[_sizeofData];
			memcpy(pData, _pData, _sizeofData);
			sizeofData = _sizeofData;
			pNext = NULL;
		}

		~DataObject()
		{
			if (pData){
				delete pData;
				pData = NULL;
			}
		}
	};

	DataObject	*GetDataObject(U32 type, rowID *pRowId);


	DataObject				*m_pHead;
	DataObject				*m_pTail;
};
#endif


