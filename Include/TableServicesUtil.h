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
// File: TableServicesUtil.h
// 
// Description:
// Defines a class used to access the Tables Services
// 
// $Log: /Gemini/Include/TableServicesUtil.h $
// 
// 1     9/12/99 10:34p Joehler
// Replaced AlarmTableServices.cpp
//  

/*************************************************************************/

#ifndef __TableServicesUtil_h
#define __TableServicesUtil_h

#include "PtsCommon.h"
#include "Fields.h"
#include "Rows.h"
#include "Table.h"

class TableServicesUtil: DdmServices {
public:
	TableServicesUtil(DdmServices *pParentDdm);
	~TableServicesUtil();
	STATUS EnumTable(
			char			*tableName,
			void			**ppTableDataRet,		// where you want data
			U32				*pSizeofTableDataRet,	// where you want size of ret data
			U32				*pNumberOfRows,
			pTSCallback_t	pCallback,
			void			*pOriginalContext);
	STATUS ReadRow(
			String64		tableName,
			rowID			*pRowToRead,
			void			*buffer,
			U32				sizeofData,
			pTSCallback_t	pCallback,
			void			*pOriginalContext);	
	STATUS ReadRowWithKey(
			String64		tableName,
			String64		keyName,
			void			*pRowKey,
			U32				sizeofKey,
			void			*buffer,
			U32				sizeofData,
			pTSCallback_t	pCallback,
			void			*pOriginalContext);
	STATUS InsertRow(
			String64		tableName,
			void			*buffer,
			U32				sizeofData,
			rowID			*pNewRowIdRet,
			pTSCallback_t	cb,
			void			*pOriginalContext);
	STATUS ModifyRow(
			String64		tableName,
			rowID			*pRowToModify,
			void			*buffer,
			U32				sizeofData,
			rowID			*pNewRowIdRet,
			pTSCallback_t	cb,
			void			*pOriginalContext);
	STATUS ModifyField(
			String64		tableName,
			rowID			*pRowToModify,
			void			*buffer,
			String64		nameOfFieldToModify,
			void			*pFieldValue,
			U32				sizeofField,
			rowID			*pNewRowIdRet,
			pTSCallback_t	cb,
			void			*pOriginalContext);
	STATUS DeleteRow(
			String64		tableName,
			rowID			*pRowId,
			pTSCallback_t	cb,
			void			*pOriginalContext);
	STATUS ModifyField(
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
	STATUS Enumerate(void *pContext, STATUS status);
	STATUS EnumReplyHandler(void *_pContext, STATUS status);
	STATUS OperationsReplyHandler(void *_pContext, STATUS status);

};
#endif


