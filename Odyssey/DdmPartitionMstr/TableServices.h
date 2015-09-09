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
// File: RmstrCapabilities.h
// 
// Description:
// Defines the Rmstr interface for Commands/Status.
// 
// $Log: /Gemini/Odyssey/DdmPartitionMstr/TableServices.h $
// 
// 2     1/21/00 12:04p Szhang
// 
// 1     8/23/99 2:17p Dpatel
// Initial creation.
// 
// 4     8/20/99 3:03p Dpatel
// added simulation for failover and failover code (CheckAnd...() methods)
// 
// 3     8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64. added simulation raid
// ddm.
// 
// 2     7/30/99 6:41p Dpatel
// Change preferred member, processing member down and stop util as
// internal cmds..
// 
// 1     7/28/99 6:36p Dpatel
// Initial Creation
// 
//
/*************************************************************************/

#ifndef __TableServices_h
#define __TableServices_h

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





#pragma	pack(4)



class TableServices: DdmServices {
public:
	TableServices(DdmServices *pParentDdm);
	~TableServices();
	STATUS TableServiceEnumTable(
			char			*tableName,
			void			**ppTableDataRet,		// where you want data
			U32				*pSizeofTableDataRet,	// where you want size of ret data
			U32				*pNumberOfRows,
			void			*pOriginalContext,
			pTSCallback_t	pCallback);
	STATUS TableServiceReadRow(
			String64		tableName,
			rowID			*pRowToRead,
			void			*buffer,
			U32				sizeofData,
			pTSCallback_t	pCallback,
			void			*pOriginalContext);

	STATUS TableServiceInsertRow(
			String64		tableName,
			void			*buffer,
			U32				sizeofData,
			rowID			*pNewRowIdRet,
			pTSCallback_t	cb,
			void			*pOriginalContext);
	STATUS TableServiceModifyRow(
			String64		tableName,
			rowID			*pRowToModify,
			void			*buffer,
			U32				sizeofData,
			rowID			*pNewRowIdRet,
			pTSCallback_t	cb,
			void			*pOriginalContext);
	STATUS TableServiceDeleteRow(
			String64		tableName,
			rowID			*pRowId,
			pTSCallback_t	cb,
			void			*pOriginalContext);
	STATUS TableServiceDeleteAllMatchedRows(
			String64		tableName,
			String64		KeyField,
			rowID			*pKeyFieldValue,
			pTSCallback_t	cb,
			void			*pOriginalContext);
	STATUS TableServiceModifyField(
			String64		tableName,
			rowID			*pRowIdToModify,
			String64		fieldNameToModify,
			void			*pNewFieldValue,		
			U32				sizeofNewFieldValue,	
			pTSCallback_t	cb,
			void			*pContext);


private:
	DdmServices			*m_pParentDdm;
	
	struct TABLE_CONTEXT{
		U32				state;
		void			*pData;
		void			*pData1;
		void			*pData2;
		void			*pData3;
		U32				value;
		U32				value1;
		U32				value2;
		U32				value3;
		rowID			newRowId;
		void			*pParentContext;
		pTSCallback_t	pCallback;

		TABLE_CONTEXT(){
			state = 0;
			pData = NULL;
			pData1 = NULL;
			pData2 = NULL;
			pData3 = NULL;
			value = 0;
			value1 = 0;
			value2 = 0;
			value3 = 0;
			pParentContext = NULL;
			pCallback = NULL;
		}
		~TABLE_CONTEXT(){
			if (pData){
				delete pData;
				pData = NULL;
			}
			if (pData1){
				delete pData1;
				pData1 = NULL;
			}
			if (pData2){
				delete pData2;
				pData2 = NULL;
			}
			if (pData3){
				delete pData3;
				pData3 = NULL;
			}
		}
	};
private:
	STATUS TableServiceEnumerate(void *pContext, STATUS status);
	STATUS TableServiceEnumReplyHandler(void *_pContext, STATUS status);
	STATUS TableServiceOperationsReplyHandler(void *_pContext, STATUS status);

};
#endif


