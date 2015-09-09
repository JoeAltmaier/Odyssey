/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
// This file is source file for the Persistant Table Service class of DDM.
// 
/*************************************************************************/

//  #define PERSIST  only use for compile verify
#include "cttypes.h"
#include "DdmPTS.h"
#include "PtsCommon.h"
#include "TableMsgs.h"
#include "buildsys.h"
#include "hw.h"

#define _TRACEF
#define	TRACE_INDEX		TRACE_PTS
#include "Odyssey_Trace.h"

//#include "Watch.h"	// Usage ex.: Watch(WATCH1, (void *)&fLoadFromFlash, sizeof(fLoadFromFlash), WATCHSTORE);

/*************************************************************************/
// Declare and initialize static variables and other global data.
/*************************************************************************/

CLASSNAME(DdmPTS, SINGLE);

SERVEVIRTUAL(DdmPTS, TS_FUNCTION_DEFINE_TABLE   );
SERVEVIRTUAL(DdmPTS, TS_FUNCTION_INSERT_ROW     );
SERVEVIRTUAL(DdmPTS, TS_FUNCTION_INSERT_VLROW     );
SERVEVIRTUAL(DdmPTS, TS_FUNCTION_ENUMERATE_TABLE);
SERVEVIRTUAL(DdmPTS, TS_FUNCTION_GET_TABLE_DEF  );
SERVEVIRTUAL(DdmPTS, TS_FUNCTION_DELETE_TABLE  );
SERVEVIRTUAL(DdmPTS, TS_FUNCTION_READ_ROW       );
SERVEVIRTUAL(DdmPTS, TS_FUNCTION_READ_VARLEN_ROW );
SERVEVIRTUAL(DdmPTS, TS_FUNCTION_DELETE_ROW     );
SERVEVIRTUAL(DdmPTS, TS_FUNCTION_MODIFY_ROW     );
SERVEVIRTUAL(DdmPTS, TS_FUNCTION_MODIFY_FIELD   );
SERVEVIRTUAL(DdmPTS, TS_FUNCTION_LISTEN         );
SERVEVIRTUAL(DdmPTS, TS_FUNCTION_STOP_LISTENING );
SERVEVIRTUAL(DdmPTS, TS_FUNCTION_QUERY_SET_RID );
SERVEVIRTUAL(DdmPTS, TS_FUNCTION_TEST_SET_FIELD );
SERVEVIRTUAL(DdmPTS, TS_FUNCTION_MODIFY_BITS );


DdmPTS*			DdmPTS::pDdmPTS = NULL;
VDN				DdmPTS::vdPTS	= NULL;
DID				DdmPTS::didPTS  = NULL;			// our did


//******  temporary memory allocations for debug  *************
// allocate the data block in static data segment for debug

// Blobs live in the heap.
dataHeapHdr		*prgDataBlks;					// pointer to the memory buffer
dataBlkHdr		*pFreeBlk;						// temp pointer to free data block list

tableHdrDef		*pHeaders;						// chunk of memory with headers

tableHeader		*pFreeHeaders;					// temp pointer to free list of headers
tableHeader		*pListHeaders;					// temp pointer to list of current headers

#ifdef PERSIST
BOOL			fLoadFromFlash = TRUE;
#else
BOOL			fLoadFromFlash = FALSE;
#endif

extern listenReply *pListenReplies;

U32				cReplyDelete;

/*************************************************************************/
// DdmPTS
// Constructor method for the class DdmPTS
/*************************************************************************/

// calls the base class with our did
DdmPTS::DdmPTS(DID did):Ddm(did)
{

	vdPTS = GetVdn();
	didPTS = did;
	pDdmPTS = this;			// save the pointer to object

}	// DdmPTS


/*************************************************************************/
// Ctor
// Create a new instance of PTS Ddm
/*************************************************************************/

Ddm *DdmPTS::Ctor(DID did) {

	TRACE_ENTRY(DdmPTS::Ctor);

	return new DdmPTS(did); 

}	// Ctor


/*************************************************************************/
// Initialize
//
// Initialize the Ddm
//	for now, initialize the link list of headers and data blocks
//	used for the tables
//
/*************************************************************************/

STATUS DdmPTS::Initialize(Message *pMsg)
{
	STATUS	status;

	status = CreatePartitions();

	status = LoadTableHeaders();
	
	status = LoadTableData();
#ifdef PERSIST
	status = CleanupNonPersistData();
#endif
	status = InitializeListenerIDs (CLISTENERIDS, INITIALIZE );

	if (!fLoadFromFlash)
	{	// these only need to be called if first init
		status = InitializeHeaps ();
		status = CreateTableOfTables (CT_PTS_TABLE_OF_TABLES, iMaxHeaders);
	}


	// dispatch request to appropriate routines
 
	DispatchRequest( TS_FUNCTION_DEFINE_TABLE, REQUESTCALLBACK(DdmPTS, PtsDefineTable));
	DispatchRequest( TS_FUNCTION_INSERT_ROW, REQUESTCALLBACK(DdmPTS, PtsInsertRow));
	DispatchRequest( TS_FUNCTION_INSERT_VLROW, REQUESTCALLBACK(DdmPTS, PtsInsertVarLenRow));
	DispatchRequest( TS_FUNCTION_ENUMERATE_TABLE, REQUESTCALLBACK(DdmPTS, PtsEnumTable));
	DispatchRequest( TS_FUNCTION_GET_TABLE_DEF, REQUESTCALLBACK(DdmPTS, PtsGetTableDef));
	DispatchRequest( TS_FUNCTION_DELETE_TABLE, REQUESTCALLBACK(DdmPTS, PtsDeleteTable));
	DispatchRequest( TS_FUNCTION_READ_ROW, REQUESTCALLBACK(DdmPTS, PtsReadRow));
	DispatchRequest( TS_FUNCTION_READ_VARLEN_ROW, REQUESTCALLBACK(DdmPTS, PtsReadVarLenRow));
	DispatchRequest( TS_FUNCTION_DELETE_ROW, REQUESTCALLBACK(DdmPTS, PtsDeleteRow));
	DispatchRequest( TS_FUNCTION_MODIFY_ROW, REQUESTCALLBACK(DdmPTS, PtsModifyRow));
	DispatchRequest( TS_FUNCTION_MODIFY_FIELD, REQUESTCALLBACK(DdmPTS, PtsModifyField));
	DispatchRequest( TS_FUNCTION_LISTEN, REQUESTCALLBACK(DdmPTS, PtsListen));
	DispatchRequest( TS_FUNCTION_STOP_LISTENING, REQUESTCALLBACK(DdmPTS, PtsStopListen));
	DispatchRequest( TS_FUNCTION_QUERY_SET_RID, REQUESTCALLBACK(DdmPTS, PtsQuerySetRID));
	DispatchRequest( TS_FUNCTION_TEST_SET_FIELD, REQUESTCALLBACK(DdmPTS, PtsTestSetField));
	DispatchRequest( TS_FUNCTION_MODIFY_BITS, REQUESTCALLBACK(DdmPTS, PtsModifyBits));

	Reply (pMsg, ercOK);

	return ercOK;
}

//***************************************************************
//
//	Quiesce:  
//
//
STATUS DdmPTS::Quiesce(Message *pMsg)
{
	Reply (pMsg, ercOK);
	return ercOK;
}


//***************************************************************
//
//	Enable:  
//
//

STATUS DdmPTS::Enable(Message *pMsg)
{
	
	Reply (pMsg, ercOK);
	return ercOK;
}


//*********************************************************************************
//
//	CreatePartitions
//
//		Create the partitions for the table headers and data
//		
//********************************************************************************


STATUS DdmPTS::CreatePartitions()
{

	// allocate table headers space and data space; later probably want to get sizes.

	pHeaders = (tableHdrDef *)(new (tBIG|tUNCACHED|tZERO) uchar [sizeof(tableHdrDef)+102400]);
	prgDataBlks = (dataHeapHdr *)(new (tBIG|tUNCACHED|tZERO) uchar [CB_RGB_DATA_BLKS + 102400]);

#ifdef PERSIST
	// check if partitions exist;  need some other check??	
	pcfTableHdrs = new ChaosFile(CT_PTS_PART_TABLE_HDRS);
	if (pcfTableHdrs->GetErrorCode() == cfSuccess)
	{
		TRACEF(TRACE_L8, ("tableHeaders open \n"));
	}
	else	
	{	// header partition does not exist... create
		pcfTableHdrs = new ChaosFile(CT_PTS_PART_TABLE_HDRS, sizeof (tableHdrDef));
		if (pcfTableHdrs->GetErrorCode() == cfSuccess)
			TRACEF(TRACE_L8, ("Table Headers partition create \n"));
		fLoadFromFlash = false;
	}

	// this could be inconsistent...  if table headers valid and tables not
	pcfTableData1 = new ChaosFile(CT_PTS_PART_TABLE_DATA1);
	if (pcfTableData1->GetErrorCode() == cfSuccess)
	{
		TRACEF(TRACE_L8, ("Table data open \n"));
	}
	else
	{	// data partition does not exist... create
		pcfTableData1 = new ChaosFile(CT_PTS_PART_TABLE_DATA1, CB_RGB_DATA_BLKS);
		if (pcfTableData1->GetErrorCode() == cfSuccess)
			TRACEF(TRACE_L8, ("Table data partition created \n"));
		fLoadFromFlash = false;
	}		
#endif	
	return ercOK;
}


//*********************************************************************************
//
//	FlushToFlash
//
//		Write out headers and data to flash
//		
//********************************************************************************


STATUS DdmPTS::FlushToFlash()
{
	int status = 0;

#ifdef PERSIST

	if (pcfTableHdrs->IsValid() == false)
  		pcfTableHdrs = new ChaosFile(CT_PTS_PART_TABLE_HDRS);

	if (pcfTableData1->IsValid() == false)
  		pcfTableData1 = new ChaosFile(CT_PTS_PART_TABLE_DATA1);

	if ((status = pcfTableHdrs->Write (pHeaders, 0, sizeof(tableHdrDef))) == cfSuccess)
	{
		TRACEF(TRACE_L8, ("File Headers written \n"));
		if ((status = pcfTableHdrs->Close()) == cfSuccess)
			TRACEF(TRACE_L8, ("Table Headers closed \n"));
	}
	if (pcfTableData1->Write (prgDataBlks, 0, CB_RGB_DATA_BLKS) == cfSuccess)
	{
		TRACEF(TRACE_L8, ("Tables written \n"));
		if ((status = pcfTableData1->Close()) == cfSuccess)
			TRACEF(TRACE_L8, ("Tables closed \n"));
	}

#endif
	return ercOK;
}


//*********************************************************************************
//
//	CleanupNonPersistData
//
//		Delete tables that will not persist
//		
//********************************************************************************


STATUS DdmPTS::CleanupNonPersistData()
{
	int status = 0;

	BOOL lastHdr = false;
	pTableHeader	pTempHdr = pListHeaders;		
 	pTableHeader	pNextHdr = pListHeaders;		

if (fLoadFromFlash)	 	
{	// pListHeaders points to the beginning of the list of table headers;
	// If no tables are defined, this pointer = 0
		
	if (pTempHdr == 0)
		lastHdr = true;

	while (!lastHdr)
	{
		if (pTempHdr->oNext == 0)					// last header?
			lastHdr = true;
		// save the next pointer in case this header is deleted	
		pNextHdr = (pTableHeader) ((U32)pHeaders + pTempHdr->oNext);

		if ((pTempHdr->persistFlags & Persistant_PT) != Persistant_PT)  
		{
			status = DeleteTable(NULL, (pTempHdr->rowId.Table));
			// if status, ahhhh
		}
		else
		{	// if table is to persist, only check for fields if there are entries
			if (pTempHdr->cCurrentEntries > 0)
				ClearNonPersistFields(pTempHdr);
		}
		pTempHdr = pNextHdr;
	}

}
	return ercOK;
}


//*********************************************************************************
//
//	LoadTableHeaders
//
//		Load the table Headers from flash unless this is the first init.
//		
//********************************************************************************


STATUS DdmPTS::LoadTableHeaders()
{
	BOOL lastHdr = false;
	BOOL found = false;
	U32 i;

#ifdef PERSIST
	U32 status;
	if (fLoadFromFlash)
	{	// read the table from flash;  
		if (pcfTableHdrs->Read (pHeaders, 0, sizeof(tableHdrDef)) == cfSuccess)
		{	TRACEF(TRACE_L8, ("ChaosFileRead 1 \n"));}
		else
		{	TRACEF(TRACE_L8, ("Table Header read failed \n"));	}
				
		if ((status = pcfTableHdrs->Close()) == cfSuccess)
		{	TRACEF(TRACE_L8, ("Table Headers closed \n"));}
		else
		{	TRACEF(TRACE_L8, ("Table Header close on read failed \n")); }	

		pFreeHeaders = (tableHeader *)((U32)pHeaders + pHeaders->oFreeHdr);
		pListHeaders = (tableHeader *)((U32)pHeaders + pHeaders->oListHdr);

		pTableHeader	pTempHdr = pListHeaders;		
		if (pTempHdr == 0)
			lastHdr = true;
		while (!lastHdr)
		{
			pTempHdr->pTableData = (dataBlkHdr *)((U32)prgDataBlks + pTempHdr->oTableData);
			pTempHdr->cListens = 0;
			pTempHdr->pListenQ = NULL;
			{	// get next header 
				if (pTempHdr->oNext == 0)					// last header?
					lastHdr = true;
				else										// 
					pTempHdr = (pTableHeader) ((U32)pHeaders + pTempHdr->oNext);
			}
		}
	}
	else
#endif

	{	// initialize tables for first time
		pFreeHeaders = pHeaders->rgHdr;			// these pointers are temp during execution
		memset (pFreeHeaders, 0, sizeof(pHeaders->rgHdr));
		pListHeaders = NULL;						// head of list of headers
		pHeaders->oFreeHdr = (U32)(pFreeHeaders) - (U32)(pHeaders);
		pHeaders->oListHdr = NULL;
		pHeaders->tableCount = 0;

		// link headers
		pTableHeader pTmp = pFreeHeaders;
		for (i = 0; i < iMaxHeaders-1; i++)
		{ // add offset of header to size to get next
			pTmp->oNext = (U32)pTmp - (U32)pHeaders + sizeof(tableHeader); 
			pTmp = (pTableHeader)((U32)pTmp + sizeof (tableHeader));
		}	 		
		 pTmp->oNext = NULL;			// zero out link in end of header list
	}
	return ercOK;
}

//*********************************************************************************
//
//	LoadTableData
//
//		Load the table data from flash unless this is the first init.
//		
//********************************************************************************


STATUS DdmPTS::LoadTableData()
{
#ifdef PERSIST
	U32 status;
	
	if (fLoadFromFlash)
	{
		uchar *pTmp = (uchar*)prgDataBlks;
		// read the table from flash;  
		if (pcfTableData1->Read (prgDataBlks, 0, CB_RGB_DATA_BLKS ) == cfSuccess)
		{	TRACEF(TRACE_L8, ("TableData read \n"));}
		else
		{	TRACEF(TRACE_L8, ("TableData read failed \n"));}	
				
		if ((status = pcfTableData1->Close()) == cfSuccess)
		{	TRACEF(TRACE_L8, ("TableData file closed \n")); }
		else
		{	TRACEF(TRACE_L8, ("TableData close on read failed \n"));}
			
		pFreeBlk = (pdataBlkHdr)((U32)prgDataBlks + prgDataBlks->oFreeBlk);
	}
	else
#endif

	{	// this is the first time
		pFreeBlk = (pdataBlkHdr)&(prgDataBlks->firstBlk);
		//  initialize heap of data blocks
		prgDataBlks->oFreeBlk = (U32)pFreeBlk - (U32) prgDataBlks;
		pFreeBlk->cbBlock = CB_RGB_DATA_BLKS - sizeof(dataHeapHdr) + sizeof (dataBlkHdr);
		pFreeBlk->oNext = 0;
	}
	return ercOK;
}


/*********************************************************************************
//
//	SendListenReplies
//
//		Send all replies
//		pListenReplies should be set to null upon leaving routine
//********************************************************************************/


void DdmPTS::SendListenReplies()
{

	listenReply	*pCurrentListenReply;

	// if there are listener(s) on the queue pointed to by pListenReplies, 
	// send replies;  Delete the listen reply block. 
	
	cReplyDelete = 0;			//debug file
	
	while (pListenReplies != NULL)	
	{
 		Reply((Message*)(pListenReplies->pListenMsg),
						 OS_DETAIL_STATUS_SUCCESS, pListenReplies->lastReply);
		pCurrentListenReply = pListenReplies;			// save for delete
		pListenReplies = pListenReplies->pNext;
		cReplyDelete++;
		delete (pCurrentListenReply);
	}
}



/*********************************************************************************
//
//	SendMirroredRequests
//
//	
//********************************************************************************/


/*void DdmPTS::SendMirroredRequests(Message *pMsg)
{
   	Message *pMirroredMsg = new Message(pMsg);


}
*/

/*************************************************************************/
//
//
/*************************************************************************/


STATUS DdmPTS::PtsDefineTable(Message *pMsg)

{

	STATUS   status;
	DefineTablePayload		*pDefineTablePayload;
	fieldDef				*prgFieldsToDefine;
	U32						cbrgFieldsToDefine;
	void					*ptableName;		// Null terminated name of the table to create.
	U32						cbTableName;
	U32						replyStatus;
		
	// Get a pointer to the message payload which is the new table definition.
	pDefineTablePayload = (DefineTablePayload*)pMsg->GetPPayload();
		
	pMsg->GetSgl( DEFINE_TABLE_MSG_TABLENAME_SGI,				//  index of the SG List of Field Defs 
				  &ptableName,
				  &cbTableName);
					  
	pMsg->GetSgl( DEFINE_TABLE_MSG_FIELDDEFS_SGI,				//  index of the SG List of Field Defs 
				  (void**)&prgFieldsToDefine,
				  &cbrgFieldsToDefine );
					  
	// extract parameters and call DefineTable.
	status = DefineTable( (char*)ptableName,					// Name of the table to define.
						  prgFieldsToDefine,					// Array of fields to define.
				 		  cbrgFieldsToDefine,					// size of fields array.
						  pDefineTablePayload->cEntriesRsv,		// number of entries to create by default.
						  pDefineTablePayload->persistFlags,	// Should this table persist?
						  &pDefineTablePayload->tableIdRet);	// table Id returned	

	// Return Status.
	replyStatus = Reply((Message*)pMsg, status, true);
	return OS_DETAIL_STATUS_SUCCESS;

}

STATUS DdmPTS::PtsInsertRow(Message *pMsg)
{

	STATUS   status;
  	char					*ptableName;		
	U32						cbTableName;
	void					*prgRows;			
	U32						cbrgRows;
	InsertRowsPayload  		*pInsertRowsPayload;
	U32						replyStatus;


	// Get a pointer to the message payload.
	pInsertRowsPayload = (InsertRowsPayload*)pMsg->GetPPayload();
		
	pMsg->GetSgl( INSERT_ROWS_MSG_TABLENAME_SGI,				//  index of the SG List to tablename 
				  (void**)&ptableName, &cbTableName);			//  pointer, size of buffer
		
	pMsg->GetSgl( INSERT_ROWS_MSG_DATA_BUFFER_SGI,				//  index of the SG List to row Defs 
				  &prgRows, &cbrgRows);							//  pointer, size of buffer

	// extract parameters and call InsertRow.
	status = InsertRow    ((char*)ptableName,					// Name of the table.
						  (uchar*)prgRows, (U32)cbrgRows,		// Array of rows, size of array
						  pMsg,
						  &(pInsertRowsPayload->cIDsRet));		// number of IDs returned

	// Reply to pMsg.  
	replyStatus = Reply(pMsg, status, true);
	TRACEF(TRACE_L8, ("Insert STATUS = %lx\n", status ));
	return OS_DETAIL_STATUS_SUCCESS;

}

STATUS DdmPTS::PtsInsertVarLenRow(Message *pMsg)
{

	STATUS   status;
  	char					*ptableName;		
	U32						cbTableName;
	void					*pRowData;			
	U32						cbRowData;
	InsertVarLenRowPayload  *pInsertVLRowPayload;
	U32						replyStatus;


	// Get a pointer to the message payload.
	pInsertVLRowPayload = (InsertVarLenRowPayload*)pMsg->GetPPayload();
		
	pMsg->GetSgl( INSERT_VLROW_MSG_TABLENAME_SGI,				//  index of the SG List to tablename 
				  (void**)&ptableName, &cbTableName);			//  pointer, size of buffer
		
	pMsg->GetSgl( INSERT_VLROW_MSG_DATA_SGI,					//  index of the SG List to row Defs 
				  &pRowData, &cbRowData);						//  pointer, size of buffer

	// extract parameters and call InsertVarLenRow.
	status = InsertVarLenRow (ptableName,						// Name of the table.
							 (uchar *)pRowData, cbRowData,		// pbcb of fixed length row; does not include varlenField structs
							 &(pInsertVLRowPayload->rowId));	// rowID returned

	// Reply to pMsg.  
	replyStatus = Reply(pMsg, status, true);
	TRACEF(TRACE_L8, ("InsertVL STATUS = %lx\n", status ));
	return OS_DETAIL_STATUS_SUCCESS;

}
STATUS DdmPTS::PtsEnumTable(Message *pMsg)
{
	STATUS   status;
	char						*ptableName;				// Null terminated name of the table.
	U32							cbTableName;
	EnumerateTablePayload		*pEnumTablePayload;
		
	// Get a pointer to the message payload which is the table name.
	pEnumTablePayload = (EnumerateTablePayload*)pMsg->GetPPayload();
	
	pEnumTablePayload->cbDataRet = 0;						// initialize in case of error
	pMsg->GetSgl( ENUMERATE_TABLE_MSG_TABLENAME_SGI,		// index of the SG List to tablename 
				  (void**)&ptableName,
				  &cbTableName);

	
	// extract parameters and call EnumerateTable.
	status = EnumerateTable((char*)ptableName,				// Name of the table.
					  pEnumTablePayload->uStartingRow,		// offset of first row to return (0 based)
					  (uchar *)NULL, (U32)NULL,				// buffer where rows returned
					  pMsg,									// message
					  &pEnumTablePayload->cbDataRet);		// number of bytes returned in buffer.
	
	// Reply to pMsg.  
	Reply((Message*)pMsg, status, true);
	return OS_DETAIL_STATUS_SUCCESS;
}


STATUS DdmPTS::PtsQuerySetRID(Message *pMsg)
{
	STATUS   status;
	char					*ptableName;				// Null terminated name of the table.
	U32						cbTableName;
	QuerySetRIDPayload		*pQuerySetPayload;
		
	// Get a pointer to the message payload which is the table name.
	pQuerySetPayload = (QuerySetRIDPayload*)pMsg->GetPPayload();

	pMsg->GetSgl( QUERY_SET_RID_MSG_TABLENAME_SGI,		//  index of the SG List to tablename 
				  (void**)&ptableName,
				  &cbTableName);

	// extract parameters and call QueryAndSetRID.
	status = QueryOrSetRID ((char*)ptableName,				// Name of the table.
					  pQuerySetPayload->opRID,				// operation: query or set RID
					  &pQuerySetPayload->rowId);			// rowID to send or return
	
	// Reply to pMsg.  
	Reply((Message*)pMsg, status, true);
	return OS_DETAIL_STATUS_SUCCESS;
}

STATUS DdmPTS::PtsGetTableDef(Message *pMsg)
{
	STATUS   status;
	GetTableDefPayload		*pGetTableDefPayload;
	char					*ptableName;		// Null terminated name of the table.
	U32						cbTableName;
	tableDef				*prgTableDef;
	U32						cbTableDef;

	// Get a pointer to the message payload.
	pGetTableDefPayload = (GetTableDefPayload*)pMsg->GetPPayload();

	// Get the table name from the scatter gather list.
	pMsg->GetSgl( GET_TABLE_DEF_MSG_TABLENAME_SGI,		// index of the SGList of Field Defs 
				  (void**)&ptableName, &cbTableName);
	
	if (cbTableName == 0)
		ptableName = NULL;
			
	// Get the address of the table definition array where info is returned.
	pMsg->GetSgl( GET_TABLE_DEF_REPLY_TABLEDEF_SGI,		// index of the SGList of Field Defs 
				  (void**)&prgTableDef, &cbTableDef);

	// extract parameters and call GetTableDef.
	status = GetTableDef( (char*) ptableName,						// Name of the table.
						  &pGetTableDefPayload->tableId,			// tableId
				 		  pMsg,
						  &pGetTableDefPayload->cbFieldDefsRet,		// number of bytes returned in above array.
						  prgTableDef,	cbTableDef);				// array returned with cRows, cFields, cbRows.

	// Reply to pMsg.  
	Reply((Message*)pMsg, status, true);
	return OS_DETAIL_STATUS_SUCCESS;

}

STATUS DdmPTS::PtsDeleteTable(Message *pMsg)
{

	STATUS   status;
	char						*ptableName;		// Null terminated name of the table.
	U32							cbTableName;
	DeleteTablePayload			*pDeleteTablePayload;
		
	// Get a pointer to the message payload.
	pDeleteTablePayload = (DeleteTablePayload*)pMsg->GetPPayload();

	pMsg->GetSgl( DELETE_TABLE_MSG_TABLENAME_SGI,		//  index of the SG List to tablename 
				  (void**)&ptableName,
				  &cbTableName);
	// extract parameters and call DeleteTable.
	status = DeleteTable((char*)ptableName, pDeleteTablePayload->tableId.Table);	
		
	// Reply to pMsg.  
	Reply((Message*)pMsg, status, true);
	return OS_DETAIL_STATUS_SUCCESS;

}

STATUS DdmPTS::PtsReadRow(Message *pMsg)
{

	STATUS   status;
	char					*ptableName;		// Null terminated name of the table.
	U32						cbTableName;
												// Request parameters
	void					*pKeyName;			//  array key field names
	U32						cbKeyName;			//  size of array
	void					*pKeyValue;		
	U32						cbKeyValue;
												// Reply parameters
	ReadRowsPayload			*pReadRowsPayload;
	
	// Get a pointer to the message payload.
	pReadRowsPayload = (ReadRowsPayload*)pMsg->GetPPayload();
	pMsg->GetSgl( READ_ROWS_MSG_TABLENAME_SGI,					//  index of the SG List to tablename 
				  (void**)&ptableName, &cbTableName);			//  pointer, size of buffer
		
	pMsg->GetSgl( READ_ROWS_MSG_KEY_NAMES_SGI,					//  index of the SG List to row Defs 
				  &pKeyName, &cbKeyName);						//  pointer, size of buffer
	if (cbKeyName == 0)											//  transport sends an addr; need NULL
		pKeyName = NULL;

	pMsg->GetSgl( READ_ROWS_MSG_KEY_VALUES_SGI,					//  index of the SG List to row Defs 
				  &pKeyValue, &cbKeyValue);						//  pointer, size of buffer

	// extract parameters and call ReadRow.
	status = ReadRow  ((char*)ptableName,						// Name of the table.
					  (char*)pKeyName,							// pointer to key name
					  (uchar*)pKeyValue,						// pointer to key value
					  cbKeyValue,
					  pMsg,
					  &pReadRowsPayload->cRowsReadRet);			// number of IDs returned

	// Reply to pMsg.  
	Reply(pMsg, status, true);
 	return OS_DETAIL_STATUS_SUCCESS;
}

STATUS DdmPTS::PtsReadVarLenRow(Message *pMsg)
{

	STATUS					status;
	char					*ptableName;		// Null terminated name of the table.
	U32						cbTableName;
												// Request parameters
	void					*pKeyName;			//  array key field names
	U32						cbKeyName;			//  size of array
	void					*pKeyValue;		
	U32						cbKeyValue;
												// Reply parameters
	ReadVarLenRowsPayload		*pReadVarLenRowsPayload;
	
	// Get a pointer to the message payload.
	pReadVarLenRowsPayload = (ReadVarLenRowsPayload*)pMsg->GetPPayload();
	pMsg->GetSgl( READ_VLROWS_MSG_TABLENAME_SGI,					//  index of the SG List to tablename 
				  (void**)&ptableName, &cbTableName);			//  pointer, size of buffer
		
	pMsg->GetSgl( READ_VLROWS_MSG_KEY_NAMES_SGI,					//  index of the SG List to row Defs 
				  &pKeyName, &cbKeyName);						//  pointer, size of buffer
	if (cbKeyName == 0)											//  transport sends an addr; need NULL
		pKeyName = NULL;

	pMsg->GetSgl( READ_VLROWS_MSG_KEY_VALUES_SGI,					//  index of the SG List to row Defs 
				  &pKeyValue, &cbKeyValue);						//  pointer, size of buffer

	// extract parameters and call ReadRow.
	status = ReadVLRow((char*)ptableName,						// Name of the table.
					  (char*)pKeyName,							// pointer to key name
					  (uchar*)pKeyValue,						// pointer to key value
					  cbKeyValue,
					  pMsg,
					  &pReadVarLenRowsPayload->cRowsReadRet);	// number of IDs returned

	// Reply to pMsg.  
	Reply(pMsg, status, true);
 	return OS_DETAIL_STATUS_SUCCESS;
}


STATUS DdmPTS::PtsModifyRow(Message *pMsg)
{

	STATUS   status;
	char					*ptableName;		// Null terminated name of the table.
	U32						cbTableName;
												// Request parameters
	void					*prgKeyName;		//  array key field names
	U32						cbrgKeyName;		//  size of array
	void					*prgKeyValue;		
	U32						cbrgKeyValue;
	void					*prgRows;
	U32						cbrgRows;
												// Reply parameters
	ModifyRowsPayload		*pModifyRowsPayload;
	U32						cRowsModify;
		
	// Get a pointer to the message payload.
	pModifyRowsPayload = (ModifyRowsPayload*)pMsg->GetPPayload();

	cRowsModify = pModifyRowsPayload->cRowsToModify;			// use local variable, so can get return value also
	pMsg->GetSgl( MODIFY_ROWS_MSG_TABLENAME_SGI,				//  index of the SG List to tablename 
				  (void**)&ptableName, &cbTableName);			//  pointer, size of buffer
	
	pMsg->GetSgl( MODIFY_ROWS_MSG_KEY_NAMES_SGI,				//  index of the SG List to row Defs 
				  &prgKeyName, &cbrgKeyName);					//  pointer, size of buffer

	if (cbrgKeyName == 0)										//  transport sends an addr; need NULL if 0
		prgKeyName = NULL;

	pMsg->GetSgl( MODIFY_ROWS_MSG_KEY_VALUES_SGI,				//  index of the SG List to row Defs 
				  &prgKeyValue, &cbrgKeyValue);					//  pointer, size of buffer

	pMsg->GetSgl( MODIFY_ROWS_MSG_DATA_BUFFER_SGI,				//  index of the SG return list of rowIDs 
				  &prgRows, &cbrgRows);							//  pointer, size of return buffer of IDs

	// extract parameters and call ModifyRow.
	status = ModifyRow((char*)ptableName,						// Name of the table.
					  (char*)prgKeyName, cbrgKeyName,			// pointer to key name
					  (uchar*)prgKeyValue,						// pointer to key value
					  cbrgKeyValue,
					  (uchar*)prgRows,							// pointer to modified rows
					  cbrgRows,
					  pMsg,
					  &cRowsModify);							// number of rows to modify, and # modified

	pModifyRowsPayload->cRowsModifiedRet = cRowsModify;

	Reply(pMsg, status, true);
	return OS_DETAIL_STATUS_SUCCESS;
}


STATUS DdmPTS::PtsModifyField(Message *pMsg)
{
	STATUS   status;
	char					*ptableName;			// Null terminated name of the table.
	U32						cbTableName;		
														// Request parameters
	void					*prgKeyName;			//  array key field names
	U32						cbrgKeyName;			//  size of array
	void					*prgKeyValue;		
	U32						cbrgKeyValue;
	void					*prgFieldName;			//	array of fieldnames of modified data
	U32						cbrgFieldName;		
	void					*prgFieldValue;			//	array of modified data
	U32						cbrgFieldValue;		
														// Reply parameters
	ModifyFieldsPayload 	*pModifyFieldsPayload;
	U32						cRowsModify;

	// Get a pointer to the message payload.
	pModifyFieldsPayload = (ModifyFieldsPayload*)pMsg->GetPPayload();
	cRowsModify = pModifyFieldsPayload->cRowsToModify;	// use local variable, so can get return value also

	pMsg->GetSgl( MODIFY_FIELDS_MSG_TABLENAME_SGI,		//  index of the SG List to tablename 
				  (void**)&ptableName, &cbTableName);	//  pointer, size of buffer
		
	pMsg->GetSgl( MODIFY_FIELDS_MSG_KEY_NAMES_SGI,		//  index of the SG List to row Defs 
				  &prgKeyName, &cbrgKeyName);			//  pointer, size of buffer

	if (cbrgKeyName == 0)								//  transport sends an addr; need NULL
		prgKeyName = NULL;

	pMsg->GetSgl( MODIFY_FIELDS_MSG_KEY_VALUES_SGI,		//  index of the SG List to row Defs 
				  &prgKeyValue, &cbrgKeyValue);			//  pointer, size of buffer

	pMsg->GetSgl( MODIFY_FIELDS_MSG_FIELD_NAMES_SGI,	//  index of the SG List to row Defs 
				  &prgFieldName, &cbrgFieldName);		//  pointer, size of buffer
	
	if (cbrgFieldName == 0)								//  transport sends an addr; need NULL
		prgFieldName = NULL;

	pMsg->GetSgl( MODIFY_FIELDS_MSG_FIELD_VALUES_SGI,	//  index of the SG List to row Defs 
				  &prgFieldValue, &cbrgFieldValue);		//  pointer, size of buffer

	// extract parameters and call ModifyField.
	status = ModifyField((char*)ptableName,					// Name of the table.
					  (char*)prgKeyName, cbrgKeyName,		// pointer to key name
					  (uchar*)prgKeyValue,					// pointer to key value
					  (U32)cbrgKeyValue,
					  (char*)prgFieldName,					// pointer to key name
					  (uchar*)prgFieldValue,				// pointer to key value
					  cbrgFieldValue,
					  pMsg,
					  &cRowsModify);						// number of IDs returned


	pModifyFieldsPayload->cRowsModifiedRet = cRowsModify;

	Reply(pMsg, status, true);
 	return OS_DETAIL_STATUS_SUCCESS;

}


STATUS DdmPTS::PtsModifyBits(Message *pMsg)
{
	STATUS					status;
	U32						cRowsModify;
	char					*ptableName;				// Null terminated name of the table.
	U32						cbTableName;		
														// Request parameters
	void					*pKeyName;					//  array key field names
	U32						cbKeyName;					//  size of array
	void					*pKeyValue;		
	U32						cbKeyValue;
	void					*pFieldName;				//	array of fieldnames of modified data
	U32						cbFieldName;		
	void					*pFieldMask;				//	array of modified data
	U32						cbFieldMask;		
														// Reply parameters
	ModifyBitsPayload 		*pModifyBitsPayload;

	// Get a pointer to the message payload.
	pModifyBitsPayload = (ModifyBitsPayload*)pMsg->GetPPayload();
	cRowsModify = pModifyBitsPayload->cRowsToModify;			// use local variable, so can get return value also

	pMsg->GetSgl( MODIFY_BITS_MSG_TABLENAME_SGI,		//  index of the SG List to tablename 
				  (void**)&ptableName, &cbTableName);	//  pointer, size of buffer
		
	pMsg->GetSgl( MODIFY_BITS_MSG_KEY_NAME_SGI,			//  index of the SG List to row Defs 
				  &pKeyName, &cbKeyName);				//  pointer, size of buffer
 	if (cbKeyName == 0)									//  transport sends an addr; need NULL
		pKeyName = NULL;

	pMsg->GetSgl( MODIFY_BITS_MSG_KEY_VALUE_SGI,		//  index of the SG List to row Defs 
				  &pKeyValue, &cbKeyValue);				//  pointer, size of buffer

	pMsg->GetSgl( MODIFY_BITS_MSG_FIELD_NAME_SGI,		//  index of the SG List to row Defs 
				  &pFieldName, &cbFieldName);			//  pointer, size of buffer

	if (cbFieldName == 0)								//  transport sends an addr; need NULL
		pFieldName = NULL;

	pMsg->GetSgl( MODIFY_BITS_MSG_FIELD_MASK_SGI,		//  index of the SG List to row Defs 
				  &pFieldMask, &cbFieldMask);			//  pointer, size of buffer

	// extract parameters and call ModifyBitsField.
	status = ModifyBitsInField((char*)ptableName,		// Name of the table.
					  (char*)pKeyName, 					// pointer to key name
					  (uchar*)pKeyValue,				// pointer to key value
					  cbKeyValue,
					  (char*)pFieldName,				// pointer to key name
					  (U32*)pFieldMask,					// pointer to key value
					  pModifyBitsPayload->opFlag,
					  pMsg,
					  &cRowsModify);					// number of IDs returned

	pModifyBitsPayload->cRowsModifiedRet = cRowsModify;

	Reply(pMsg, status, true);
 	return OS_DETAIL_STATUS_SUCCESS;
}


STATUS DdmPTS::PtsTestSetField(Message *pMsg)
{

	STATUS   status;
	char					*ptableName;			// Null terminated name of the table.
	U32						cbTableName;		
													// Request parameters
	void					*pKeyName;				//  array key field names
	U32						cbKeyName;				//  size of array
	void					*pKeyValue;		
	U32						cbKeyValue;
	void					*pFieldName;			//	array of fieldnames of modified data
	U32						cbFieldName;		
														// Reply parameters
	TestAndSetFieldPayload 	*pTestSetFieldPayload;
	// Get a pointer to the message payload.
	pTestSetFieldPayload = (TestAndSetFieldPayload*)pMsg->GetPPayload();

	pMsg->GetSgl( TEST_SET_FIELD_MSG_TABLENAME_SGI,		//  index of the SG List to tablename 
				  (void**)&ptableName, &cbTableName);	//  pointer, size of buffer
		
	pMsg->GetSgl( TEST_SET_FIELD_MSG_KEY_NAME_SGI,		//  index of the SG List to field Defs 
				  &pKeyName, &cbKeyName);				//  pointer, size of buffer

	if (cbKeyName == 0)									//  transport sends an addr; need NULL
		pKeyName = NULL;

	pMsg->GetSgl( TEST_SET_FIELD_MSG_KEY_VALUE_SGI,		//  index of the SG List to field Defs 
				  &pKeyValue, &cbKeyValue);				//  pointer, size of buffer

	pMsg->GetSgl( TEST_SET_FIELD_MSG_FIELD_NAME_SGI,	//  index of the SG List to fiedl Defs 
				  &pFieldName, &cbFieldName);			//  pointer, size of buffer

	if (cbFieldName == 0)								//  transport sends an addr; need NULL
		pFieldName = NULL;

	// extract parameters and call TestAndSetField.
	status = TestAndSetOrClearField((char*)ptableName,	// Name of the table.
				  pTestSetFieldPayload->opSetOrClear,	// flag 	
				  (char*)pKeyName,						// pointer to key name
				  (uchar*)pKeyValue,					// pointer to key value
				  (U32)cbKeyValue,
				  (char*)pFieldName,					// pointer to field name
				  &pTestSetFieldPayload->fTestRet);		// return boolean value

	Reply(pMsg, status, true);
 	return OS_DETAIL_STATUS_SUCCESS;
}

STATUS DdmPTS::PtsDeleteRow(Message *pMsg)
{
	STATUS   status;
	char					*ptableName;		// Null terminated name of the table.
	U32						cbTableName;
												// Request parameters
	void					*pKeyName;			//  array key field names
	U32						cbKeyName;			//  size of array
	void					*pKeyValue;		
	U32						cbKeyValue;
	DeleteRowsPayload		*pMyDeleteRowsPayload;

	// Get a pointer to the message payload which is the table name.
	pMyDeleteRowsPayload = (DeleteRowsPayload*)pMsg->GetPPayload();

	pMsg->GetSgl( DELETE_ROWS_MSG_TABLENAME_SGI,				//  index of the SG List to tablename 
				  (void**)&ptableName, &cbTableName);			//  pointer, size of buffer
		
	pMsg->GetSgl( DELETE_ROWS_MSG_KEY_NAMES_SGI,				//  index of the SG List to row Defs 
				  &pKeyName, &cbKeyName);						//  pointer, size of buffer
	if (cbKeyName == 0)											//  transport sends an addr; need NULL
		pKeyName = NULL;

	pMsg->GetSgl( DELETE_ROWS_MSG_KEY_VALUES_SGI,				//  index of the SG List to row Defs 
				  &pKeyValue, &cbKeyValue);						//  pointer, size of buffer

  	// extract parameters and call InsertRow.
	status = DeleteRow((char*)ptableName,						// Name of the table.
					  (char*)pKeyName,							// pointer to key name
					  (uchar*)pKeyValue,						// pointer to key value
					  cbKeyValue,
					  pMyDeleteRowsPayload->cRowsToDelete,		// count of rows to delete
					  &pMyDeleteRowsPayload->cRowsDeletedRet);	// number of rows deleted

	Reply(pMsg, status, true);
	return OS_DETAIL_STATUS_SUCCESS;
}

 STATUS DdmPTS::PtsListen(Message *pMsg)
{

	STATUS   status;
												// Request parameters
	char					*ptableName;			// Null terminated name of the table.
	U32						cbTableName;
	void					*pRowKeyName;			//  row keyfield name
	U32						cbRowKeyName;			//  size of rowkeyname
	void					*pRowKeyValue;			//  row keyfield value	
	U32						cbRowKeyValue;			//	size of row keyfield value
	void					*pKeyName;				//  array key field names
	U32						cbKeyName;				//  size of array
	void					*pKeyValue;		
	U32						cbKeyValue;
	uchar					*pRowBufRet = 0;			// Reply parameters
	U32						cbRowBuf = 0;
	ListenPayload			*pListenPayload;

	// Get a pointer to the message payload which is the table name.
	pListenPayload = (ListenPayload*)pMsg->GetPPayload();

	pMsg->GetSgl( LISTEN_MSG_TABLENAME_SGI,				//  index of the SG List to tablename 
				  (void**)&ptableName, &cbTableName);	//  pointer, size of buffer

	pMsg->GetSgl( LISTEN_MSG_ROWKEY_FIELDNAME_SGI,		//  index of the SG List to fieldname 
				  &pRowKeyName, &cbRowKeyName);			//  pointer, size of fieldname

	if (cbRowKeyName == 0)								//  transport sends an addr; need NULL
		pRowKeyName = NULL;

	pMsg->GetSgl( LISTEN_MSG_ROWKEY_FIELDVALUE_SGI,		//  index of the SG List to fieldvalue 
				  &pRowKeyValue, &cbRowKeyValue);		//  pointer, size of buffer
			
	pMsg->GetSgl( LISTEN_MSG_FIELDNAME_SGI,				//  index of the SG List to fieldname 
				  &pKeyName, &cbKeyName);				//  pointer, size of fieldname
 
	if (cbKeyName == 0)									//  transport sends an addr; need NULL
		pKeyName = NULL;
 
	pMsg->GetSgl( LISTEN_MSG_FIELDVALUE_SGI,			//  index of the SG List to fieldvalue 
				  &pKeyValue, &cbKeyValue);				//  pointer, size of buffer

	// extract parameters and call ListenTable
	status = ListenTable((char*)ptableName,				// Name of the table.
						  (char*)pRowKeyName,				// pointer to rowkey name
						  (uchar*)pRowKeyValue,				// pointer to rowkey value
						  cbRowKeyValue,					// size of row key value
						  (char*)pKeyName,					// pointer to key name
						  (uchar*)pKeyValue,				// pointer to key value
						  cbKeyValue,						// size of key value
						  pListenPayload->listenType,		// listen flags
						  pListenPayload->replyMode,		// reply once or multiple
						  pMsg,								// listen message
						  &(pListenPayload->listenerIDRet));// listener ID	

		// There are three SGLs that are not fetched here because of their dynamic feature;
		// They are LISTEN_REPLY_PTABLE_SGI, LISTEN_REPLY_ID_AND_OPT_ROW_SGI, LISTEN_REPLY_LISTENTYPE_SGI,
		// LISTEN_REPLY_LISTENERID_SGI

		// ennumerates and returns in the client's buffer and the count written in the
		// variables specified above

		// Reply to pMsg;  set last reply flag to false.  
	Reply(pMsg, status, false);
	return OS_DETAIL_STATUS_SUCCESS;
} 

 STATUS DdmPTS::PtsStopListen(Message *pMsg)
{
	STATUS   status;
	StopListenPayload		*pMyStopListenPayload;
		
	// Get a pointer to the message payload which is the table name.
	pMyStopListenPayload = (StopListenPayload*)pMsg->GetPPayload();

	// Find the listen request and remove it from queue.
	status = StopListen(pMyStopListenPayload->listenerID);

	// check if any listeners found	

	SendListenReplies();
   
	// Reply to pMsg;  set last reply flag to true.  
	Reply(pMsg, status, true);

	return OS_DETAIL_STATUS_SUCCESS;
} 

//*********************************************************************************
// $Log: /Gemini/Odyssey/DdmPTS/DdmPTS.cpp $
// 
// 44    1/26/00 2:36p Joehler
// Added very long fields for Sherri.
// 
// 43    1/07/00 5:40p Agusev
// Fixed WIN32 build. 
// Is this the last time or what?!
// 
// 42    12/20/99 4:42p Sgavarre
// fix persist to close file after read also;  Reviewed by legger
// 
// 41    11/04/99 1:19p Sgavarre
// Add PERSIST flag; remove debug stuff;  add ClearNonPersistData
// 
// 40    10/28/99 9:48a Sgavarre
// Persist tables to flash;  VLF: if errors, back out changes to tables
// 
// 39    10/06/99 3:08p Sgavarre
// VSS merge did his own thing
// 
// 38    10/06/99 3:00p Sgavarre
// add var len fields
// 
// 36    9/07/99 9:49p Agusev
// Bumped table memory from 400,000 to 700,000 bytes.
// 
// 35    9/04/99 4:38p Agusev
// Bumped up the size of the PTS heap to 400000 bytes
// Added an assert() for the case when the PTS runs out of available heap
// blocks
// 
// 34    8/28/99 5:51p Sgavarre
// add dynamic sgls for return data;  send stop listenerID message replies
// before reply
// 
// 33    8/16/99 3:17p Sgavarre
// fix a win32 parameter
// 
// 32    8/13/99 4:50p Sgavarre
// Add ModifyBits;  Update ddm to later model;  add support for multirow
// operations;  
// 
// 31    7/23/99 2:52p Sgavarre
// Add QuerySetRID, TestSetOrClearField, Table of tables
// 
// 30    7/12/99 1:52p Sgavarre
// Add listen to DeleteTable;  add deletetable by tableId;   Update Ctor
// and Initialize to lastest spec.
// 
// 28    7/01/99 7:04p Hdo
// Add TS_FUNCTION_DELETE_TABLE
// 
// 27    6/30/99 12:54p Mpanas
// Change Tracef to TRACEF and set level 8
// for real trace info
// 
// 26    6/08/99 3:42p Sgavarre
// Add SendListenReplies;
// Update listentable to support all listen types and parameter;  cleanup
// some old global variables
//
// 25    6/08/99 8:04p Tnelson
// Fix all warnings...
// 
// 24    5/24/99 10:39a Hdo
// Change pInsertRowReply from pointer to a local variable after PtsProcs
// got a MIPS exception.
// 
// 23    5/11/99 10:53a Jhatwich
// win32
// 
// 22    4/16/99 6:14p Jlane
// Modifications made debugging Listen.
// 
// 21    4/05/99 7:18p Jlane
// SERVEVIRTUAL instead of SERVELOCAL.
// 
// 20    4/03/99 6:32p Jlane
// Multiple changes to interface, messaage payloads and message handlers
// to avoid having to add reply payloads as this destroys SGL items.  We
// do this by allocating space for reply data in the msg payload.
// 
// 19    4/03/99 6:24p Jlane
// In SearchListenQueue Don't return ercNoLIsteners it's not a crime not
// to listen.
// 
// 18    3/30/99 4:08p Ewedel
// Added the SERVELOCAL() macros which must replace Serve()s.  Silly me.
// 
// 17    3/30/99 3:52p Ewedel
// Removed newly-obsolete Serve() calls from DdmPTS::Initialize().
// 
// 16    3/29/99 7:13p Ewedel
// Cleaned up to work with latest CHAOS drop.  Also removed a relic or two
// from bygone eras.
// 
// 15    3/27/99 5:03p Sgavarre
// Moved some of the globals back to local 
// 
// 14    3/27/99 1:01p Sgavarre
// Add listen, StopListen;  need to remove global debug
// 
// 13    3/12/99 6:22p Mpanas
// Add a way to change the size of the Table allocated
// 
// 12    3/12/99 5:42p Ewedel
// Changed Reply() calls to report honest status.
//
// 02/12/99 jl:	Modify DoWork to have a single switch statement and remove
//				all references to MessagePrivate.  Also convert GetSgl cb
//				parameters to be U32 so that we can find overloaded GetSgl.
// 02/10/99 jl: Change ctor and constructor to take DID as the Parameter.
// 02/06/99 sg: add break at end of case
// 01/25/99 sg: variables relocated for global debug:  temporary 
// 01/20/99 sg: added support for ModifyField, ModifyRow, DeleteRow, ReadRow
// 12/01/98 sg: add classname macro,  didPTS
// 11/06/98 sg: fixup sgls, add initialization procedures
// 10/22/98 sgavarre: connect Ddm to table procedures;  initialize structures
// 10/05/98	JFL	Created DdmPTS.cpp/h from DdmNull.cpp/h.  Thanks Joe!
// 8/17/98 Joe Altmaier: Create file
/*************************************************************************/
