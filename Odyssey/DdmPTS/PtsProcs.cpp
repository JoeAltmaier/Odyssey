/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: PtsProcs.c
// 
// Description:
// This module implements the procedures used by the persistent table
// service.			
//
/*************************************************************************/

//#define PTSDEBUG

#include "TableMsgs.h"
#include "DdmPTS.h"
#include "Odyssey_Trace.h"


/*************************************************************************/
//  Global definitions
//
/*************************************************************************/

/// debug only : count of operations

U32 cListens = 0;
U32 cDeleteRow = 0;
U32 cInsertRow = 0;
U32 cInsertVLRow = 0;
U32 cModifyRow = 0;
U32 cListenFound = 0;
U32 cDefineTables = 0;

	extern tableHdrDef	*pHeaders;					// chunk of memory with headers
	extern pTableHeader pFreeHeaders;				// temp pointer to free list of headers
	extern pTableHeader pListHeaders;				// temp pointer to list of current headers

    //extern U32			cbrgDataBlks;				// sizeof memory buffer
	extern pDataHeapHdr	prgDataBlks;				// pointer to the memory buffer

	extern pdataBlkHdr	pFreeBlk;					// temp pointer to free data block list	

#ifdef PTSDEBUG
	dbgBufEntry		*pDebugBuffer;
	dbgBufEntry		*pNextEntry;
#endif

	listenerIdMap		*pFreeListenerId;			// ptr to current free listener IDs
	listenerIdMap		*pListenerIds;				// pointer to first block of listenerIDs
	listenReply			*pListenReplies;			// list of listen replies 


   //  table for mapping from data type (fieldType enum in PtsCommon.h)
   //  to the number of bytes needed for the type's representation
   //  [ewx, 3/11/99]
   //  *** be sure to keep this table in sync with the enum fieldType,
   //  *** found in Include\PtsCommon.h.  It is what indexes this table.

	U32   acbDataTypeSize []  =  {
            sizeof (U32),        // BINARY_FT
            sizeof (S32),        // S32_FT
            sizeof (U32),        // U32_FT
            sizeof (S64),        // S64_FT
            sizeof (U64),        // U64_FT
            16,                  // STRING16_FT  (STRING16 unavail)
            32,                  // STRING32_FT  (STRING32 unavail)
            64,                  // STRING64_FT  (STRING64 unavail)
            sizeof (rowID),      // ROWID_FT
			sizeof (U32),		 // BOOL_FT
			sizeof (S32),		 //	VDN_FT   
			32,					 // USTRING_16
			64,					 // USTRING_32
			128,				 // USTRING_64
			256,				 // USTRING_128
			512,				 // USTRING_256
			sizeof (DID)		 // DID
	};


   //  Table for mapping table operation to listen flags
   // *** This table must be kept in sync with the enum tableOpType 
   // *** int include\PtsCommon.h as it is the index to this table	

	U32   rgOpToListenFlags []  =  {
            NULL,					// NOOP
            lFlagDefineTable,		// DefineTable
            lFlagInsertRow,			// InsertRow
            lFlagDeleteRow,			// DeleteRow
            lFlagModifyRow,			// ModifyRow
            lFlagModifyField,  		// ModifyField
			lFlagDeleteTable		// DeleteTable
   };

  //  Table for mapping index of heap header to the size of entries

	U32   rgiHeapToSize []  =  {
            24,			// 24 bytes hdr + 24 bytes data
            40,			// 24 bytes hdr + 40 bytes data
            72,			//  ""			+ 72 bytes data	
            132,		// 132 bytes data
            264,		// 264 bytes data
            520,		// 520 bytes data
			400000,		// 400000,		// image support
			600000		// 600000
   };
	U32   rgiHeapToCountEntries []  =  {
          	10,			// 16 byte heap
          	10,			// 32 byte heap
          	10,			// 64 byte heap	
          	10,			// 128 byte heap
          	10,			// 256 byte heap
          	10,			// 512 byte heap
 			 1,			// 
			 1			 };


#ifdef PTSDEBUG
inline void AddDebugEntry(dbgBufEntry *pDbgEntry)
{
	*pNextEntry = *pDbgEntry;
	pNextEntry = (dbgBufEntry *)((U32)pNextEntry + sizeof(dbgBufEntry));
	if (((U32)pNextEntry - (U32)pDebugBuffer) >= cbDebugBuffer)
		pNextEntry = pDebugBuffer;
}
#endif


/*************************************************************************/
//
//		PTS table procedures
//
/*************************************************************************/

/*************************************************************************/
//
//	Utility routines used by the procedures of the service
//
//	AllocFromTableHeap
//	GetTableHeader
//	GetListenHeader
//	ReturnListenHeader
//	FindField
//	CompareField
//	SearchListenQueue
//	SearchForListenerID
//	ReturnListenHeader
//	CompareRows
//	FieldMatch
//
/*************************************************************************/
//
//	AllocFromTableHeap
//
//  For now, allocate data from a heap;  each chunk of data will be defined
//	with a header; A pointer to the header + data chunk is returned.
//
//  The header contains an offset to the next data block for this table (if it
//	has been expanded), and the size of the data in the block;
//
//	Called from DefineTable when the table is creates and from InsertRows
//	when the table needs to be expanded.
//
//	pFreeBlk - global pointer to the free chunk;  this is stored as an offset in
//		the header of the data blocks
//
//	The data heap will become a chunk of ram the is mapped into the flash. 
/*************************************************************************/


	dataBlkHdr* AllocFromTableHeap
				(U32 cbEntry,		// cbEntry - count of bytes in an entry
				 U32 cEntries)		// cEntries - count of entries to allocate

	{
		dataBlkHdr	*pNextBlk;
		U32			cbAlloc, i, oCurrentEntry;
		entryDef	*pTmp;

		// for now, just allocate the table area from the chunk;
		//	at some point there will be a real free list of memory
		cbAlloc = cbEntry * cEntries;
		// add the size of the data block header
		cbAlloc += sizeof(dataBlkHdr) - sizeof(entryDef);	

		if ((pFreeBlk != NULL) && (cbAlloc < pFreeBlk->cbBlock)) 
		{
			pNextBlk = pFreeBlk;

			pFreeBlk= (dataBlkHdr*)((U32)pNextBlk + cbAlloc);
			pFreeBlk->cbBlock = pNextBlk->cbBlock - cbAlloc;
			pFreeBlk->oNext = 0;
			prgDataBlks->oFreeBlk = (U32)pFreeBlk - (U32) prgDataBlks;
			// count includes the datablkhdr
			pNextBlk->cbBlock = cbAlloc;				
			// clear memory of block
			memset(&(pNextBlk->data), 0, cbEntry * cEntries);			
			// link the row entries in the data block table (free list)

			// count includes the datablkhdr
			pTmp = (entryDef*)&(pNextBlk->data);			
			// for first entry the offset of previous will be 0
			oCurrentEntry = 0;							
			for (i=0; i < cEntries; i++)
			{
				pTmp->oPrevEntry = oCurrentEntry;
 				// offset of the entry from top of data blob
				oCurrentEntry = (U32)pTmp - (U32)prgDataBlks; 
				// is this the last entry?
				if (i == cEntries-1)					
					pTmp->oNextEntry = 0;
				else
					pTmp->oNextEntry = oCurrentEntry + cbEntry;

				pTmp = (entryDef*)((U32)pTmp + cbEntry);	// pointer to next entry
			}

			return(pNextBlk);
		}

	// out of table space		   
	  assert(0);
	  return (NULL);

	}	// AllocFromTableHeap


/*************************************************************************/
//
//  GetTableHeader
//		Searches the table headers for a tablename or tableID  match. 
//		
//		Return values:
//			If the table exists, returns pointer to the table header with
//				the status ercTableExists;
//			If the table does not exist, it returns	status ercTableNotFound
//
//		Searches using either table name string or the table ID;  To search
//		for table by Id, tableName must be null.
//
//
/*************************************************************************/

	
    int GetTableHeader (char* ptName,					// tablename
						rowID *pTableId,				// tableID
    					pTableHeader *pCurrentHdrRet)	// location where table hdr pointer returned 

	{
		BOOL lastHdr = false;
		BOOL found = false;
 		pTableHeader	pTempHdr = pListHeaders;		
 	
		// pListHeaders points to the beginning of the list of table headers;
		// If no tables are defined, this pointer = 0
		
		if ((ptName == NULL) && (pTableId == NULL))
			return (ercBadParameter);
			
		if (pTempHdr == 0)
			lastHdr = true;

		while ((!found) && (!lastHdr))
		{
			if (ptName != NULL)  
			{
				if (strcmp(pTempHdr->name, ptName) == 0) 
					found = true;
			}
			else if (pTableId != NULL)
			{
				if (pTempHdr->rowId.Table == pTableId->Table)
					found = true;
			}
			if (!found) 
			{	// get next header 

				if (pTempHdr->oNext == 0)					// last header?
					lastHdr = true;
				else										// 
					pTempHdr = (pTableHeader) ((U32)pHeaders + pTempHdr->oNext);
			}
		}

		// if table header found, return a pointer to the header
		if (found)
		{
			*pCurrentHdrRet = pTempHdr;
			return (ercTableExists);
		}
		// else	
		*pCurrentHdrRet = NULL;
		return (ercTableNotfound);

	}  // GetTableHeader



/*************************************************************************/
//
//  GetListenHeader - allocates a listen header and a listenerId  
//		for the listen request.  The listenerId is added to the listener
//		header.  This id will be used by StopListen to find the table of
//		the listener.
//
//		pCurrentHdrRet - location where pointer to listen hdr returned
//		pTableId - pointer to rowId of table to listen on 
//			if 0, table has not been defined yet		
//		listenFlags - information stored in listenerId to use later
//			for possible list of listeners
//
//		pFreeListenerId - pointer to list of free listener Ids;  if there
//			are no available headers on the list, allocate another chunk
//			by calling InitializeListenerIDs
//
//		If there are no headers, return ercNoFreeHeaders
//
/*************************************************************************/

	
    int GetListenHeader (pListenMsgHdr *pCurrentHdrRet,	// location where table hdr pointer returned
						 rowID *pTableId,				// table id of table to listen on
						 U32	listenFlags)			// listen flags that define the request

	{
			
		U32 	erc = ercOK;
		*pCurrentHdrRet = new listenMsgHdr;				// deleted in returnListenHeader
		if (*pCurrentHdrRet == NULL)
			return (ercNoMoreHeap);
			
		if (pFreeListenerId == NULL)
		{
			// if we have run out of listener IDs, get some more...
			erc = InitializeListenerIDs (CLISTENERIDS, AUGMENT);
		}
		if ((erc == ercOK)  && (pFreeListenerId != NULL))
		{
			// put listenerID in the listen block
			(*pCurrentHdrRet)->listenerID = pFreeListenerId->listenerID;
			pFreeListenerId->tableId = *pTableId;		// put tableId in listenerID
			pFreeListenerId->listenMode = listenFlags;	// set flags for info purposes
			pFreeListenerId = pFreeListenerId->pNext;	// update pointer to next block
		}
		return (erc);
	}


//************************************************************************
//
//	SearchForListenerID
//		Search ListenerID blocks for the given listenerID.  Return the
//		address of the block; 
//		Used by ReturnListenerID and StopListen before returning IDs
//
/*************************************************************************/


  int SearchForListenerId( listenerIdMap **pListenerIdRet,// address where pointer to listenerId returned
						   U32 listenerId)				  // current listenerID block to return
	

  {
	U32 i = 0;
	int erc = 0;
	listenerIdMap	*pCurrentListenerId;
	U32	cTotalListenerIds;

	*pListenerIdRet = NULL;
	pCurrentListenerId = pListenerIds;		// first header
	cTotalListenerIds = pCurrentListenerId->listenerID;

	//  search for the header of id array
	while (listenerId > cTotalListenerIds) 
	{
		pCurrentListenerId = pCurrentListenerId->pNext;
		if (pCurrentListenerId == NULL)
			return (ercEOF);					// end of listeners:  something wrong
		cTotalListenerIds += pCurrentListenerId->listenerID;
	}
	i = listenerId;
	// step through the array
	while ((i <= cTotalListenerIds) && ( *pListenerIdRet == 0))
	{
		if (pCurrentListenerId[i].listenMode != FreeListener)
		{
			if (pCurrentListenerId[i].listenerID == listenerId)
				*pListenerIdRet = &(pCurrentListenerId[i]);
		}
		i++;
	}		
	if (*pListenerIdRet == NULL)
		return (ercNoSuchListener);

	return (ercOK);
  }

	  
//************************************************************************
//	
// ReturnListenHeader
//
//	Return the ListenHeader to the heap;  return the id to its free lists
//	If the listener is not queued on a table, then pTableHdr = NULL
//	
/*************************************************************************/

  int ReturnListenHeader (tableHeader	*pTableHdr,			// table where listen queued
						  listenMsgHdr  *pCurrentListenHdr)	// listen header to deallocate

  {
	  int erc = 0;
	  listenerIdMap	*pCurrentListenerId;

		
		// remove from listener queue of the table
		if (pCurrentListenHdr->pPrev)
			(listenMsgHdr *)(pCurrentListenHdr->pPrev)->pNext = pCurrentListenHdr->pNext;
		else
		{	// if no previous pointer, then list header must be modified
			pTableHdr->pListenQ = pCurrentListenHdr->pNext;
		}

		if (pCurrentListenHdr->pNext)
			(listenMsgHdr *)(pCurrentListenHdr->pNext)->pPrev = pCurrentListenHdr->pPrev;

		pTableHdr->cListens--;
			
 		// put listener id structure at head of free list

		erc = SearchForListenerId(&pCurrentListenerId, pCurrentListenHdr->listenerID);
 
		if (erc == ercOK)
		{
			pCurrentListenerId->pNext = pFreeListenerId;	
			pFreeListenerId = pCurrentListenerId;
			pFreeListenerId->listenMode = FreeListener;		// use later for searching current listeners 
		}
		
		delete pCurrentListenHdr;
		
		return (erc);
  }


//********************************************************************************************
//
//	ChkFieldDefs -  Validate FieldDefs equal to sizeof(PtsRecord) **TOM**
//
//
//********************************************************************************************
  
int ChkFieldDefs(const char *pszTableName,const fieldDef *pFieldDefs,U32 cbFieldDefs,U32 cbClass)
  {
        extern U32 acbDataTypeSize[];

        U32 cbTotal = sizeof(rowID);
        for (U32 ii=0; ii < cbFieldDefs/sizeof(fieldDef); ii++) {
                if (pFieldDefs[ii].cbField > 0)
                        cbTotal += pFieldDefs[ii].cbField;
                else {
                        fieldType tField = pFieldDefs[ii].iFieldType;   
                        if ((tField < BINARY_FT) || (tField > VDN_FT)) {
                                Tracef("ERROR: Invalid FieldDef type in field %u of PtsRecord \"%s\"\n",ii,pszTableName);
                        }
                        else
                                cbTotal += acbDataTypeSize[tField];
                }
        }
        if (cbTotal != cbClass) {
                Tracef("ERROR: FieldDefs do not match size of PtsRecord\"%s\"\n",pszTableName);
        }
        return true;
}



/*************************************************************************/
//
//  FindField - returns the offset of the field within the row, and the
//		index of the field.  The offset is from the beginning of the row
//		information, which includes the pointers to the next and previous
//		entries.  To get the offset of the field from the beginning of the
//		row data (rowID), need to subtract the size of the entryHdr. 
//
//	This routine is currently called from CompareField and a few
//	other routines.  In a few cases the functionality is duplicated when
//	this routine is called before CompareField;  this should be fixed
//
/*************************************************************************/

 
int FindField (tableHeader *pCurrentHdr,	// header of table to search 
			   char *pKeyName,				// key field name to find
			   U32 *poFieldRet,				// returned byte offset of field in row
			   U32 *piFieldRet)				// returned index of field in rgEntryDef

{	
	U32			iField, oField;
	BOOL		found;
//	entryDef	*pCurrentEntry;
	

  	// get a pointer to the first row entry in the table

	//pCurrentEntry = (entryDef*)((U32)prgDataBlks + pCurrentHdr->oFirstEntry);
	found = false;
	iField = 0;
	
	// offset includes nextentry  and prevEntry pointers;  need to adjust
	oField = sizeof(entryHdr); 

	//  search the array of field descriptions to find the key field
	//  iField is the field index;  oField is the byte offset of the field within the row

	while ((!found) && (iField < pCurrentHdr->cFields) )
		if (strcmp(pCurrentHdr->rgEntryDef[iField].name, pKeyName) == 0)
			found = true;
		else
		{	oField += pCurrentHdr->rgEntryDef[iField].cbField;
			iField++;
		}

	if (!found)
		return (ercKeyNotFound);

	*poFieldRet = oField;
	*piFieldRet = iField;
	return (ercOK);
}


/*************************************************************************/
//
//  CompareField - searches the table for rows where the given key field
//		contains the given key value.
//		It then returns pointers to the rows that match.  The address
//		returned is that of the beginning of the row data, which is the
//		rowID field.  
//		This routine only works for fixed length fields.
//
/*************************************************************************/

 
int CompareField (tableHeader *pCurrentHdr,		// header of table to search
				  char *pKeyName,				// key field name
				  uchar *pKeyValue,				// key field value to find
				  U32	cbKeyValue,				// size of key value for check...
				  U32 *ppRowRet, U32 cbMax,		// location and size of buffer where row addrs returned
				  U32* pcRowsRet)				// count of rows found

{	
	U32		cRows, cbEntry, cbField, iField, iRow, j, oField, erc;
	BOOL	fCompare, fAllRows;
	entryDef *pCurrentEntry;
	uchar	*pSource, *pDest;
	

	if (pCurrentHdr->oFirstEntry == 0)
		return (ercTableEmpty);

  	// get a pointer to the first row entry in the table

	pCurrentEntry = (entryDef*)((U32)prgDataBlks + pCurrentHdr->oFirstEntry);
	// test for special "all" rows feature...
	fAllRows = false;
	if  ((strcmp(pKeyName, CT_PTS_ALL_ROWS_KEY_FIELD_NAME)) == 0)
		fAllRows = true;
	// otherwise, check for field 
	else
	{	// if this is not 'all rows' then must have a key value 
		if (pKeyValue == NULL)
			return (ercBadParameter);
		if ((erc = FindField(pCurrentHdr, pKeyName, &oField, &iField)) != ercOK)
			return (erc);
		if ((pCurrentHdr->rgEntryDef[iField].persistFlags & VarLength_PT) != 0)
			return (ercVlfNotSupported);
		cbField = pCurrentHdr->rgEntryDef[iField].cbField;
		if (cbKeyValue != cbField)
			return(ercIncorrectFieldSize);
	}
	cbEntry = pCurrentHdr->cbEntryData;
	iRow = 0;							// row counter for iteration
	cRows = cbMax / sizeof(pcRowsRet);	// maximum number of row ptrs that fit in ret buf
	*pcRowsRet = 0;

	// if the key name is rowid, and the tableID in the key value is null, stuff the tableId
	// into the key value for the search.

	if  ((strcmp(pKeyName, CT_PTS_RID_FIELD_NAME)) == 0)
	{
		if (((rowID *)pKeyValue)->Table == 0)
			((rowID *)pKeyValue)->Table = pCurrentHdr->rowId.Table;
	}

	// Search each row of the table for the key field until the end of the table,
	// or end of return buffer

	while ((iRow < pCurrentHdr->cCurrentEntries) && (cRows))
	{
		pDest =  (uchar*)pCurrentEntry + oField;
		pSource = pKeyValue;
		j = 0;
		fCompare = true;

		// if the key is "all rows" or there is a real match...
		if  (fAllRows == true) 
			fCompare = true;
		else
			if ((memcmp (pDest, pSource, cbField)) == 0)
				fCompare = true;
			else
				fCompare = false;	 
      
  		// If a match was found, write the address of the row to the return buffer

		if (fCompare)
		{  
			pSource = (uchar*)&(pCurrentEntry->rowId);
			*ppRowRet = (U32)pSource;
			ppRowRet++;					// increment by the size of a pointer
			(*pcRowsRet)++;
			cRows--;
		}
		pCurrentEntry = (entryDef*)((U32)prgDataBlks + pCurrentEntry->oNextEntry);
		iRow++;
	}

	return (ercOK);

}  // CompareField
 
/*************************************************************************/
//
//  ClearNonPersistFields - sets fields to zero that do not persist.
//
/*************************************************************************/

 
void ClearNonPersistFields (tableHeader *pCurrentHdr)	// header of table to search 

{	
	U32			iField, oField, iRow;
	BOOL		fDID, fVarLen;
	entryDef	*pCurrentEntry, *pFirstEntry;
	DID			*pDIDfield;
	varLenFieldDef *pVarLenDef;						// PTS VLF struct in actual row
	heapEntryDef *pCurrentHeapEntry;

  	// get a pointer to the first row entry in the table

	pFirstEntry = (entryDef*)((U32)prgDataBlks + pCurrentHdr->oFirstEntry);
	iField = 0;
	
	// offset includes nextentry  and prevEntry pointers;  need to adjust
	oField = sizeof(entryHdr); 

	//  search the array of field descriptions to find any non persistent fields
	//  iField is the field index;  oField is the byte offset of the field within the row

	while (iField < pCurrentHdr->cFields) 
	{
		iRow = 0;
		fDID = false;
		fVarLen = false;

		if (((pCurrentHdr->rgEntryDef[iField].persistFlags & Persistant_PT) != Persistant_PT))
		{	// non persist field found;  clear it in all rows
			if (pCurrentHdr->rgEntryDef[iField].iFieldType == DID_FT)
			{	// if field type DID... set to DIDNULL...
				fDID = true;
			}
			else if ((pCurrentHdr->rgEntryDef[iField].persistFlags & VarLength_PT) != 0)
			{  
				fVarLen = true;
			}

			pCurrentEntry = pFirstEntry;

			while (iRow < pCurrentHdr->cCurrentEntries) 
			{	// clear the field and advance pointer to next row
				if (fDID)
				{	// set did fields to didNULL
					pDIDfield = (DID *)((U32)pCurrentEntry + oField);
					*pDIDfield = DIDNULL;
				}
				else if (fVarLen)
				{  // clear any non persistent var len fields
					pVarLenDef = (varLenFieldDef *)((U32)pCurrentEntry + oField);
					pCurrentHeapEntry = (heapEntryDef*)((U32)prgDataBlks + pVarLenDef->oHeapEntry);
					memset((uchar *)pCurrentHeapEntry, 0, pVarLenDef->cbField);
				}
				else
					// otherwise, just clear standard field...
					memset((uchar *)((U32)pCurrentEntry + oField), 0, pCurrentHdr->rgEntryDef[iField].cbField);
				
				pCurrentEntry = (entryDef*)((U32)prgDataBlks + pCurrentEntry->oNextEntry);
				iRow++;	
			}
		}	
		// update offsets to next field
		oField += pCurrentHdr->rgEntryDef[iField].cbField;
		iField++;
	}

}

/*************************************************************************/
//
//  CompareRows - compares the current row to the new row from ModifyRow.
//	   Returns an array of fieldnames of the rows that have been modified.  
//
/*************************************************************************/

 
int CompareRows	( tableHeader *pCurrentHdr,		// header of table to search
				  uchar		*pCurrentRow,		// current row
				  uchar		*pNewRow,			// new modified row
				  char		*prgFieldSpec,		// pointer to array of modified fields
				  U32		*pcModifiedFields)	// number of fields modified

{	
	U32  i, k;
	BOOL fCompare;

	k = 0;
	*pcModifiedFields = 0;
	fCompare = true;
	*pcModifiedFields = 0;


	// step through the field definition array, comparing each field of the rows
	for (i = 0; i < pCurrentHdr->cFixedLenFields; i++)
	{	// compare contents of field 
		if (memcmp(pCurrentRow, pNewRow, pCurrentHdr->rgEntryDef[i].cbField))
		{	
			// if does not match copy the field name into the array for listen
			strcpy (prgFieldSpec, pCurrentHdr->rgEntryDef[i].name);
			prgFieldSpec += sizeof(pCurrentHdr->rgEntryDef[i].name);
			(*pcModifiedFields)++;
		}
		// bump the row pointers to the next field
		pCurrentRow += pCurrentHdr->rgEntryDef[i].cbField;
		pNewRow += pCurrentHdr->rgEntryDef[i].cbField;
	}
	
	return (ercOK);

} // CompareRows

//*********************************************************************************/
//
// IsThisDuplicateEntry 
//
//	Every table must define one field to be a non duplicate field where every
//	row has a unique value.  This routine finds the non duplicate field, compares
//	it to the same field in the row being inserted, and if a match, makes sure
//	that the entire row compares.
//  
//	Three possible statuses can result:
//		ercOkDuplicateNotFound - there are no matching non duplicate fields
//		ercOkDuplicateRowFound - a duplicate row was found
//		ercDuplicateFieldFound - a matching nonDuplicate field was found, but row
//				does not compare:  this is an error condition and should not happen
//
//*********************************************************************************/

STATUS IsThisDuplicateEntry ( tableHeader	*pCurrentHdr,	// current table header
						      uchar			*pRow)			// row to check for duplicate
							  
{
 	U32  iField = 0;
	U32	 i;
	U32	 oField = 0;

	BOOL foundDuplicateFlag = false;
	BOOL foundMatchingField = false;
	BOOL foundMatchingRow = false;
	uchar	*pCurrentEntry, *pDest;				// uchar pointer to entry

	// if the table is empty, no chance of a duplicate...
	
	if (pCurrentHdr->oFirstEntry == 0)
		return (ercOkDuplicateNotFound);

  	// get a pointer to the first row entry in the table

	pCurrentEntry = (uchar *)prgDataBlks + pCurrentHdr->oFirstEntry;

	// offset includes nextentry  and prevEntry pointers;  need to adjust
	//oField = sizeof(((entryDef*)pCurrentEntry)->oNextEntry) + sizeof(((entryDef*)pCurrentEntry)->oPrevEntry); 

	// step through the field definition array, to find the nonDuplicate field
	while ((!foundDuplicateFlag) && (iField < pCurrentHdr->cFixedLenFields))
	{
		if ((pCurrentHdr->rgEntryDef[iField].persistFlags & NonDuplicate_PT) != 0)
			foundDuplicateFlag = true;
		else
		{	// continue looking... update offset
			oField += pCurrentHdr->rgEntryDef[iField].cbField;
			iField++;
		}
	}
	// now check for matching field and row   
	if (foundDuplicateFlag == true)
	{	
		i = 0;
 		while ((i < pCurrentHdr->cCurrentEntries) && (foundMatchingField == false))
		{
			pDest = (uchar *) (&(((entryDef *)pCurrentEntry)->rowId));	// pointer to row data		
			// does the field value match?
			if ((memcmp ((pDest + oField), (pRow + oField),
						  pCurrentHdr->rgEntryDef[iField].cbField)) == 0)
				foundMatchingField = true;		// yes, so go compare the entire row...
			else
				pCurrentEntry = ((uchar *)prgDataBlks) + ((entryDef *)pCurrentEntry)->oNextEntry;
			i++;
		}
 		if (foundMatchingField == true)
 		{	// if the matching field is found, the row must also match 
 			if ((memcmp (pDest + sizeof(rowID), pRow + sizeof (rowID),
						  (pCurrentHdr->cbFixedLenData - sizeof (rowID)))) == 0)
			{
				foundMatchingRow = true;
				return (ercOkDuplicateRowFound);  
			}	
			else
				// if it does not, there is a duplicate field value in a nonDuplicate field
				return (ercDuplicateFieldFound);					
		}
	}	
	// two conditions will end up here... a matching field is not found or a nonDuplicate field is not found
	//	the first condition is normal operation...  the second we may want to check later	
	return (ercOkDuplicateNotFound);
}

	
/*************************************************************************/
//
//  FieldMatch - finds a specified field in a given row and compares its 
//	   new value with that of the listener.  
//
/*************************************************************************/

 
BOOL FieldMatch	( tableHeader	*pCurrentTableHeader,	// header of table to search
				  uchar			*pModifiedRow,			// current row
				  char			*pKeyName,				// field name to find in row
				  uchar			*pKeyValue)				// field value to find

{	
	U32  oField, iField;
	BOOL foundField;
	
	oField = 0;
	iField = 0;
	foundField = false;

	if (pModifiedRow == NULL)
		return foundField;

	// search the field definitions for the field that will identify the row
	while (iField < pCurrentTableHeader->cFixedLenFields) 
	{	
		if (strcmp(pCurrentTableHeader->rgEntryDef[iField].name, pKeyName) != 0)
			// no compare, bump offset to next field
			oField += pCurrentTableHeader->rgEntryDef[iField++].cbField;
		else
		{
			// Is the row key value of the listener the same as the row changed?
			if ((memcmp (pKeyValue, pModifiedRow + oField,
						 pCurrentTableHeader->rgEntryDef[iField].cbField )) == 0)
				foundField = true;
			break;
		}
	}
	
	return (foundField);
}	 


/*************************************************************************/
//
//  SearchListenQueue - searches the listen queue of a given table header
//		for a listener with similar listen attributes.
//
//	Each table header has a queue of listeners for that table.  The queue 
//	of listeners for tables not yet defined lives in header of TableOfTables.
//
//	pCurrentListenHdr is a pointer to the list to be searched.  If a listener
//  is found, it gets the listen message from the queue, puts it in
//	a list of messages to send (rgListenReplies).  The list entry includes the pMsg,
//	the rowID, and whether it is a once only reply.  This list is processed after
//	all of the listen queues are searched.
//	
//	If this is a once only reply, delete listen header from queue.
//
//	If a listener is not found, ercNoListeners is returned.
//
/*************************************************************************/

 
int SearchListenQueue ( tableHeader *pCurrentTableHdr,	// table header
						uchar		*pModifiedRow,		// row that has been modified
						char		*pKeyField,			// key field(s) that was modified
						U32			cKeyField,			// number of entries in array of key field
						tableOpType	tableOp)			// table operation that just modified a table 
{
	

	U32				erc, i, status;
	BOOL			foundListener;
	listenMsgHdr	*pCurrentListenHdr, *pNextListenHdr;
	uchar			*pRowBufRet;
	U32				cbRowBuf, listenType;
	listenReply		*pNextListenReply;					// listen reply to be queued 
	ListenPayload	*pListenPayload;
 	char			*pModKeyField;
	entryDef		*pRowEntry;

	cListenFound = 0;
	// listen requests for tables not yet defined are on a queue in the tableOfTables header
	//	all other listen requests are on the queue in the header

	pCurrentListenHdr = pCurrentTableHdr->pListenQ;

	// Search the queue for a listen header that matches the listenFlags
	while (pCurrentListenHdr != NULL)
	{
	  foundListener = false;
	  i = 0;
	  pNextListenHdr = pCurrentListenHdr->pNext;		// save the next listener in case this one deleted
	  // test listeners for the type of function just completed
	  // If a DeleteTable was done, return all listeners on queue in table header

	  if (tableOp == OpDeleteTable)
		  listenType = ListenReturnedOnDeleteTable;
	  else
	  	  	// save listen type for reply
		listenType = (pCurrentListenHdr->listenMode) & (rgOpToListenFlags[tableOp]);

 	  if (listenType != 0) 
	  { 
		// header found : pCurrentListenHdr 
		  switch (listenType)
		  {
			case ListenOnDefineTable:
			{
				foundListener = true;
				break;
			}
			case ListenReturnedOnDeleteTable:
			{
				foundListener = true;
				break;
			}
			case ListenOnInsertRow:
			{
				foundListener = true;
				break;
			}
			case ListenOnDeleteAnyRow:
			{
				foundListener = true;
				break;
			}
			case ListenOnDeleteOneRow:
			case ListenOnModifyOneRowAnyField:	
			// Row Key of listener must match a field and field value within modified row;
			// do not care which field was modified
			{
				foundListener = FieldMatch (pCurrentTableHdr, pModifiedRow, 
											pCurrentListenHdr->rowKeyName,
											pCurrentListenHdr->rowKeyValue); 
				break;
			}	

			case ListenOnModifyAnyRowOneField:
			{	// Field key of listener must match the field modified
				pModKeyField = pKeyField;			// pointer to move thru array
				while (( !foundListener) &&  (i < cKeyField))
				{	// is the field(s) modified the one being listened to?
					if (strcmp(pCurrentListenHdr->fieldName, (char*)pModKeyField) == 0)
					{	// modified field matches listener field 
						foundListener = true;
						// but, is there a value defined?
						if (pCurrentListenHdr->cbFieldValue != 0)
						{	// get new field value and compare to desired value
							if ((FieldMatch (pCurrentTableHdr, pModifiedRow, (char*)(pCurrentListenHdr->fieldName),
											pCurrentListenHdr->fieldValue)) != true)
							{ 	// not the right value... get out of loop
								foundListener = false;
								i = cKeyField;		   // get out of while
							}
						}
					}
					else // get next field in array...
					{
						pModKeyField+= CBMAXFIELDSPEC; 
						i++;
					}					
				}	// end while
				break;
			}

			case ListenOnModifyAnyRowAnyField:
			{
				foundListener = true;
				break;
			}
			case ListenOnModifyOneRowOneField:	
			{
				// if the rowKey and rowValue of the listen header compare to a field and value 
				//	of the modified row... 
				if (FieldMatch (pCurrentTableHdr, pModifiedRow, pCurrentListenHdr->rowKeyName,
								pCurrentListenHdr->rowKeyValue))
				{  // then check if the field has been modified 
					pModKeyField = pKeyField;		// ptr variable for stepping thru array
					while ((!foundListener) && (i < cKeyField))
					{	// step through the array of modified fields, to see if field modified
						if (strcmp(pCurrentListenHdr->fieldName, (char*)pModKeyField) == 0)
						{	// found a listener for this field
							foundListener = true;
							// but, if a field value is defined, need to check the value also
							if (pCurrentListenHdr->cbFieldValue !=0)
							{	// get new field value and compare to desired value
								if ((FieldMatch (pCurrentTableHdr, pModifiedRow,(char*)(pCurrentListenHdr->fieldName),
									pCurrentListenHdr->fieldValue))	 != true)
								{	// value does not match... exit loop
									foundListener = false;
									i = cKeyField;
								}
							}

						}  // if field not found, get the next field 
						else
						{	// update counter and pointer to modified fields  
							pModKeyField+= CBMAXFIELDSPEC;
							i++;
						}
					}	// end while
				} // end if rowkey matches
				break;
			}

			default:
			{
				erc = ercBadParameter;
				break;
			}

		} // end switch

		if (foundListener)
		{
			// if defineTable, return the tablename if a sgl defined
			if (tableOp == OpDefineTable)
				cbRowBuf = CBMAXTABLESPEC;

			// otherwise, return either the row, or rowID as specified in the reply flags
			else
				if (pCurrentListenHdr->replyMode & ReplyWithRow)
				{ // if want the row back, calculate the size of row
					if ((pCurrentTableHdr->persistFlags & VarLength_PT) != 0)
					{
						cbRowBuf = ((pCurrentTableHdr->cbFixedLenData) + 
							(sizeof(varLenField) * (pCurrentTableHdr->cFields - pCurrentTableHdr->cFixedLenFields)));
						if (cbRowBuf > 8000)
							cbRowBuf = pCurrentTableHdr->cbFixedLenData;
					}
					else
				   		cbRowBuf = pCurrentTableHdr->cbEntryData; 
				}
				else
				{	// or maybe want rowID only
					if (pCurrentListenHdr->replyMode & ReplyWithRowID)
						cbRowBuf = sizeof (rowID); 
					 else
						 cbRowBuf = 0;			
				}
			if (cbRowBuf != 0 )
			{
				// if there is a reply data SGL item then allocate enough for a row or tablename 
				pCurrentListenHdr->pListenMsg->GetSgl( LISTEN_REPLY_ID_AND_OPT_ROW_SGI,// Index of the SG List to row or rowID 
													(void**)(&pRowBufRet),			// Pointer to reply SGL buffer
													&cbRowBuf );					// Size of buffer.
				if ((pRowBufRet != NULL) && (pModifiedRow != NULL))
				{ 	 // if definetable row buffer contains the tablename
					if ((pCurrentTableHdr->persistFlags & VarLength_PT) != 0)
					{ //  get the var len row...
						pRowEntry = (entryDef *)(pModifiedRow - sizeof(entryHdr));
						erc = GetVarLenRow (pCurrentTableHdr, pRowEntry, pRowBufRet, cbRowBuf, false, NULL);
 					}
					else // for all others, just copy the row:  it could be only rowID or the tablename 
						memcpy (pRowBufRet, pModifiedRow, cbRowBuf);
				}
			}
			// Get a pointer to the message payload which is the table name.
			pListenPayload = (ListenPayload*)pCurrentListenHdr->pListenMsg->GetPPayload();
			pListenPayload->listenTypeRet = listenType;

			pNextListenReply = new listenReply;			// deleted in ddmPts::	
			cListenFound++;		// debug flag

			// if list exists, add new listen reply to the beginning of the linked list
			if (pListenReplies != NULL)					// if list exists
				pNextListenReply->pNext = pListenReplies;		
			else
			// otherwise, make reply the first of list
				pNextListenReply->pNext = NULL;
				
			pListenReplies = pNextListenReply;			// list pointer update 

			// new ListenReply will contain the msg and other information for reply

			pNextListenReply->pListenMsg =  pCurrentListenHdr->pListenMsg;

			// if this listen is not multiple reply, or the table was deleted,
			// remove from the listener queue and return to the free list
			
 			if ((pCurrentListenHdr->replyMode & ReplyOnceOnly) || (tableOp == OpDeleteTable))
			{  
				pNextListenReply->lastReply = true;
				// put headers back on free lists: no current errors
				status = ReturnListenHeader (pCurrentTableHdr,  pCurrentListenHdr);
											 
			} //endif replyOnceOnly
			else
 				pNextListenReply->lastReply = false;
  
		} // endif foundListener
	 } // endif listener Flag match
	 
	 pCurrentListenHdr = pNextListenHdr;		// get next listen header in list
	
	}// end while

	// Send the replies found; must be sent for each modification so that data
	// in the dynamic buffers is not overwritten.
	DdmPTS::pDdmPTS->SendListenReplies();

	return (ercOK);
} // end Search

 
/*************************************************************************/
//
//	ExtendTable - allocate a new chunk of data for the table and update the
//		 table header and previous chunk to reflect changes.  This table 
//		 contains the fixed length portion of the row including the structure
//		 that defines the varlen field
//
//************************************************************************
 
dataBlkHdr* ExtendTable(tableHeader *pCurrentHdr)

{
	dataBlkHdr	*pCurrentBlk;
	dataBlkHdr	*pPrevBlk;					// pointer to previous data block

	pCurrentBlk =  AllocFromTableHeap (pCurrentHdr->cbEntry, pCurrentHdr->cEntriesAlloc);

	if (pCurrentBlk == NULL)
		return (pCurrentBlk);

	// update the table header free entry offset
	pCurrentHdr->oFreeEntry = (U32)(&(pCurrentBlk->data)) - (U32)prgDataBlks;

	// get the last data block for this file, and point it to the new one
	pPrevBlk = pCurrentHdr->pTableData;
	// start at the first block... and move through until last one found...
	while (pPrevBlk->oNext != 0)
	{
		U32 pTempBlk = (U32)prgDataBlks + (pPrevBlk->oNext);
		 pPrevBlk = (dataBlkHdr*)(pTempBlk);
	}
	pPrevBlk->oNext = (U32)pCurrentBlk - (U32)prgDataBlks;	
	 
	return (pCurrentBlk);

}

/*************************************************************************/
//
//	ExtendHeap - allocate a new chunk of data for the specific heap
//		This code is same as ExtendTable, except for headers... at some
//		point it could be merged
//
//************************************************************************
 
dataBlkHdr* ExtendHeap(heapHeader *pCurrentHdr)

{
	dataBlkHdr	*pCurrentBlk;
	dataBlkHdr	*pPrevBlk;					// pointer to previous data block

	pCurrentBlk =  AllocFromTableHeap (pCurrentHdr->cbEntry, pCurrentHdr->cEntriesAlloc);

	if (pCurrentBlk == NULL)
		return (pCurrentBlk);

	// update the table header free entry offset
	pCurrentHdr->oFreeEntry = (U32)(&(pCurrentBlk->data)) - (U32)prgDataBlks;

	// get the last data block for this heap, and point it to the new one
	pPrevBlk = pCurrentHdr->pTableData;

	while (pPrevBlk->oNext != NULL)
		  pPrevBlk = (dataBlkHdr*)((U32)prgDataBlks + pPrevBlk->oNext);

	pPrevBlk->oNext = (U32)pCurrentBlk - (U32)prgDataBlks;	
	 
	return (pCurrentBlk);
}


/*************************************************************************/
//
//	ListenTable - Allocates a listen header, fills in information and puts
//		the header on the table's listen queue
//	If the table does not exist yet, and request is for ListenDefineTable
//	then queue in TableOfTables queue
//
//	Returns listenID that can be used to stop listen 
//	Returns ercNoSuchTable for listen requests other than Definetable if
//		table does not exist;  might want to change this later
//
//*************************************************************************/


int ListenTable(char		*ptName,
			  char			*pRowKeyName,	// keyname to find
			  uchar			*pRowKeyValue,	// key value to find
			  U32			cbRowKeyValue,	// row key value size
			  char			*pFieldKeyName,	// keyname to find
			  uchar			*pFieldKeyValue,// key value to find
			  U32			cbFieldKeyValue,// size of key value
			  U32			listenFlags,	// listen mode
			  U32			replyMode,		// reply mode
			  Message		*pListenMsg,	// listener message
			  U32			*pListenerIdRet)// * returned by 1st reply
 

{
  	tableHeader  *pCurrentHdr;
	listenMsgHdr *pNewListenHdr, *pCurrentListenHdr;
	uchar		 *pTableBufRet;				// pb of table returned 
	U32			 cbTableBuf, cbTableBufRet;	// cb of table written to tableBuf
	U32			 status, tableOffset, erc;
	rowID		 tableId;
	U32			 i, cRows, cRowsRet, oField, iField;
	U32		 	 *pRowsRet;
	ListenPayload	*pListenPayload;

cListens++;	
	// if listen on define table, put on tableOfTables queue 
	if ((listenFlags & ListenOnDefineTable) != 0)				
	{
		pCurrentHdr = &(pHeaders->rgHdr[iTOTHeader]);
		tableId.Table = NULL;
		strcpy (ptName, CT_PTS_TABLE_OF_TABLES);
	}
	// otherwise, look for table header;  if does not exist error
	else
	{
		status = GetTableHeader(ptName, NULL, &pCurrentHdr);
		if (status != ercTableExists)
			return (ercTableNotfound);
		tableId.Table = pCurrentHdr->rowId.Table;
	}

	pCurrentListenHdr = pCurrentHdr->pListenQ;
	
 	// zero out possible rowId values from table header
	tableId.LoPart = 0;
	tableId.HiPart = 0;

	// if this is a listen on modify operation, make sure fields are not vlf
	if ((listenFlags & (lFlagModifyRow)) != 0) 
	{
		if (pRowKeyName != NULL)
		{	// check the row key name for vlf...
			erc = FindField(pCurrentHdr, pRowKeyName, &oField, &iField);
			if ((erc == ercOK) && ((pCurrentHdr->rgEntryDef[iField].persistFlags & VarLength_PT) != 0))
				return (ercVlfNotSupported);
		}
		if (pFieldKeyName != NULL)
		{	// now check the field name for vlf...
			erc = FindField(pCurrentHdr, pFieldKeyName, &oField, &iField);
			if ((erc == ercOK) && ((pCurrentHdr->rgEntryDef[iField].persistFlags & VarLength_PT) != 0))
				return (ercVlfNotSupported);
		}
	}

	// Get a free listenHeader and ...
	if ((status = (GetListenHeader(&pNewListenHdr, &tableId, listenFlags))) == ercOK)
	{
	 	pNewListenHdr->pPrev = NULL;					// new header at beginning of list
	 	
	  // put this listen header at the beginning of listen queue in table header
		
	  if (pCurrentListenHdr != NULL)
	  {	
	  	  pNewListenHdr->pNext= pCurrentListenHdr;		// new header at beginning of list
		  pCurrentListenHdr->pPrev = pNewListenHdr;
	  }	  
	  else
		  pNewListenHdr->pNext = NULL;
	
	  pCurrentHdr->pListenQ = pNewListenHdr;
	  pCurrentHdr->cListens++;


	  // save the parameters from the request in the listen header; these parameters are 
	  // used to return information when the actual listen comes through

	  pNewListenHdr->listenMode = listenFlags;
	  pNewListenHdr->replyMode = replyMode;
	  pNewListenHdr->pListenMsg = pListenMsg;

	  if (pRowKeyName != NULL)
		strcpy( pNewListenHdr->rowKeyName, pRowKeyName);

	  if (pRowKeyValue != NULL)
		memcpy( pNewListenHdr->rowKeyValue, pRowKeyValue, cbRowKeyValue );

	  if (pFieldKeyName != NULL)
		strcpy( pNewListenHdr->fieldName, pFieldKeyName);

	  if (pFieldKeyValue != NULL)
		memcpy( pNewListenHdr->fieldValue, pFieldKeyValue, cbFieldKeyValue);
		
	  pNewListenHdr->cbFieldValue = cbFieldKeyValue;
	  
	  // the user may have requested that the table is returned upon registering a listen...
	  if (replyMode & ReplyFirstWithTable)
	  {
		// Prepare to call GetSgl to cause the table reply buffer to be allocated.  
		// First though we need to figure out how big the table buffer should be.
		cbTableBuf	= pCurrentHdr->cbEntryData			// # of bytes of data per entry including rowID
					  * pCurrentHdr->cCurrentEntries;	// # of entries in table
		
		// Call GetSgl to cause the table reply buffer to be allocated.  If there
		// is a reply data SGL item then copy the row (or as much as will fit)
		// to the sgl entry.  Note that normally GetSgl returns the cpount but in
		// the SGL_DYNAMIC_REPLY case we (the server) supply the count.
		pListenMsg->GetSgl( LISTEN_REPLY_PTABLE_SGI,	// Index of the SG List to row or rowID 
							(void**)(&pTableBufRet),	// Pointer to reply SGL buffer
							&cbTableBuf );				// Size of buffer.
		
		// If we got a buffer enumerate the table into it.					
		if ((pTableBufRet != NULL) && (cbTableBuf != NULL))
		{
		  	tableOffset = 0;
			status = EnumerateTable
					 (ptName, tableOffset,				// Name of the table, offset of first row.
				      pTableBufRet, cbTableBuf,			// buffer where rows returned
				      NULL,								// pMsg
					  &cbTableBufRet);					// number of bytes returned in buffer.
		}
		
	  } // endif replyFirstWithTable
	  
	  // or... they may want the rows they are listening on... based on the key
	  else if ((replyMode & ReplyFirstWithMatchingRows) && (pRowKeyName != NULL) && (pRowKeyValue != NULL))
	  {
		cRows = pCurrentHdr->cCurrentEntries;			// count of current rows
		if ((pRowsRet = new U32[cRows]) == NULL)		// for CompareField;  deleted at end
			return (ercNoMoreHeap);
		 
		if ((CompareField(pCurrentHdr, pRowKeyName, pRowKeyValue, cbRowKeyValue,
			 pRowsRet, (sizeof(U32) * cRows), &cRowsRet)==ercOK) && cRowsRet)
		{
			// matches were found...  calculate amount of memory needed from dynamic SGL alloc
			cbTableBuf = (pCurrentHdr->cbEntryData) * cRowsRet; // # of bytes per row * matching rows
		
			// Call GetSgl to cause the reply buffer to be allocated.  If there
			// is a reply data SGL item then copy the row (or as much as will fit)
			// to the sgl entry.  Note that normally GetSgl returns the cpount but in
			// the SGL_DYNAMIC_REPLY case we (the server) supply the count.
			pListenMsg->GetSgl( LISTEN_REPLY_PTABLE_SGI,	// Index of the SG List to row or rowID 
							(void**)(&pTableBufRet),		// Pointer to reply SGL buffer
							&cbTableBuf );					// Size of buffer.
		
			// If we got a buffer, copy the rows into it.					
			if ((pTableBufRet != NULL) && (cbTableBuf != NULL))
			{
				for (i= 0; i < cRowsRet; i++) 
				{	// copy the actual row into the return buffer starting at rowID
					memcpy (pTableBufRet, (uchar*)(pRowsRet[i]), pCurrentHdr->cbEntryData);
				}
			}
			delete [] pRowsRet;
		}
	  } // endif replyFirstWithMatchingRows

		// Get a pointer to the message payload which is the table name.
		pListenPayload = (ListenPayload*)pListenMsg->GetPPayload();
		pListenPayload->listenTypeRet = ListenInitialReply;
		pListenPayload->tableIDRet = pCurrentHdr->rowId;
		pListenPayload->tableIDRet.LoPart = 0;
   
	  // return the listenerID
	  if (pListenerIdRet != NULL)
		*pListenerIdRet = pNewListenHdr->listenerID;

	} // endif getListenHeader

	else
		return (ercNoFreeHeaders);

	return (ercOK);
  
}	// ListenTable


/*************************************************************************/
//
//	StopListen - given a listener id, removes the listen request 
//
//	Find the listenerId entry which maps the ID to a table name.  
//	Remove it from the list, then locate the listen header.  Remove
//	it from the queue and return for reply.
//
/*************************************************************************/


int StopListen(U32	listenerId)		

{
  	tableHeader		*pCurrentTableHdr;
	listenMsgHdr	*pCurrentListenHdr;

	U32			status;
	BOOL		fDefineTable = false;
	BOOL		foundListener = false;
	listenerIdMap	*pCurrentListenerId;
	listenReply		*pNextListenReply;			// listen reply to be queued 
	ListenPayload	*pListenPayload;


	// find the listenerId structure that corresponds to id
	status = SearchForListenerId(&pCurrentListenerId, listenerId);

	if (pCurrentListenerId == NULL)
		return (ercNoSuchListener);

	// listener Id should match, but make sure this listener exists
	if ((pCurrentListenerId->listenerID == listenerId) &&
		(pCurrentListenerId->listenMode != FreeListener))
	{
		// get the table header where the listener is queued
		if (pCurrentListenerId->tableId.Table)
		{
			if (GetTableHeader (NULL, &pCurrentListenerId->tableId, &pCurrentTableHdr) != ercTableExists)
				return (ercTableNotfound);
		}
		else
		{	// listener is queued for a table that does not exist yet
			pCurrentTableHdr = &(pHeaders->rgHdr[iTOTHeader]);
		}
		pCurrentListenHdr = pCurrentTableHdr->pListenQ;

		// Search queue for the listener
		while ((pCurrentListenHdr != NULL) && (!foundListener))
		{
			if (pCurrentListenHdr->listenerID == listenerId)
				foundListener = true;
			else
				pCurrentListenHdr = pCurrentListenHdr->pNext;
		}

		if (foundListener)
		{	
			// Get a pointer to the message payload which is the table name.
			pListenPayload = (ListenPayload*)pCurrentListenHdr->pListenMsg->GetPPayload();
			pListenPayload->listenTypeRet = ListenReturnedOnStopListen;

			// put the listen message on the end of the list of replies so we can do last reply
			// to the listen

			pNextListenReply = new listenReply;			// deleted in ddmPTS	
			pNextListenReply->pNext = NULL;

			if (pListenReplies != NULL)					// if list exists
				pListenReplies->pNext = pNextListenReply;		
			else
				pListenReplies = pNextListenReply;

			// new ListenReply will contain the msg and other information for reply
			pNextListenReply->pListenMsg =  pCurrentListenHdr->pListenMsg;
			pNextListenReply->lastReply = true;

			// put headers back on free lists: no current errors
			status = ReturnListenHeader (pCurrentTableHdr, pCurrentListenHdr);
				
			return (ercOK);
		} // if listener found
	} // else valid listener not found	
	return (ercNoSuchListener);
  
}	// Stop listen



/*************************************************************************/
//
//	DefineTable
//
//  Creates a table header and allocates a chunk of memory from "the chunk"
//	for the table rows.  User defines the table name,  the fields to be 
//	defined, and the expected number of entries.  At some point the tables
//	will be expanded??? as needed.
//
//	Returns the table id as defined in rowID;

// need to return table header when an error encountered
//
/*************************************************************************/


int DefineTable (char *ptName,							// new table name
				 fieldDef *prgFields, U32 cbrgFields,	// array of field definitions for table
				 U32 cEntriesRsv,						// expected number of rows to allocate
				 U32 persistFlags,						// will this table persist?  flags 
				 rowID	*pTableIdRet)					// table Id returned
				 										

{
		U32 i = 0;
		BOOL lastHdr = false;
		BOOL firstVLF = false;
 		pTableHeader	pCurrentHdr;		
 		U32 oCurrentEntry = 0;
		U32 cFields, erc, cbEntry, cbFixedEntry, cRowsInserted;
		dataBlkHdr *pCurrentBlk;
 		rowDef	rowTableOfTables;

cDefineTables++;
	
		if (cEntriesRsv == 0)
			return (ercBadParameter);

		// check to see if table already exists
		
		if (GetTableHeader(ptName, NULL, &pCurrentHdr) == ercTableExists)
		{
			*pTableIdRet = pCurrentHdr->rowId;
			return (ercTableExists);
		}

		// table does not exist, get a header
       //  but first, verify that our internal structures have room
       //  for the requested field defs

		cFields = cbrgFields/sizeof(fieldDef);

		if (cFields > (CMAXENTRIES + 1))      // +1 for implicit rowID
         {
			 return (ercPtsStructsTooSmall);
         }

      //  new table fits in our structs, so see if we can allocate a header

		if (pFreeHeaders != NULL)					// are there any left?
			pCurrentHdr = pFreeHeaders;
		else
			return (ercNoFreeHeaders);

		// Update the offset to the next free header in list, and update the pointer
		//	pFreeHeaders to the same entry

		pHeaders->oFreeHdr = pFreeHeaders->oNext;		
 		pFreeHeaders = (pTableHeader)((U32)pHeaders + pFreeHeaders->oNext); 

		// If this is the first header to be used, set its next link to 0;
		// If not, put it on the front of pListHeaders;
		// Modify oListHdr to reflect new header;
 		
		if (pListHeaders == 0)
			// first header?
			pCurrentHdr->oNext = 0;
		else
			// set link to current list head???
			pCurrentHdr->oNext = (U32)pListHeaders - (U32)pHeaders;	 
	
		// head of list points to this current header
		pListHeaders = pCurrentHdr;			
		pHeaders->oListHdr = (U32)pListHeaders - (U32)pHeaders;
		pHeaders->tableCount++;					// increment unique table count

		// Initialize header
		//  unique rowID:  currently low dword is counter, mid word not used
		//				   high word is table ID
		
		pCurrentHdr->persistFlags = persistFlags;
		pCurrentHdr->rowId.HiPart = NULL;			
 		pCurrentHdr->rowId.LoPart = NULL;
		pCurrentHdr->rowId.Table = (unsigned short)pHeaders->tableCount;
		pCurrentHdr->pListenQ = NULL;

		strcpy((char *)(pCurrentHdr->name),ptName);
		
		// cFields has total # of fields including rowID and vlfs
		pCurrentHdr->cFields = cFields + 1;			
		// cFixedLenFields has count of fixed length fields only
		pCurrentHdr->cFixedLenFields = 1;  
		// add rowID field to the entry definition

		strcpy (pCurrentHdr->rgEntryDef[0].name, CT_PTS_RID_FIELD_NAME);
 		pCurrentHdr->rgEntryDef[0].cbField = sizeof (rowID); 
 		pCurrentHdr->rgEntryDef[0].iFieldType =  ROWID_FT;
		pCurrentHdr->rgEntryDef[0].persistFlags =  Persistant_PT;

 		cbEntry = sizeof (rowID);		
		cbFixedEntry = sizeof (rowID);
		// copy the field definitions into the header
		
		for (i=1; i < pCurrentHdr->cFields; i++)
		{
			strcpy (pCurrentHdr->rgEntryDef[i].name, prgFields[i-1].name);
			// store the fieldtype
			pCurrentHdr->rgEntryDef[i].iFieldType = prgFields[i-1].iFieldType;
			// store the persisttype
			pCurrentHdr->rgEntryDef[i].persistFlags = prgFields[i-1].persistFlags;
			// if var len field, add in size of varlenField struct
			if (((prgFields[i-1].persistFlags) &  VarLength_PT) != 0)
			{
				pCurrentHdr->rgEntryDef[i].cbField = sizeof(varLenFieldDef);
				// add in variable length field flag for table header
				pCurrentHdr->persistFlags |= VarLength_PT;
				cbEntry += pCurrentHdr->rgEntryDef[i].cbField;
				firstVLF = true;		// once this is true, no more nonVarLen fields
			}
			else 
				if (!firstVLF)
				{	//  compute field size, based on requestor's parameters (ewx 3/11/99)
					if (prgFields[i-1].cbField == 0) 
					{ //  definer wants field size to be one instance of given data type
						//BUGBUG - must verify that iFieldType is legal fieldType enum value
						pCurrentHdr->rgEntryDef[i].cbField =
							  acbDataTypeSize [prgFields[i-1].iFieldType];
					}
					else
					{  //  requestor gave us a field size, use it as-is
						pCurrentHdr->rgEntryDef[i].cbField = prgFields[i-1].cbField;
         			}
					//  bump total entry size by computed size of this field
					cbEntry += pCurrentHdr->rgEntryDef[i].cbField;
					cbFixedEntry += pCurrentHdr->rgEntryDef[i].cbField;
					pCurrentHdr->cFixedLenFields++;
				}  //  end if firstVLF
				else
				// if the variable length parameters are not at the end, error
					return (ercBadParameter);
		}//end for
		
		// size of actual row data including rowID and structs for VLFs
		pCurrentHdr->cbEntryData = cbEntry;			
		pCurrentHdr->cbFixedLenData = cbFixedEntry;	// no size of VLF structs			

		// round to dword boundary, then add size of links
		//	cbEntry includes oNextEntry and oPrevEntry
		
 		cbEntry = (((cbEntry + 0x3) & 0xfffffffc) + sizeof(entryHdr));

		// get block of memory from the chunk and initialize header to point to it

		pCurrentHdr->cEntriesAlloc =  cEntriesRsv;
		pCurrentBlk =  AllocFromTableHeap (cbEntry, cEntriesRsv);

		if (pCurrentBlk == NULL)
			return (ercNoMoreDataHeap);
		// initialize offset to next chunk for this table
		pCurrentBlk->oNext = 0;			
		// initialize rest of header
		pCurrentHdr->oTableData  = (U32)pCurrentBlk - (U32)prgDataBlks;
		pCurrentHdr->pTableData = pCurrentBlk;
		pCurrentHdr->cbrgEntryDef = pCurrentHdr->cFields * sizeof(fieldDef);
		pCurrentHdr->cbEntry  = cbEntry;
		pCurrentHdr->oFirstEntry = 0;
		pCurrentHdr->oLastEntry = 0;
		pCurrentHdr->cCurrentEntries = 0;
		pCurrentHdr->cbCurrentTable = 0;
		// points to first free row ready for data
		pCurrentHdr->oFreeEntry = (U32)(&(pCurrentBlk->data)) - (U32)prgDataBlks;
		pCurrentHdr->version = 1;
			
		#ifdef PTSDEBUG
			dbgBufEntry newRecord = { "DfT", 0, pCurrentHdr->rowId};
			AddDebugEntry (&newRecord);
		#endif					
		// If this is not the table of Tables definition...
		// Add an entry to the tableOfTables
		if ((persistFlags & TableOfTables_PT) == 0)
		{  
			memset (&rowTableOfTables, 0, sizeof(rowDef));
			strcpy((char *)(&rowTableOfTables.tableName), ptName);
			rowTableOfTables.tableId = pCurrentHdr->rowId;

			erc = InsertRow (CT_PTS_TABLE_OF_TABLES, (uchar *)&rowTableOfTables,
							 sizeof (rowTableOfTables),
							 NULL,  &cRowsInserted);
		
			// return the table id
			*pTableIdRet = pCurrentHdr->rowId;

			// search the listen queue for table defines;  if any are found, pointers
			// to messages are put in rgpListenReplies;  the ddm then replies to messages

 			erc = SearchListenQueue	(&(pHeaders->rgHdr[iTOTHeader]),
									(uchar *)ptName, (char*)NULL, 
									(U32)NULL,
									OpDefineTable);
		}
	return (ercOK);
} //DefineTable



/*************************************************************************/
//
//	DeleteTable
//
//  Remove the table from the list
//	Return table space to heap  ??? tbd
//
/*************************************************************************/

int DeleteTable (char* ptName, U32 tableId)

 {
	pTableHeader	pPrevHdr = pListHeaders;
	pTableHeader	pCurrentHdr = pListHeaders;		
	BOOL tablefound = false;
	int 			erc;
	U32				cRowsDeleted;

	
	//  the table can either be defined through its name or its tableID 
	//	from the rowID.  Check for the name, then check for tableID.
	while ((pCurrentHdr != NULL) && (!tablefound))
	{	
		if (ptName != NULL)
		{
			if (strcmp(pCurrentHdr->name, ptName) == 0)
			{	tablefound = true; 	}
		}
 		else
		{
			if (tableId == pCurrentHdr->rowId.Table)
 			{	tablefound = true;	}
		}
		if (!tablefound)
		{	// not found:  get next header
			if (pCurrentHdr->oNext != NULL)				
			{
				pPrevHdr = pCurrentHdr;
				pCurrentHdr = (pTableHeader)((U32)pHeaders +pCurrentHdr->oNext);
			}
			else
				// no more headers; return 0 pointer								
				pCurrentHdr = NULL;	  
		}
	}  // until table header found or end of headers

	if (tablefound)
	{
		// Delete row from the tableOfTables
		erc = DeleteRow (CT_PTS_TABLE_OF_TABLES, CT_PTS_TOT_TABLE_NAME, 
						 (uchar *)pCurrentHdr->name, CBMAXTABLESPEC, (U32)1, &cRowsDeleted);

		// Check to see if any listens on this table;  if any are found, pointers
		// to messages are put in rgpListenReplies;  the ddm then replies to messages
 		erc = SearchListenQueue	(pCurrentHdr,
								(uchar*) NULL, (char*)NULL, 
								(U32)NULL,
								OpDeleteTable);

		if (pCurrentHdr->oNext == 0)			// is this last table?
		{
			pListHeaders = NULL;
			pHeaders->oListHdr = 0;
		}
		else
		{
			if (pCurrentHdr != pListHeaders)			
			// if not first in list remove link to this header
				pPrevHdr->oNext = pCurrentHdr->oNext;	

			else
			// fixup pListHeaders
				pListHeaders = (pTableHeader)((U32)pHeaders + pCurrentHdr->oNext);
		}
		// add table header to free headers list
 	    pCurrentHdr->oNext = pHeaders->oFreeHdr;	
	    pFreeHeaders = pCurrentHdr;
		pHeaders->oFreeHdr = (U32)pFreeHeaders - (U32)pHeaders;	

		// zero out fields
		strcpy(pCurrentHdr->name, "");

		for (U32 j = 0; j < pCurrentHdr->cFields; j++)
		{	strcpy((char*)pCurrentHdr->rgEntryDef[j].name,"");
			pCurrentHdr->rgEntryDef[j].cbField = 0;		
		}
		#ifdef PTSDEBUG
			dbgBufEntry newRecord = { "DlT", 0, pCurrentHdr->rowId};
			AddDebugEntry (&newRecord);
		#endif
		pCurrentHdr->cFields = 0;
		pCurrentHdr->oTableData = 0;
		pCurrentHdr->rowId.Table = 0;
		pCurrentHdr->cCurrentEntries = 0;
		
		return (ercOK);
 	}
	//	else table not found
	return (ercTableNotfound);

 }	//DeleteTable


/*************************************************************************/
//  
//   GetTableDef - return information about the table:
//
//		definitions of the fields, current number of rows, number of 
//		fields per row, size in bytes of row		
//
//		If the size of either of the arrays is 0 or the pointer are null,
//		that information is	not returned;  (assumes that user is not interested)
//
/*************************************************************************/


 int GetTableDef (char	*ptName,					// table name 
 				  rowID	*pTableID,					// tableID
 				  Message	*pMsg,
 				  U32 *pcbrgFieldsRet,				// location of returned size of field defs
 				  tableDef *pTableDefRet,			// ret structure of 3 integers where # of
 				  U32 cbTableDef)					//  current rows, fields per row, size of row

 {
	U32 i, status;
  	pTableHeader pCurrentHdr;
	U32 cbrgFields;									
	fieldDef *prgFieldsRet;

 	if (GetTableHeader(ptName, pTableID, &pCurrentHdr) == ercTableExists)
	{
		cbrgFields =  pCurrentHdr->cbrgEntryDef;			// size of the field def array
		// Get a dynamically allocated buffer for the field definition array.
		pMsg->GetSgl ( GET_TABLE_DEF_REPLY_FIELDDEFS_SGI,	// index of the SGList of Field Defs 
						(void **)&prgFieldsRet, &cbrgFields);

		if ((prgFieldsRet != NULL) && (cbrgFields >= pCurrentHdr->cbrgEntryDef))
		{ // copy the fieldDefs into the buffer
			for (i=0; i < pCurrentHdr->cFields; i++)
				  prgFieldsRet[i] = pCurrentHdr->rgEntryDef[i];
		}
		// if the return parameter is large enough for the data, copy it in
		if ((cbTableDef >= sizeof (tableDef)) && pTableDefRet)
		{	 
			// cbRow includes the rowID field
			(*pTableDefRet).cbRow = pCurrentHdr->cbEntryData;
			(*pTableDefRet).cFields = pCurrentHdr->cFields;
			(*pTableDefRet).cRows = pCurrentHdr->cCurrentEntries;
		}
		status = ercOK;	
		#ifdef PTSDEBUG
			dbgBufEntry newRecord = { "GtD", 0, pCurrentHdr->rowId};
			AddDebugEntry (&newRecord);
		#endif
	
	}	
	else
		return (ercTableNotfound);

	// if the pcbrgFields is defined, return size of of fields

	if (pcbrgFieldsRet != NULL)
		*pcbrgFieldsRet = cbrgFields;

	return (status);

 } // GetTableDef


/*************************************************************************/
//
//	AddRowToTable - Add a new row entry to the table by updating links in 
//		current entry and update links in tableheader
//
//	pCurrentEntry - pointer to the next free entry for the table
//		calling routine checks for existence
//
//************************************************************************
 
int AddRowToTable(tableHeader *pCurrentHdr, entryDef *pCurrentEntry)

{
	entryDef  *pLastEntry;
	entryDef  *pFreeEntry;

  // update table header to reflect new entry

	if (pCurrentHdr->oFirstEntry == NULL)			
	{ 	// Is this the first entry to table?
		pCurrentHdr->oFirstEntry = pCurrentHdr->oFreeEntry;
		pCurrentEntry->oPrevEntry = NULL;
	}
	else
	{	// if an entry exists, then there is an offset to first and last
		// get the current last entry of table and put new entry on end 
		pLastEntry = (entryDef*)((U32)prgDataBlks + (pCurrentHdr->oLastEntry));
		pLastEntry->oNextEntry = pCurrentHdr->oFreeEntry;
		pCurrentEntry->oPrevEntry = pCurrentHdr->oLastEntry;	
	}

	// update the new head of the free list
	if (pCurrentEntry->oNextEntry != NULL)		// if not end of free list entries
	{
		pFreeEntry = (entryDef*)((U32)prgDataBlks + pCurrentEntry->oNextEntry);
		pFreeEntry->oPrevEntry = NULL;
	}

	// update table header entries
	// fixup pointers in header to the table entries  
		pCurrentHdr->oLastEntry = pCurrentHdr->oFreeEntry;
		pCurrentHdr->oFreeEntry = pCurrentEntry->oNextEntry;

 	// since last on list, update next pointer
		pCurrentEntry->oNextEntry = NULL;

	return (ercOK);
}


/*************************************************************************/
//
//	RemoveRowFromTable - remove the row entry from the table and update 
//	links in other entries and update links in tableheader
//	if fDecRid is true, then decrement the rid;  this is because an error 
//	occured and the row was never entered
//
//************************************************************************
 
void RemoveRowFromTable(tableHeader *pCurrentHdr, entryDef *pCurrentEntry,
						BOOL fDecRid)


{
	entryDef  *pPrevEntry;
	entryDef  *pNextEntry;
	entryDef  *pFreeList;


	// if a previous entry in list, fixup its next pointer to current's next pointer
	if (pCurrentEntry->oPrevEntry != NULL)
	{	//	fixup its next pointer to current's next pointer
		pPrevEntry= (entryDef*)((U32)prgDataBlks + pCurrentEntry->oPrevEntry);
		pPrevEntry->oNextEntry = pCurrentEntry->oNextEntry;
	}
	else
		// if this is the first entry in the table, fixup pointer in header
		pCurrentHdr->oFirstEntry = pCurrentEntry->oNextEntry;
	// if a next entry in list....
	if (pCurrentEntry->oNextEntry != NULL)
	{	// fixup the next entry's pointers
		pNextEntry = (entryDef*)((U32)prgDataBlks + pCurrentEntry->oNextEntry);
		pNextEntry->oPrevEntry = (U32)pCurrentEntry->oPrevEntry;
	}
	else
	// if this is the last entry, need to modify last entry offset in header.
		pCurrentHdr->oLastEntry = pCurrentEntry->oPrevEntry;

	// add entry to free list
	if (pCurrentHdr->oFreeEntry != NULL)
	{ // get pointer to first entry on free list and link this row to front of list
		pFreeList = (entryDef*)((U32)prgDataBlks + pCurrentHdr->oFreeEntry); 
		pFreeList->oPrevEntry = (U32)pCurrentEntry - (U32)prgDataBlks;	
		// fixup deleted entry's pointers so point to next free list entry
		pCurrentEntry->oNextEntry = pCurrentHdr->oFreeEntry;			
		pCurrentEntry->oPrevEntry = NULL;
		pCurrentHdr->oFreeEntry = pFreeList->oPrevEntry;
	}
	else
	{ //  if free list is empty,  add this row
	 	pCurrentEntry->oNextEntry = NULL;
		pCurrentEntry->oPrevEntry = NULL;
		pCurrentHdr->oFreeEntry = (U32)pCurrentEntry - (U32)prgDataBlks;
	}
	// if an error was encountered before this row was input, need to decrement
	// the rowID count
	if (fDecRid)
		pCurrentHdr->rowId.LoPart--;
}

/*************************************************************************/
//
//	InsertRow - Write one or more row entries into the table;
//		prgRows - array of rows which are the same format as defined;
//			 the rows sent by the user will include space for rowIDs,
//			 so the user can define one structure to use throughout
//		cbrgRows- count of bytes in prgRows;  this value divided by the 
//			size of the row is the number of rows to insert
//		prgIdsRet, cbrgIdsRet - user buffer where Ids of inserted rows are
//			returned;  if either null, no ids returned
//		pcIdsRet - user variable where count of Idsif null, 
//
//	 This routine will return the rowIDs into the buffer provided by the
//	 caller.
//
/*************************************************************************/


 
int InsertRow(char *ptName,					// table name
			 uchar *prgRows, U32 cbrgRows,	// array of 1 or more rows which match row definition
			 Message *pMsg,					// message, 
			 U32   *pcRowsInRet)			// number of rows inserted

{
   	U32			 status, i,  erc;
	BOOL lastHdr = false;
  	pTableHeader pCurrentHdr;
	entryDef 	*pCurrentEntry;
	U32			cRowsInserted = 0;			// actual count of rows inserted;  returned in pcRowsInRet 
	U32			cbData, cEntries, cbBuf;	 
	uchar 		*pSource, *pRowBuf;
	dataBlkHdr	*pCurrentBlk;	
	rowID		*prgIDsRet;



	cInsertRow++;
	if (cInsertRow > 100)
		int foobar = 9;	
	
	// If the table exists and there are still free entries, then add row to table

	if (GetTableHeader(ptName, (rowID*)prgRows, &pCurrentHdr) != ercTableExists)
		return (ercTableNotfound);
		
	if (prgRows == NULL)
		return (ercBadParameter);	

	if ((pCurrentHdr->persistFlags  & VarLength_PT) != 0)
		return (ercInvalidCmdForVLF);

	cEntries = cbrgRows/(pCurrentHdr->cbEntryData);

	// A buffer is dynamically allocated that is used to hold the rowIDs of the inserted rows
	// if this routine is called locally, then pMsg will be NULL, and rowIDs are not available
	if (pMsg != NULL)
	{
		cbBuf = cEntries * sizeof(rowID);
		if (cbBuf != 0)
			pMsg->GetSgl (INSERT_ROWS_REPLY_ROWIDS_SGI,		// Index of the SG List to row or rowID 
						  (void**)(&prgIDsRet),	&cbBuf );	// Pointer to reply SGL buffer
		else
			prgIDsRet = NULL;
	}
	else
		prgIDsRet = NULL;
	
	pSource = prgRows;  				// define pointer to move through buffer
	status = ercOK;

	// process each row of the array separately;  the row includes rowID field
	// which is ignored , as it is set by the service

	for (i = 0; i < cEntries; i++)
	{
	  	// check for a duplicate entry
		erc = IsThisDuplicateEntry (pCurrentHdr, pSource);
		// if a duplicate value is found in a non duplicate field, return, get out
		if (erc == ercDuplicateFieldFound)
		{	
		  status = ercDuplicateFieldFound;
		  break;
		}
		if (erc == ercOkDuplicateRowFound)
			status = ercOK;  		// need to return something???

		// if a duplicate is not found, go ahead and insert row...
		if (erc == ercOkDuplicateNotFound)
		{
			if (pCurrentHdr->oFreeEntry == NULL)
			{ // if no more free entries, allocate a new data block
				if ((pCurrentBlk = ExtendTable (pCurrentHdr)) == NULL)
					return (ercNoMoreDataHeap);
			}
			// get the next free entry
			pCurrentEntry = (entryDef*)((U32)prgDataBlks + pCurrentHdr->oFreeEntry);
			// currently, no errors returned
			erc = AddRowToTable (pCurrentHdr, pCurrentEntry);
			// increment unique row id
			pCurrentHdr->rowId.LoPart++;			
					
 			// copy row into entry in table;  include new, unique rowID
			pCurrentEntry->rowId = pCurrentHdr->rowId;		   
			pCurrentEntry->cbRow = pCurrentHdr->cbEntryData;	
			//pDest = (uchar*)(pCurrentEntry->entrydata);
			cbData = pCurrentHdr->cbEntryData - sizeof(pCurrentEntry->rowId);
 			memcpy ((uchar*)(pCurrentEntry->entrydata), (pSource + sizeof(rowID)), cbData);
  			pCurrentHdr->cbCurrentTable += pCurrentHdr->cbEntryData;
 	    	pCurrentHdr->cCurrentEntries++;
			cRowsInserted++;						// count of rows inserted				

			// save new row address so that if listener, can send row back
			pRowBuf = (uchar*)(&(pCurrentEntry->rowId));
			// store the rowID in the dynamic buffer
			if (prgIDsRet != NULL)
				*prgIDsRet++ = pCurrentEntry->rowId;		

			#ifdef PTSDEBUG
				dbgBufEntry newRecord = { "IR ", 0, pCurrentEntry->rowId};
				AddDebugEntry (&newRecord);
			#endif
			
			pCurrentHdr->version++;

			// search the listen queue for table defines;  if any are found, pointers
			// to messages are put in rgpListenReplies;  the ddm then replies to messages
			// erc is not used here

 			erc = SearchListenQueue	(pCurrentHdr,  pRowBuf,	(char*)NULL,
									 (U32)NULL, OpInsertRow);
		} // endif duplicate entry...  get next
	
		pSource += pCurrentHdr->cbEntryData;			// advance src ptr to next

	}  // end of for loop, get next row
	
	if (pcRowsInRet != NULL)
		*pcRowsInRet= cRowsInserted;					// if parameter defined, return rows inserted		

	return (status);

}  // InsertRow	

/*************************************************************************/
//
//	AddEntryToHeap - Get an entry from the specified heap for a 
//		variable length field. 
//
//************************************************************************
 
int AddEntryToAHeap (entryDef *pCurrentEntry, varLenFieldDef *pVLFinRow,
					 varLenField *pVarLenFields, uchar *pHeapData)

{
	heapEntryDef  *pFreeHeapEntry, *pCurrentHeapEntry;
	dataBlkHdr	  *pCurrentHeapBlk;
	U32		iHeap = 0;
	heapHeader	*pHeapHdr;
	BOOL	foundFit = false;


	while ((!foundFit) && (iHeap < iMaxHeaps))
	{	// search the heaps for one that has big enough records
		if ((pVarLenFields->cbVLField) < rgiHeapToSize[iHeap])
		{	// entry will fit in record
			pHeapHdr = (heapHeader *)&(pHeaders->rgHeapHdr[iHeap]);
			foundFit = true;
		}
		else	// try next heap
			iHeap++;
	}
	// if a fit is not found, ...  need to have a big heap
	if (!foundFit)
		return (ercFieldSizeTooLarge);
	// Get heap entry for var length part of row
	if (pHeapHdr->oFreeEntry == NULL)
	{  // if no more free entries, allocate a new data block
		if ((pCurrentHeapBlk = ExtendHeap (pHeapHdr)) == NULL)
			return (ercNoMoreDataHeap);
	}
	// get the next free entry
	pCurrentHeapEntry = (heapEntryDef*)((U32)prgDataBlks + pHeapHdr->oFreeEntry);

	// update heap header to reflect new entry
	if (pCurrentHeapEntry->oNextEntry != NULL)		
	{	// if not end of free list entries, update pointers
		pFreeHeapEntry = (heapEntryDef*)((U32)prgDataBlks + pCurrentHeapEntry->oNextEntry);
		pFreeHeapEntry->oPrevEntry = NULL;
	}
	// update heap header entries
 	pHeapHdr->cCurrentEntries++;
	// fixup pointers in header to the table entries  
	pHeapHdr->oFreeEntry = pCurrentHeapEntry->oNextEntry;
	// zero pointer since off of free list
	pCurrentHeapEntry->oNextEntry = NULL;
	// write rowId to heap entry also...  just in case
	pCurrentHeapEntry->rowId = pCurrentEntry->rowId;	
	// write rowId to heap entry also...  just in case
	pCurrentHeapEntry->vlfOfUser = *pVarLenFields;
	// copy varible length field to heap after user varlenfield struct
	memcpy ((((uchar*)&(pCurrentHeapEntry->vlfOfUser) + sizeof(varLenField))), pHeapData,
			  pVarLenFields->cbVLField);
	// update structure in fixed part of row to point to heap entry
	pVLFinRow->iHeapHdr = iHeap;
	pVLFinRow->oHeapEntry = (U32)pCurrentHeapEntry - (U32)prgDataBlks;
	pVLFinRow->cbField = pVarLenFields->cbVLField;
	return (ercOK);
	
} // AddEntryFromHeap


/*************************************************************************/
//
//	DeleteEntryFromHeap - Given a row to delete, and vlf struct, return 
//		variable length fields to their heaps 
//
//************************************************************************
 
int DeleteEntryFromHeap(entryDef *pCurrentEntry, varLenFieldDef *pVLFinRow)
{
	heapEntryDef  *pCurrentHeapEntry, *pFreeEntry;
	heapHeader	*pHeapHeader;


	// start with a check on the struct info
	if (pVLFinRow->iHeapHdr < iMaxHeaps)
	{	// get the variable length field... do a sanity check...
		pCurrentHeapEntry = (heapEntryDef*)((U32)prgDataBlks + pVLFinRow->oHeapEntry);
		// another sanity check... make sure rowIDs match...
		if ((memcmp(&(pCurrentHeapEntry->rowId), &(pCurrentEntry->rowId), sizeof(rowID))) == 0)
		{	// match, so return entry to heap
			// get a pointer to the heap header where this entry resides
			pHeapHeader = (heapHeader *)&(pHeaders->rgHeapHdr[pVLFinRow->iHeapHdr]);		
			// add entry to free list
			if (pHeapHeader->oFreeEntry != NULL)
			{	// get pointer to first entry on free list and link this row to front of list
				pFreeEntry = (heapEntryDef*)((U32)prgDataBlks + pHeapHeader->oFreeEntry); 
				pFreeEntry->oPrevEntry = (U32)pCurrentHeapEntry - (U32)prgDataBlks;	

				// fixup deleted entry's pointers so point to next free list entry
				pCurrentHeapEntry->oNextEntry = pHeapHeader->oFreeEntry;			
				pCurrentHeapEntry->oPrevEntry = NULL;
				pHeapHeader->oFreeEntry = pFreeEntry->oPrevEntry;
			}
			else
			{  // if free list is empty,  add this row
			 	pCurrentHeapEntry->oNextEntry = NULL;
				pCurrentHeapEntry->oPrevEntry = NULL;
				pHeapHeader->oFreeEntry = (U32)pCurrentHeapEntry - (U32)prgDataBlks;
			}
			// put delete flag in data area for debug; clear rest of row
			memset (&(pCurrentHeapEntry->rowId), 0, (pVLFinRow->cbField + sizeof(rowID) + sizeof(varLenField)));
			pCurrentHeapEntry->rowId.Table = 0xde1;
			// increment deleted row counter

			pHeapHeader->cCurrentEntries--;
			pVLFinRow->iHeapHdr = 0;
			pVLFinRow->oHeapEntry = 0;
			pVLFinRow->cbField = 0;
		}	// if match
		else
			return (ercInconsistentRowIds);	
	} // if < maxHeaps
	else
		return 	(ercBadVlfStruct);		

	return (ercOK);
	
} // DeleteEntryFromHeap


//**************************************************************************************
//	InsertVarLenRow - 
//
//	 Insert the fixed portion into table, then get heap entries and insert others.
//	 If an error occurs and the fixed row has been written, it is backed out.
//**************************************************************************************


int InsertVarLenRow (char *ptName,					// table name
			 uchar	*pRowData, U32 cbRowData,		// pbcb of row; 
			 rowID	*pRowIdRet)						// location where row IDs are returned

{
   	U32			 i, status, cVarLenFields, oVLField;
  	tableHeader *pCurrentHdr;
	entryDef 	*pCurrentEntry;
	U32			cRowsInserted = 0;			// actual count of rows inserted;  returned in pcRowsInRet 
	U32			cbData, cbRowTotal, cbWritten, cbVarLenDefs; 
	uchar 		*pDest,	*pRowBuf;
	dataBlkHdr	*pCurrentBlk;
	BOOL		foundFit = false;
	varLenFieldDef *pVLFinRow;
	varLenField *pVarLenFields;
	U32			erc = ercOK;
	
	// Check if the table exists...
	cInsertVLRow++;

	if (GetTableHeader(ptName, (rowID*)pRowData, &pCurrentHdr) != ercTableExists)
		return (ercTableNotfound);
		
	if ((pRowData == NULL) || (cbRowData < pCurrentHdr->cbFixedLenData))
		return (ercBadParameter);

	if ((status = IsThisDuplicateEntry (pCurrentHdr, pRowData)) == ercDuplicateFieldFound)
	{	// if a duplicate value is found in a non duplicate field, return, get out
		  return (ercDuplicateFieldFound);
	}
	if (status == ercOkDuplicateRowFound)
		erc = ercOK;  		// just return....

	// if a duplicate is not found, go ahead and insert row...
	if (status == ercOkDuplicateNotFound)
	{	// Get entry for fixed length part of row
		if (pCurrentHdr->oFreeEntry == NULL)
		{	// if no more free entries, allocate a new data block
			if ((pCurrentBlk = ExtendTable (pCurrentHdr)) == NULL)
				return (ercNoMoreDataHeap);
		}
		// get the next free entry
		pCurrentEntry = (entryDef*)((U32)prgDataBlks + pCurrentHdr->oFreeEntry);
		// go ahead and add the current fixed entry to the table
		erc = AddRowToTable (pCurrentHdr, pCurrentEntry);
		// increment unique row id
		pCurrentHdr->rowId.LoPart++;			

		// copy row into entry in table:  include new, unique rowID
		pCurrentEntry->rowId = pCurrentHdr->rowId;
		pDest = (uchar*)(pCurrentEntry->entrydata);
		cbData = pCurrentHdr->cbFixedLenData - sizeof(rowID);
		cbRowTotal = pCurrentHdr->cbFixedLenData;
		memcpy (pDest, pRowData + sizeof(rowID), cbData);

		// next, set up for copy of variable length parts
		cVarLenFields = pCurrentHdr->cFields - pCurrentHdr->cFixedLenFields;
		cbVarLenDefs = pCurrentHdr->cbEntryData - pCurrentHdr->cbFixedLenData;

		// calculate address of var len field structs in client's row
		pVarLenFields = (varLenField *)((U32)pRowData + cbRowTotal);
		// calculate address of PTS var len definitions in table entry
		pVLFinRow = (varLenFieldDef *)((U32)pDest + cbData);   

		// fixed length row is in the table... 
		for (i = 0;  i < cVarLenFields; i++)
		{ // now, find heaps for variable length parts and update row structs with info
			cbData = pVarLenFields->cbVLField; 
			oVLField = pVarLenFields->oVLField;
			// keep running sum of row length;  this includes the user's vlf struct
			cbRowTotal += (cbData + sizeof(varLenField));
			// limit checks:  if the offset of the VLF > size of input row
			//		or if the calculated row size is greater than the input row
			if ((oVLField > cbRowData) || (cbRowTotal > cbRowData))
			{	// if either the offset or total size is off, return error...
				erc = ercInVarLenFieldDef;
			}
			else
			{	// otherwise, enough room... add entry
				if (cbData == 0)
				{ // if variable length field has no length, just update the table info
					pVLFinRow->iHeapHdr = 0;
					pVLFinRow->oHeapEntry = 0;
					pVLFinRow->cbField = cbData;
				}
				else
				// otherwise add the entry to the heap
					erc = AddEntryToAHeap (pCurrentEntry, pVLFinRow, pVarLenFields,
										(uchar *)((U32)pRowData + oVLField));
			}
			if (erc != ercOK)
			{	// if an error, back out fixed row entry and return
				RemoveRowFromTable(pCurrentHdr, pCurrentEntry, true);
				return (erc);
			}
			// advance pointer into client's buffer
			pVarLenFields++;		// point to next vlf struct in client row			
			pVLFinRow++;			// pointer to next vlf struct in table row
		} // end for
		
		pCurrentEntry->cbRow = cbRowTotal;					// store the row size in row
 		pCurrentHdr->cbCurrentTable += cbRowTotal;			// add row size to total
		pCurrentHdr->cCurrentEntries++;

		// get a buffer for new row so that if listener, can send row back
		if (cbRowTotal > 8000)
			cbRowTotal = pCurrentHdr->cbFixedLenData;
		pRowBuf = new  uchar[cbRowTotal];

		erc = GetVarLenRow (pCurrentHdr, pCurrentEntry, pRowBuf, cbRowTotal, false, &cbWritten);
		//if (erc != ercOK)
		//	Assert...;
		
		*pRowIdRet = pCurrentEntry->rowId;
		pCurrentHdr->version++;

		#ifdef PTSDEBUG
			dbgBufEntry newRecord = { "IvR", 0, pCurrentEntry->rowId};
			AddDebugEntry (&newRecord);
		#endif
	
		// search the listen queue for table defines;  if any are found, pointers
		// to messages are put in rgpListenReplies;  the ddm then replies to messages
		// erc is not used here
 		erc = SearchListenQueue	(pCurrentHdr,  pRowBuf,	(char*)NULL,
								 (U32)NULL, OpInsertRow);
		delete [] pRowBuf;

	}  // end if duplicate not found
	
	return (ercOK);

}  // InsertVarLenRow	


/**************************************************************************/
//
//	GetVarLenRow - get and write the row into client's buffer. 
//		Gets either a fixed length row, the fixed length portion of a vl row,
//		or the whole row.
//		pCurrentEntry - pointer to row entry in the table,	
//		pRowRet, cbRow - pointer to buffer where the row + user VLF structs +
//						variable length field data is returned
//		fIncludeVLFields -  include varlen field data in return buffer?
//		pcbRowRet	- count of bytes returned in buffer

/*************************************************************************/

int GetVarLenRow 	(tableHeader *pCurrentHdr,
		     entryDef	*pCurrentEntry,	
		     uchar 		*pRowRet, U32 cbRow,
			 BOOL		fIncludeVLFields,			// include varlen fields?
		     U32		*pcbRowRet)
			   	
{
	varLenFieldDef *pVarLenDef;						// PTS VLF struct in actual row
 	heapEntryDef	*pCurrentHeapEntry;				// heap entry associated with VLF
 	varLenField		*pUserVarLenStructs;			// client's vlf struct in row output
 	uchar 			*pVarLenFieldData;				// client's vlf data in row output
 	uchar 			*pIntoHeapEntry;				// ptr into heap entry
 	U32	cVarLenFields;
 	U32	cbRowRet = 0;
 	
	// check for big enough buffer for fixed length data
 	if (cbRow < pCurrentHdr->cbFixedLenData)
 		return ercBufTooSmall;
 	 		
	// copy the fixed length part into the buffer
	memcpy (pRowRet, (uchar*)&(pCurrentEntry->rowId), pCurrentHdr->cbFixedLenData);
	// if the row has variable length fields... get those 
	
	cbRowRet = pCurrentHdr->cbFixedLenData;
	pRowRet += cbRowRet;					// increment client's buffer pointer
	// set up ptr to var length structure in row in table, if exist...
	if (pCurrentHdr->cbEntryData > pCurrentHdr->cbFixedLenData)
		pVarLenDef = (varLenFieldDef*)((U32)&(pCurrentEntry->rowId) + pCurrentHdr->cbFixedLenData);
	
	cVarLenFields = (pCurrentHdr->cFields) - (pCurrentHdr->cFixedLenFields);
	// check for enough space for VLF fields or structs
	if (cVarLenFields > 0 )
	{
		if (fIncludeVLFields == true)
		{
			if (cbRow < pCurrentEntry->cbRow)
				return ercBufTooSmall;
		}
		else
			if (cbRow < (cbRowRet + (cVarLenFields * sizeof(varLenField))))
				return ercBufTooSmall;		
		// set up pointers to move data into the return row:  
		pUserVarLenStructs = (varLenField *)pRowRet;
		pVarLenFieldData = pRowRet + (cVarLenFields * sizeof(varLenField));
	}
	while (cVarLenFields > 0) 
	{  // get the variable length field...
		if (pVarLenDef->cbField > 0)
		{ // the variable length field can be of zero length... if so, skip to next
			if (pVarLenDef->iHeapHdr < iMaxHeaps)
			{	 // do a sanity check...
				pCurrentHeapEntry = (heapEntryDef*)((U32)prgDataBlks + pVarLenDef->oHeapEntry);
				// another sanity check... make sure rowIDs in tableEntry and heapEntry match...
				if ((memcmp(&(pCurrentHeapEntry->rowId), &(pCurrentEntry->rowId), sizeof(rowID))) == 0)
				{	// compare, so start copying after the rowID
					// if client wants all fields, flag is set, get count 
					pIntoHeapEntry = (uchar*)&(pCurrentHeapEntry->rowId) + sizeof(rowID);
					// copy user's VLF first...
					memcpy (pUserVarLenStructs, pIntoHeapEntry, sizeof(varLenField));
					// bump pointer to data
					pIntoHeapEntry += sizeof(varLenField);
					cbRowRet += sizeof(varLenField);
					// if client has requested VLF data, get it too...
					if (fIncludeVLFields == true)
					{
						memcpy (pVarLenFieldData, pIntoHeapEntry, pVarLenDef->cbField);
						// bump pointers into user buffer to next data  
						pVarLenFieldData += pVarLenDef->cbField;
						cbRowRet += pVarLenDef->cbField;
					}	
				}
				else   // rowIds do not match 
					return (ercInconsistentRowIds);	
			}  // bad heap number
			else
				return 	(ercBadVlfStruct);
		}
		else   // else if cbVLField = 0: clear out the user varlen struct 
		{
			memset (pUserVarLenStructs, 0, sizeof(varLenField));
			cbRowRet += sizeof(varLenField);
		}
		pUserVarLenStructs++;	 // increment the ptr to the VLFs in Returned user row
		pVarLenDef++;			// increment the ptr to the VLFs in PTS row
		cVarLenFields--;
			
	} // end while

	if (pcbRowRet != NULL)
		*pcbRowRet = cbRowRet;
	return (ercOK);
}

/*************************************************************************/
//
//	EnumerateTable - Read one or more row entries from the table starting
//	 at the offset into the user's buffer.  If offset is 0, starts at the 
//	 beginning of the linked list of row entries.
//
//	 Writes row entries into the buffer until end of buffer or end of table.
//	 pBufRet - if null, an exception occurs;
//	 pcbBufRet - returns # of bytes written to buffer;  if null, ignored.
//
//	 offset - is the n+1 row in the linked list of the table (0 based)
//
//	 errors returned:  ercTableNotFound	
//
/*************************************************************************/


int EnumerateTable(char *ptName,				// table name
 				   U32 offset,					// where to start enumerating, 0 is first row
				   uchar *pUserBufRet, U32 cbMax,// location where table rows returned for internal routines
				   Message	*pMsg,
				   U32 *pcbBufRet)				// count of bytes returned in buffer
				   	
{
    U32			 i, erc;
  	pTableHeader pCurrentHdr;
	entryDef	*pCurrentEntry;
	U32			 cbBufRet, cRows, cbAlloc, cbRowRet, cVarLenFields;
	uchar		*pBufRet;
 
	if (GetTableHeader(ptName, NULL, &pCurrentHdr) != ercTableExists)
		return (ercTableNotfound);

	// get the first entry on list
	pCurrentEntry = (entryDef*)((U32)prgDataBlks + pCurrentHdr->oFirstEntry);
	// calculate the desired size of the buffer....and the desired number of rows
	cRows = (pCurrentHdr->cCurrentEntries) - offset;
	cVarLenFields = pCurrentHdr->cFields - pCurrentHdr->cFixedLenFields;		
	// if this table has var len rows, return only the fixed length portion
	cbAlloc = cRows * ((pCurrentHdr->cbFixedLenData) + 
				(sizeof(varLenField) * cVarLenFields));		

	// if an offset is greater than 0, use as a zero based index
	for (i = 1; i <= offset; i++)	
		pCurrentEntry = (entryDef*)((U32)prgDataBlks + pCurrentEntry->oNextEntry);
 
 	// the return buffer for the table is allocated dynamically by OS unless 
	//  enumerate table is called internally:  then buffer is defined by pUserBufRet
 	if (pMsg == NULL) 
 	{	//  called from local routine?
		if (pUserBufRet != NULL) 
 		{	//  use the client buffer, and calculate rows based on buffer size
 			pBufRet = pUserBufRet;
			cbAlloc = cbMax;
		}	
		else
			return(ercBadParameter);
 	}
 	else	
	{	// if no entries in the table, do not get a buffer
		if (cbAlloc != 0)
		{ // get a dynamically allocated buffer for the rows
			pMsg->GetSgl( ENUMERATE_TABLE_REPLY_SGI,		// index of the SG List to row buffer 
					  (void**)&pBufRet, &cbAlloc);
			if ((pBufRet == NULL) || (cbAlloc == NULL))  
				return (ercNoDynamicReplyBuffer);
		}		
	}		
	cbBufRet = 0;						// get actual count for return
	// while there are still rows to write, and enough room...
	while ((cRows > 0) && (cbAlloc >= 
				(pCurrentHdr->cbFixedLenData + (cVarLenFields * sizeof(varLenField)))))
	{	// copy the row into the buffer
		if ((erc = GetVarLenRow (pCurrentHdr, pCurrentEntry, pBufRet, cbAlloc, false, &cbRowRet)) == ercOK)
		{//memcpy (pBufRet, (uchar*)&(pCurrentEntry->rowId), pCurrentHdr->cbEntryData);
		// update the pointers and counters
			pCurrentEntry = (entryDef*)((U32)prgDataBlks + pCurrentEntry->oNextEntry);
			pBufRet += cbRowRet;
			cbBufRet += cbRowRet;		// keep a count of bytes written for return
			cRows--;
			cbAlloc -= cbRowRet;
		}
		else
			return (erc);
	}
	if (pcbBufRet != NULL)
		*pcbBufRet = cbBufRet;

	#ifdef PTSDEBUG
		dbgBufEntry newRecord = { "EnT", 0, pCurrentHdr->rowId};
		AddDebugEntry (&newRecord);
	#endif
	
	return (ercOK);

}	// EnumerateTable


/*************************************************************************/
//
//	ReadRow - searches and returns one or more rows that match one specified
//	 key value
//
//	The number of rows returned in the user's buffer depends on how many
//		will fit; rows include rowID
//	If the user buffer is not large enough for all rows, it will continue to 
//	read and count.  The count of rows will be returned at pcRowsRead, if not null.  
//		???????need to look into return errors...
/*************************************************************************/


int ReadRow(char *ptName,
 			char  *pKeyName,						// key field name to find
  			uchar *pKeyValue,						// key field value to find 
			U32	  cbKeyValue,
   			Message *pMsg,
			U32   *pcRowsRead)						// count of rows read and returned if space

{
	U32 *pRowsRet;
	U32 i;
	U32	cRowsCompare, cbRowRet;						// count of rows that matched key value
  	pTableHeader pCurrentHdr;
	rowID	*pTableId = NULL;
	uchar	*prgRowsRet, *pRowEntry;
	U32		cbrgRows;
	U32		erc = ercOK;
 	
	//  Get the table header;  if the table name is not defined, and the key is "rid" use the tableID 
	//	 to find the table

	if (pKeyName == NULL)
		return (ercBadParameter);
	if ((pKeyValue != NULL) && (ptName == NULL))
	{	// table ID can be taken from the keyvalue if no ptName	
		if (strcmp(pKeyName, CT_PTS_RID_FIELD_NAME) == 0)
			pTableId = (rowID *)pKeyValue;
	}	
	if (GetTableHeader(ptName, pTableId, &pCurrentHdr) != ercTableExists)
		return (ercTableNotfound);
		
	if ((pRowsRet = new U32[(pCurrentHdr->cCurrentEntries)]) == NULL)	// allocate mem for CompareField
		return (ercNoMoreHeap);					// deleted at end of routine

	if ((CompareField(pCurrentHdr, pKeyName, pKeyValue, cbKeyValue, pRowsRet,
		 (sizeof(U32) * (pCurrentHdr->cCurrentEntries)), &cRowsCompare) == ercOK) && cRowsCompare)
	{ 	// if one or more rows were found where the key compares, copy them to
		//  the dynamically allocated reply buffer
		cbrgRows = cRowsCompare * (pCurrentHdr->cbEntryData);
		pMsg->GetSgl ( READ_ROWS_REPLY_DATA_BUFFER_SGI,			//  index of the SG return list of rowIDs 
						(void **)&prgRowsRet, &cbrgRows);		//  pointer, size of return buffer of IDs

		if ((prgRowsRet != NULL) && (cbrgRows != NULL))  
		{	// while there are still rows to write, and enough room...
			i = 0;
			while ((i < cRowsCompare) && (erc == ercOK))
			{	// copy the row into the buffer
				pRowEntry = (uchar *)(pRowsRet[i++]) - sizeof(entryHdr);
				erc = GetVarLenRow (pCurrentHdr, (entryDef *)pRowEntry, prgRowsRet, cbrgRows, false, &cbRowRet);
				prgRowsRet += cbRowRet;
				cbrgRows -= cbRowRet;
			}
		
		}	// return count of rows that match, even if they do not fit in buffer
		if (pcRowsRead != NULL)
			*pcRowsRead = cRowsCompare;
		delete [] pRowsRet;
		return (ercOK);
	}
	//else if the key was not found

	#ifdef PTSDEBUG
		dbgBufEntry newRecord = { "RR", (U16)cRowsCompare, pCurrentHdr->rowId};
		AddDebugEntry (&newRecord);
	#endif
		
	if (pcRowsRead != NULL)
		*pcRowsRead = 0;
	delete [] pRowsRet;	
	return (ercKeyNotFound);
}	// end ReadRows

/*************************************************************************/
//
//	ReadVLRows - searches and returns one or more rows that match one specified
//	 key value.  This routine returns the entire row, including the variable 
//	 length fields.
//
//	The number of rows returned in the user's buffer depends on how many
//	will fit; rows include rowID
//	If the user buffer is not large enough for all rows, it will continue to 
//	read and count.  The count of rows will be returned at pcRowsRead, if not null.  
//		???????need to look into return errors...
/*************************************************************************/

int ReadVLRow(	char *ptName,
				char  *pKeyName,		// key field name to find
  				uchar *pKeyValue,		// key field value to find 
				U32	  cbKeyValue,
   				Message *pMsg,
				U32   *pcRowsRead)		// count of rows read and returned if space

{
	U32 *pRowsRet;
	U32 i, j, cbVLFields, cbFixedLenRow;
	U32	cRowsCompare, cbRowRet, cVLFields;			
  	pTableHeader pCurrentHdr;
	rowID	*pTableId = NULL;
	uchar	*prgRowsRet, *pRowEntry, *pIntoRowsRet;
	U32		cbrgRows, irgVLFret;
	U32		erc = ercOK;
 	varLenField	*prgVarLenFieldsRet, *pVarLenField;
	
	//  Get the table header;  if the table name is not defined, and the key is "rid" use the tableID 
	//	to find the table

	if (pKeyName == NULL)
		return (ercBadParameter);
	if ((pKeyValue != NULL) && (ptName == NULL))
	{	// table ID can be taken from the keyvalue if no ptName	
		if (strcmp(pKeyName, CT_PTS_RID_FIELD_NAME) == 0)
			pTableId = (rowID *)pKeyValue;
	}	
	if (GetTableHeader(ptName, pTableId, &pCurrentHdr) != ercTableExists)
		return (ercTableNotfound);
	
	if ((pRowsRet = new U32[(pCurrentHdr->cCurrentEntries)]) == NULL)	// allocate mem for CompareField
		return (ercNoMoreHeap);					// deleted at end of routine

	if ((CompareField(pCurrentHdr, pKeyName, pKeyValue, cbKeyValue, pRowsRet,
		 (sizeof(U32) * (pCurrentHdr->cCurrentEntries)), &cRowsCompare) == ercOK) && cRowsCompare)
	{	// if this table has variable length rows, get the size of the info to return
		cbrgRows = 0;		
		for (i = 0; i < cRowsCompare; i++) 
		{	// calculate the size of buffer needed for return data
			pRowEntry = (uchar *)(pRowsRet[i]) - sizeof(entryHdr);
			cbrgRows += (((entryHdr *)pRowEntry)->cbRow);
		}
		// if one or more rows were found where the key compares, copy them to
		//  the dynamically allocated reply buffer
		pMsg->GetSgl (READ_VLROWS_REPLY_ROWDATA_SGI,				//  index of the SG return list of rowIDs 
						(void **)&prgRowsRet, &cbrgRows);			//  pointer, size of return buffer of IDs
		cbVLFields = cRowsCompare * sizeof (varLenField);
		//  the dynamically allocated reply buffer for info
		pMsg->GetSgl (READ_VLROWS_REPLY_VLFS_SGI,					//  index of the SG return list of rowIDs 
						(void **)&prgVarLenFieldsRet, &cbVLFields);	//  pointer, size of return buffer of IDs

		if ((prgRowsRet != NULL) && (cbrgRows != NULL))  
		{	// while there are still rows to write, and enough room...
			i = 0;
			pIntoRowsRet = prgRowsRet;
			cbFixedLenRow = pCurrentHdr->cbFixedLenData;
			cVLFields = pCurrentHdr->cFields - pCurrentHdr->cFixedLenFields;
			irgVLFret = 0;			// index into Var len field struct array
			while ((i < cRowsCompare) && (erc == ercOK))
			{ // copy the row into the buffer
				pRowEntry = (uchar *)(pRowsRet[i]) - sizeof(entryHdr);
				erc = GetVarLenRow (pCurrentHdr, (entryDef *)pRowEntry, pIntoRowsRet,
									cbrgRows, true, &cbRowRet);

				if ((prgVarLenFieldsRet != NULL) && (cbVLFields != NULL))
				{  // copy offsets info into return array
					pVarLenField = (varLenField *)(pIntoRowsRet + cbFixedLenRow);
					for (j = 0; j < cVLFields;  j++)
					{  // get each of the variable len field structs
						prgVarLenFieldsRet[irgVLFret].cbVLField = pVarLenField->cbVLField;
						prgVarLenFieldsRet[irgVLFret++].oVLField = 
								(U32)(pIntoRowsRet  - (U32)prgRowsRet);
						pVarLenField++;
					}	
				}
				pIntoRowsRet += cbRowRet;
				cbrgRows -= cbRowRet;
				i++;
			}
		
		}	// return count of rows that match, even if they do not fit in buffer
		if (pcRowsRead != NULL)
			*pcRowsRead = cRowsCompare;
		delete [] pRowsRet;

		#ifdef PTSDEBUG
			dbgBufEntry newRecord = { "RvR", cRowsCompare, ((entryDef *)pRowEntry)->rowId};
			AddDebugEntry (&newRecord);
		#endif
	
		return (ercOK);
	}
	//else if the key was not found
	
	if (pcRowsRead != NULL)
		*pcRowsRead = 0;
	delete [] pRowsRet;	
	return (ercKeyNotFound);
}	// end ReadVLRows

/*************************************************************************/
//
//	ReadRowMultiKey - searches and returns one or more rows that matches
//	 the specified key value.
//
//	This procedure accepts an array of key values.  
//
//	The return array contains all rows in the order of the keynames.  The 
//	user must identify the matching rows by reading the keys returned.
//
/*************************************************************************/

/*
int ReadRowMultiKey(char *ptName,							// table name
					char *prgKeyNames, U32 cbrgKeyNames,	// array of key names which correspond to
					uchar *prgKeyValues,/*cbrgKeyValues	// array of key values
					uchar *prgRowsRet, U32 cbMax,			// location where rows returned
					U32 *pcbRet)							// count of bytes returned
					
{
    U32 i, erc, cbLeft, cbField, cKeyCommands;
	uchar *pDest;
	char  *pKeyName;
	uchar *pKeyValue;
  	pTableHeader pCurrentHdr;
	U32   iField = 0;
	U32   oField = 0;
    //U32   cbWrittenRet;
	U32	  cbBufRet = 0;

	
	pDest = prgRowsRet;			
	cbLeft = cbMax;


	if (GetTableHeader(ptName, NULL, &pCurrentHdr) != ercTableExists)
		return (ercTableNotfound);

	// define local pointers into user's arrays
	
	pKeyName = prgKeyNames;
 	pKeyValue = prgKeyValues;
	
	// cKeyCommands is derived by dividing the size of key array by size of keyname
	cKeyCommands = cbrgKeyNames/CBMAXFIELDSPEC;

	// prgKeyNames and prgKeyValues together define the one or more keys to be found;
	// treat each key separately

	for (i=0; i < cKeyCommands; i++)   
	{	
		if ((FindField(pCurrentHdr, pKeyName, &oField, &iField)) != ercOK)
			return ercKeyNotFound;
		cbField= pCurrentHdr->rgEntryDef[iField].cbField;

		// send the address of the return buffer, so the rows are stored directly
		/*  disable this routine... until later
		if ((erc = ReadRow(ptName, pKeyName, pKeyValue, pDest, cbLeft, &cbWrittenRet)) != 0)
			return (erc);

		else
		{	
			cbBufRet += cbWrittenRet;		// keep a running sum of the # of bytes written
			cbLeft -= cbWrittenRet;			// use cbLeft to make sure ReadRow does not overflow buffer
			pDest += cbWrittenRet;			// increment the pointer in user's buffer
			pKeyName += CBMAXFIELDSPEC;		// increment pointer to next key in array
			pKeyValue += cbField;
		} 
	}

	if (pcbRet != NULL)
		*pcbRet = cbBufRet;					// return # bytes written into user buffer
	return (ercOK);

} // ReadRowMultiKey
*/


/*************************************************************************/
//
//	DeleteRow - searches for and deletes one or more rows that match one
//    specified key value
//
//	cRowsDelMax - if 0, will delete all rows that match;  otherwise the number
//					as specified.
//	pcRowsDel - will return the number of rows deleted even if buffer does not 
//		exist
//
/*************************************************************************/


int DeleteRow(char  *ptName,
			  char  *pKeyName,					// keyname to find
			  uchar *pKeyValue,					// key value to find
			  U32	cbKeyValue,
			  U32	cRowsDelMax,				// maximum number of rows to delete
			  U32   *pcRowsDel)					// count of rows deleted returned

{
	U32		 	*pRowsRet;					
	U32 	 	i, cRowsRet, cRows, erc, cbRowRead, cbRow;
	U32			cRowsDel = 0;
	uchar		*pRowBuf;
	rowID		*pTableId = NULL;
  	tableHeader *pCurrentHdr;
	entryDef	*pCurrentEntry;
	varLenFieldDef *pVarLenDef;
  	

cDeleteRow++;
if (cDeleteRow == 50)
	erc = 0;

	//  Get the table header;  if the table name is not defined, and the key is "rid" use the tableID 
	//	 to find the table

	if (pKeyName == NULL)
		return (ercBadParameter);
	if ((pKeyValue != NULL) && (ptName == NULL))
	{	// table ID can be taken from the keyvalue if no ptName	
		if (strcmp(pKeyName, CT_PTS_RID_FIELD_NAME) == 0)
			pTableId = (rowID *)pKeyValue;
	}	
	if (GetTableHeader(ptName, pTableId, &pCurrentHdr) != ercTableExists)
		return (ercTableNotfound);

	cRows = pCurrentHdr->cCurrentEntries;		// count of current rows
	if ((pRowsRet = new U32[cRows]) == NULL)	// for CompareField;  deleted at end
		return (ercNoMoreHeap);

	if ((CompareField(pCurrentHdr, pKeyName, pKeyValue, cbKeyValue, pRowsRet,
		(sizeof(U32) * cRows), &cRowsRet) == ercOK) && cRowsRet)
	{
		// if cRowsDelMax is defined, delete whichever is smaller: cRowsDelMax or cRowsRet
		if ((cRowsDelMax != 0) && (cRowsDelMax < cRowsRet))		
			cRowsRet = cRowsDelMax;

		// for each of the rows with a match...

		for (i = 0; i < cRowsRet; i++) 
		{	// adjust pointer to row to include links to next and previous entries
			pCurrentEntry = (entryDef*)(pRowsRet[i] - (sizeof(entryHdr)));  
			// copy row being deleted to a buffer so that can be sent to listener
			cbRow = ((pCurrentHdr->cbFixedLenData) + 
					(sizeof(varLenField) * (pCurrentHdr->cFields - pCurrentHdr->cFixedLenFields)));

			pRowBuf = new uchar[cbRow];		// deleted below
			if (pRowBuf != NULL)
				erc = GetVarLenRow (pCurrentHdr, pCurrentEntry, pRowBuf, cbRow, false, NULL);

			// if this is a var len row table, need to return heap entries
			if ((pCurrentHdr->persistFlags & VarLength_PT) != 0)
			{ // set up pointer to variable length structure if exist...
				cbRowRead = pCurrentHdr->cbFixedLenData;
				while (cbRowRead < pCurrentHdr->cbEntryData)
				{	// if vl fields actually exist, process...
					pVarLenDef = (varLenFieldDef*)((U32)&(pCurrentEntry->rowId) + cbRowRead);
					erc = DeleteEntryFromHeap (pCurrentEntry, pVarLenDef);
					cbRowRead += sizeof(varLenFieldDef);
				}
			}
			RemoveRowFromTable (pCurrentHdr, pCurrentEntry, false);
			// update the size of table to reflect removal of this row
			pCurrentHdr->cbCurrentTable -= pCurrentEntry->cbRow;			

			#ifdef PTSDEBUG
				dbgBufEntry newRecord = { "DlR", 0, pCurrentEntry->rowId};
				AddDebugEntry (&newRecord);
			#endif
			
			// delete flag in data area for debug; clear rest of row
			pCurrentEntry->rowId.Table = 0xde1;
			memset (&(pCurrentEntry->entrydata), 0, ((pCurrentHdr->cbEntryData) - sizeof(rowID)));
			// increment deleted row counter
			cRowsDel++;
			pCurrentHdr->version++;
				
	 		erc = SearchListenQueue	(pCurrentHdr, pRowBuf, (char*)NULL,
									 (U32) NULL, OpDeleteRow);
			delete [] pRowBuf;

		}  //end for
	
		pCurrentHdr->cCurrentEntries -= cRowsDel;
		if (pcRowsDel != NULL)
			*pcRowsDel = cRowsDel;
		
		delete [] pRowsRet;
		return (ercOK);
	} 
	// else key value not found
 	delete [] pRowsRet;
	if (pcRowsDel != NULL)
		*pcRowsDel = 0;

	return (ercKeyNotFound);
}	// DeleteRow


/*************************************************************************/
//
//
//	DeleteRowMultiKey - searches and returns one or more rows that match
//		  the specified key value.  
//
//		This procedure accepts an array of key values and returns the number
//		of rows that were deleted.  
//
/*************************************************************************/
//
/*
int DeleteRowMultiKey(char	*ptName,						// tablename
					 char  *prgKeyNames, U32 cbrgKeyNames,	// key names to find that correspond to
					 uchar *prgKeyValues,					// key values to find
					 U32	cRowsDelMax,					// location where rowIDs are returned
					 U32   *pcRowsDelRet)					// count of rows deleted


{
    U32 i, erc, cIdsRet, cKeyCommands, oField, iField, cbField;
	char			*pKeyName;
	uchar			*pKeyValue;
  	tableHeader		*pCurrentHdr;
	U32				cRowIdsRet = 0;						// # of rowIds written to user buffer


	pKeyName = prgKeyNames;
 	pKeyValue = prgKeyValues;

	// there are some inefficiencies in this code, as DeleteRow is later called, and
	// also calls GetTableHeader ... need to look into this

	if (GetTableHeader(ptName, NULL, &pCurrentHdr) != ercTableExists)
		return (ercTableNotfound);

	cKeyCommands = cbrgKeyNames/CBMAXFIELDSPEC;

	// prgKeyNames and prgKeyValues together define the one or more keys to be found;
	// treat each key separately
	// cKeyCommands could be derived by dividing the size of key array by size of key

	for (i=0; i < cKeyCommands; i++)   
	{
		erc = FindField(pCurrentHdr, pKeyName, &oField, &iField);
		cbField= pCurrentHdr->rgEntryDef[iField].cbField;

		// send the address of the return buffer, so the rows are stored directly

		erc = DeleteRow(ptName, pKeyName,  pKeyValue, 
			cRowsDelMax, &cIdsRet);
			// if a match was not found  ??? for now, keep going

		cRowIdsRet += cIdsRet;					// keep a running sum of the # of rows deleted
	}

	if (pcRowsDelRet != NULL)
		*pcRowsDelRet = cRowIdsRet;

	return (ercOK);
}*/

/*************************************************************************/
//
//	ModifyVarLenField - Modify a specific variable length field in an entry.
//	This is done by deleting the entry and then adding the new one.. 
//
//************************************************************************
 
int ModifyVarLenField(entryDef *pCurrentEntry, varLenFieldDef *pVLFinRow,
					 varLenField *pVarLenFields, uchar *pNewField)
{

	int erc;
	U32 cbData, oVLField;

 	// get info from new row ...
	cbData = pVarLenFields->cbVLField; 
	oVLField = pVarLenFields->oVLField;

	// limit check before field is deleted...
//	if ((oVLField + cbData) > cbRowData)
//		return (ercInVarLenFieldDef);
//  NEED TO MOVE ALL ERROR CHECKING TO BEFORE THE ROW IS MODIFIED
	if (cbData > rgiHeapToSize[iMaxHeaps])
 		return (ercInVarLenFieldDef);

	// get the current var len field struct...and delete from heap
	if (pVLFinRow->cbField > 0)
	{  // only delete if variable length field has data
		if ((erc = DeleteEntryFromHeap (pCurrentEntry, pVLFinRow)) != ercOK)
			// not good... error
			return (erc);
	}

	// update the table's row vlf structure...
	if (cbData == 0)
	{ // if variable length field has no length, just update the table info
		pVLFinRow->iHeapHdr = 0;
		pVLFinRow->oHeapEntry = 0;
		pVLFinRow->cbField = cbData;
	}
	else
	// otherwise add the entry to the heap
		erc = AddEntryToAHeap (pCurrentEntry, pVLFinRow, pVarLenFields, pNewField);
		
	return (erc);
}


/*************************************************************************/
//
//  ModifyRow - modify one or more row entries
//
//		prgKeyField - description of the key field names to search for;
//			 each entry is a separate command
//		cKeyCommands - number of commands 
//		prgRowBuf - array of rows that will replace the current rows;
//			 one to one correspondence with	prgKeyField
//		  currently, only one row supported... so cbRowBuf is size of row
//		  if this is a VLF, then size can be the fixed length portion or ALL
//			of the row	
//		prgRowIdsRet - array of row Ids that have been modified
//		cbMax - size of pRowIdsRet
//		pcRowsModRet - number of rows modified
//
//	RowIDs in modified rows remain unchanged

//	TODO:  need to do all checks before the row is modified
//
/*************************************************************************/
//

int ModifyRow ( char	*ptName,						// table name
				char	*prgKeyNames, U32 cbrgKeyNames,	// array of key names to find
				uchar	*prgKeyValues,					// array of key values to find
				U32		cbKeyValue,
				uchar	*prgRowBuf,						// array of replacement rows
				U32		cbRowBuf,
				Message	*pMsg,
				U32		*pcRowsModRet)					// count of IDs returned


{
    U32				i, j, k, cRowsRet, cKeyCommands, cRows, cVarLenFields;
	U32				oField, iField,  cbKeyField, cModifiedFields, cbMax, cbRowTotal, cbFixedLenData;
	U32				erc = ercOK;
  	pTableHeader 	pCurrentHdr;
	U32 			*pRowsRet;
	U32				cRowsToModify = 0;						// count of rows to modify from user
	uchar			*pDest, *pSource, *pKeyValue, *pRowBuf;
	char			*pKeyName, *prgModifiedFields;
	rowID			*prgIDsRet;
	entryDef		*pCurrentEntry;
	varLenField		*pVarLenFields; 
	varLenFieldDef	*pVLFinRow;
	
	cModifyRow++;

	if ((ptName == NULL) ||
		(prgKeyNames == NULL) || 
		(prgRowBuf == NULL ) ||
		(cbrgKeyNames > CBMAXFIELDSPEC ))
		return (ercBadParameter);

	if (GetTableHeader(ptName, NULL, &pCurrentHdr) != ercTableExists)
		return (ercTableNotfound);

	cVarLenFields = pCurrentHdr->cFields - pCurrentHdr->cFixedLenFields;
	if (cbRowBuf < pCurrentHdr->cbFixedLenData)
		return (ercRowSizeMisMatch);

	cRows = pCurrentHdr->cCurrentEntries;		// count of current rows
	if ((pRowsRet = new U32[cRows]) == NULL)	// for CompareField;  deleted at end
		return (ercNoMoreHeap);

	// set up local pointers into user buffers, and calculate number of rowIDs that
	// will fit into user return buffer, and number of keys sent

	if (pcRowsModRet != NULL)					// using parameter for in and out 
		cRowsToModify = *pcRowsModRet;			// save locally
		
	pKeyName = prgKeyNames;
 	pKeyValue = prgKeyValues;
	pSource = prgRowBuf;

	// restrict number of key commands for now... keep this code for now...(commented out)
	// cKeyCommands = cbrgKeyNames/CBMAXFIELDSPEC;
	// if (cKeyCommands > 1)
	cKeyCommands = 1;
	// treat each of the entries in the key arrays as one command and process separately
	for (i = 0; i < cKeyCommands; i++)   
	{
		// search for key field and size of field, used to move through row
		if ((FindField(pCurrentHdr, pKeyName, &oField, &iField)) != ercOK)
			erc = ercKeyNotFound;
		else
		{	// check size of key data
			cbKeyField= pCurrentHdr->rgEntryDef[iField].cbField;
			if (cbKeyField != cbKeyValue)
				erc = ercIncorrectFieldSize;
		}
 		if (erc != ercOK)
		{	// if error, get out...
			delete [] pRowsRet;
			return (erc);
		}
		if ((CompareField(pCurrentHdr, pKeyName, pKeyValue, cbKeyValue, pRowsRet,
			(sizeof(U32) * cRows), &cRowsRet)==ercOK) && cRowsRet)
		{ // if one or more rows were found where the key compares, modify those rows
			if ((cRowsToModify == 1) && (cRowsRet > 1))
			{	// key can only match one row if '1' specified
				delete [] pRowsRet;
				return (ercModifyMatchMoreThanOne);
			}
				// if user sends a '0', will modify all that match, otherwise use user's value
			if ((cRowsToModify != 0) && (cRowsToModify < cRowsRet))
				cRowsRet = cRowsToModify;

			// get a dynamically allocated reply buffer for the rowIDs
			cbMax = cRowsRet * sizeof(rowID);
			pMsg->GetSgl ( MODIFY_ROWS_REPLY_ROWIDS_SGI,	//  index of the SG return list of rowIDs 
						   (void **)&prgIDsRet, &cbMax);	//  pointer, size of return buffer of IDs
  
			// for each row that matches the key...
			for (j = 0; j < cRowsRet; j++)	 
			{	// get address of row that has match
				pDest= (uchar*)(pRowsRet[j]);	
				pCurrentEntry = (entryDef *)(pDest - sizeof(entryHdr));
				// update the size of table to reflect removal of this row
				pCurrentHdr->cbCurrentTable -= pCurrentEntry->cbRow;			
 				cbFixedLenData = pCurrentHdr->cbFixedLenData;		
 				cbRowTotal = cbFixedLenData;			// keep running total for later
				// save pointer to row in table for Listener
	  			pRowBuf = pDest;		
				prgModifiedFields = new char[CBMAXFIELDSPEC * CMAXENTRIES];
				// compare the new row to the current row, checking for modified fields for listeners
				erc = CompareRows (pCurrentHdr, pDest, pSource, prgModifiedFields, &cModifiedFields);
				// setup for copy of new row into table starting after rowID
				// copy rowID into returned array of rowIds
				if ((prgIDsRet != NULL) && (cbMax != NULL))
					*prgIDsRet++ = *((rowID*)pDest);
				// copy fixed length part of row into table;  if this is the entire row, we are done
				memcpy ((pDest + sizeof(rowID)), (pSource + sizeof(rowID)), cbRowTotal - sizeof(rowID));

				// now check for var len fields....
				if (((pCurrentHdr->persistFlags & VarLength_PT) != 0) &&
					 (cbRowBuf > (pCurrentHdr->cbFixedLenData + (cVarLenFields * sizeof(varLenField)))))
				{	// for each var len field, delete old and get new...
					pVarLenFields = (varLenField *)(pSource + cbFixedLenData);	// ptr to user vl structs
					pVLFinRow = (varLenFieldDef *)(pDest + cbFixedLenData);		// ptr to PTS vl struct
					for ( k = 0; k < cVarLenFields; k++)
					{ // check the offsets before writing field;  
						if ((pVarLenFields->oVLField + pVarLenFields->cbVLField) > cbRowBuf)
							return (ercInVarLenFieldDef);
						// call the routine to modify the variable length field... if error, get out...
						if ((erc = ModifyVarLenField (pCurrentEntry, pVLFinRow, pVarLenFields,
								(uchar *)((U32)pSource + pVarLenFields->oVLField))) != ercOK)
						{	// not good... error
							delete [] prgModifiedFields;
							delete [] pRowsRet;
							return (erc);
						}
						// keep running sum of row length;  this includes the user's vlf struct
						cbRowTotal += (pVarLenFields->cbVLField + sizeof(varLenField));
						// advance pointer into client's buffer
						pVarLenFields++;		// point to next vlf struct in client row			
						pVLFinRow++;			// pointer to next vlf struct in table row
					} // end for cVarLenFields
				}  // endif variable length fields
				// store the row size in row and add row size to total table size
				pCurrentEntry->cbRow = cbRowTotal;				
		 		pCurrentHdr->cbCurrentTable += cbRowTotal;			

				#ifdef PTSDEBUG
					dbgBufEntry newRecord = { "MR", 0, pCurrentEntry->rowId};
					AddDebugEntry (&newRecord);
				#endif
				// search the listen queue for modify row listeners; if any are found,
				// pointers to messages are put on a linked list at pListenReplies;
				// the ddm then replies to messages
				pCurrentHdr->version++;

 				erc = SearchListenQueue	(pCurrentHdr, pRowBuf, prgModifiedFields,
										 cModifiedFields, OpModifyRow);
				delete [] prgModifiedFields;
			}  // end for each matching row
		}
 		// else if the key was not found, set an error
		else
			erc = ercKeyNotFound;

		// advance pointers to next key
		//pKeyName += CBMAXFIELDSPEC;
		//pKeyValue += cbKeyField;
		//pSource += cbEntryData;
	}  // end for loop, get next command
 
	if (pcRowsModRet != NULL)
		*pcRowsModRet = cRowsRet;
	delete [] pRowsRet;
	return (erc);

}	//  ModifyRow

		

/*************************************************************************/
//
//	ModifyField - searches for a key field and modifies same or another
//		field within that row.
//	This procedure accepts an array of key values and an array of fields
//	to modify.  The array of Keys, keyvalues, fields and fieldvalues have
//  a one to one correspondence.  
//
//	An array of rowIds is returned in prgRowIdsRet if it is defined;
//	pcRowsModRet - returns the count of rows that have been modified.
//  todo:  need to make sure all boundary checks are done prior to modifications
/*************************************************************************/


int ModifyField(char  *ptName,							// tablename
				char  *prgKeyNames,U32 cbKeyNames, 		// array of key name to find
 				uchar *prgKeyValues,				 	// array of key values to fine
				U32	  cbKeyValue,
 				char  *prgFieldNames,				 	// array of field names to modify
  				uchar *prgFieldValues,					// array of field values to modify
				U32	  cbFieldValue,
				Message	*pMsg,
    			U32	  *pcRowsModRet)					// count of rows modified


{
    U32 	i, j, cRowsRet, cbKeyField, cbField, cKeyCommands, iField, oField;
	uchar	*pDest;
	U32		*pRowsRet;
	U32		cRows, cRowsToModify, cbMax, iVLF;
	pTableHeader pCurrentHdr;
	char	*pKeyName;
	uchar	*pKeyValue;
 	char	*pFieldName;
	uchar	*pFieldValue;
	U32		cRowsMod = 0;					// # of rows modified
	U32		erc = ercOK;
	rowID	*prgRowIdsRet; 
	varLenField userVLF;
	varLenFieldDef *pVLFinRow;
	entryDef *pCurrentEntry;
	
	if  ((ptName == NULL) ||
		 (prgKeyNames == NULL) || 
		 (prgFieldNames == NULL ) ||
		 (prgFieldValues == NULL ) ||
		 (cbKeyNames > CBMAXFIELDSPEC ))
	   return (ercBadParameter);

	if (GetTableHeader(ptName, NULL, &pCurrentHdr) != ercTableExists)
		return (ercTableNotfound);

	if (pcRowsModRet != NULL)				//  using parameter for in and out 
		cRowsToModify = *pcRowsModRet;

	// set up temporary pointers to user's parameters
	pKeyName = prgKeyNames;
 	pKeyValue = prgKeyValues;
	pFieldName = prgFieldNames;
 	pFieldValue = prgFieldValues;

	cRows = pCurrentHdr->cCurrentEntries;		// alloc memory for CompareField; delete at end
	if ((pRowsRet = new U32[cRows]) == NULL)
		return (ercNoMoreHeap);
	
	// keyNames must be sent in fixed length strings
	// cKeyCommands = cbKeyNames/CBMAXFIELDSPEC;
	// if (cKeyCommands > 1)
	
	cKeyCommands = 1;						// limit cKeyCommand feature for now
	
	for (i=0; i < cKeyCommands; i++)   
	{
		// modification of rid not allowed
		if (strcmp(pFieldName, CT_PTS_RID_FIELD_NAME) == 0)
		{	delete [] pRowsRet;
			return (ercNoModRowId);
		}
		// find the key field so can get the size of the key for 'multi' key ops
		if ((FindField(pCurrentHdr, pKeyName, &oField, &iField)) != ercOK)
		{
			delete [] pRowsRet;
			return (ercKeyNotFound);
		}	

		cbKeyField= pCurrentHdr->rgEntryDef[iField].cbField;

		if ((CompareField(pCurrentHdr, pKeyName, pKeyValue, cbKeyValue,	pRowsRet,
			(sizeof(U32) * cRows), &cRowsRet)== ercOK) && cRowsRet)
		{
			// if user sends a '0', will modify all that match, otherwise check user's value
			if ((cRowsToModify != 0) && (cRowsToModify < cRowsRet))
				cRowsRet = cRowsToModify;

			// get the dynamic allocated reply buffer for the rowIDs
			cbMax = cRowsRet * sizeof(rowID);
			pMsg->GetSgl( MODIFY_FIELDS_REPLY_ROWIDS_SGI,		//  index of the SG return list of rowIDs 
						  (void**)&prgRowIdsRet, &cbMax);		//  pointer, size of return buffer of IDs
			
			// get the info on the field that is to be modified
			if ((erc = FindField(pCurrentHdr, pFieldName, &oField, &iField)) == ercOK)
			{	// get the offset of field from rowID
				oField-= sizeof(entryHdr);
				// for each row, modify the field 
				for (j= 0; j < cRowsRet; j++) 
				{	// point to row to modify
					pDest= (uchar*)(pRowsRet[j]);
					// if this field is VLF, then need to delete and add...
					if ((pCurrentHdr->rgEntryDef[iField].persistFlags & VarLength_PT) != 0)
					{	
						pCurrentEntry = (entryDef *)(pDest - sizeof(entryHdr));
						iVLF = iField - pCurrentHdr->cFixedLenFields;
						userVLF.cbVLField = cbFieldValue;
						userVLF.oVLField = pCurrentHdr->cbFixedLenData + (iVLF * sizeof(varLenField));
						pVLFinRow = (varLenFieldDef *)(pDest + oField);				// ptr to PTS vl struct
						erc = ModifyVarLenField (pCurrentEntry, pVLFinRow, &userVLF, pFieldValue);
						if (erc != ercOK)
						{
							delete [] pRowsRet;
							return erc;
						}
						if ((prgRowIdsRet != NULL) && (cbMax != NULL))
								*(prgRowIdsRet++) = *((rowID*)pDest);
						cRowsMod++;
					}
					else
					{
						cbField= pCurrentHdr->rgEntryDef[iField].cbField;
						// check to make sure sizes match...
						if (cbFieldValue == cbField)
						{ // for each of the row pointers returned...

						#ifdef PTSDEBUG
							dbgBufEntry newRecord = { "MF", 0, pCurrentEntry->rowId};
							AddDebugEntry (&newRecord);
						#endif

							// only modify field and send listens if field changed...
							if ((memcmp((pDest + oField), pFieldValue, cbField)) != 0)
							{
								if ((prgRowIdsRet != NULL) && (cbMax != NULL))
									*(prgRowIdsRet++) = *((rowID*)pDest);
								memcpy ((pDest + oField), pFieldValue,  cbField);
								cRowsMod++;
								// search the listen queue for modify listens;  if any are found, pointers
								// to messages are put in rgListenReplies;  the ddm then replies to messages

								pCurrentHdr->version++;

 								erc = SearchListenQueue	(pCurrentHdr, (uchar*)(pRowsRet[j]),
											 pFieldName, (U32)1,  OpModifyField);
							}
						} // end if cb ==
						else
							erc = ercIncorrectFieldSize;
					} // end else
				} // end for 
			} // cannot find field to modify
			else
				erc = ercFieldNotFound;
		}  //  endif CompareField
		else  
			erc = ercKeyNotFound;

		// advance pointers to next entry
		pKeyName += CBMAXFIELDSPEC;
		pKeyValue += cbKeyField;
		pFieldName += CBMAXFIELDSPEC;
		pFieldValue += cbField;
		
	} // end for loop
	
	delete [] pRowsRet;
	if (pcRowsModRet != NULL)
		*pcRowsModRet = cRowsMod;
	
  return (erc);
}  // ModifyField


/*************************************************************************/
//
//	ModifyBitsInField - searches for a key field and modifies same or another

/*************************************************************************/

int ModifyBitsInField (char  *ptName,					// tablename
				char	*pKeyName,				 		// array of key name to find
 				uchar	*pKeyValue,					 	// array of key values to find
				U32		cbKeyValue,
 				char	*pFieldName,					// array of fields names to modify
  				U32		*pBitMaskValue,					// array of field values to modify
				fieldOpType	OpBitField,
				Message	*pMsg,
    			U32		*pcRowsModRet)					// count of rows modified

{
    U32 	j, cRowsRet, cbField, iField, oField;
	U32		*pDest;
	U32		*pRowsRet;
	U32		cRows, erc, currentValue, cRowsToModify, cbMax;
	pTableHeader pCurrentHdr;
	U32   cRowsMod = 0;					// # of rows modified
	rowID	rowIdSave;
	modField  *pFieldsRet;


	if  ((ptName == NULL) ||
		 (pKeyName == NULL) || 
		 (pFieldName == NULL ) ||
		 (pBitMaskValue == NULL ))
	  return (ercBadParameter);

	if ((OpBitField != OpAndBits) && (OpBitField != OpOrBits) &&
		(OpBitField != OpXorBits))
		  return (ercBadParameter);

	if (GetTableHeader(ptName, NULL, &pCurrentHdr) != ercTableExists)
		return (ercTableNotfound);

	if (pcRowsModRet != NULL)				//  using parameter for in and out 
		cRowsToModify = *pcRowsModRet;
		
	// modification of rid not allowed
	if (strcmp(pFieldName, CT_PTS_RID_FIELD_NAME) == 0)
		return (ercNoModRowId);

	erc = ercOK;
	cRows = pCurrentHdr->cCurrentEntries;		// count of current rows
	if ((pRowsRet = new U32[cRows]) == NULL)
		return (ercNoMoreHeap);

	if ((CompareField(pCurrentHdr, pKeyName, pKeyValue, cbKeyValue,	pRowsRet,
		(sizeof(U32) * cRows), &cRowsRet) == ercOK) && cRowsRet)
	{
		// if user sends a '0', will modify all that match, otherwise check user's value
		if ((cRowsToModify != 0) && ( cRowsToModify < cRowsRet))
			cRowsRet = cRowsToModify;
		
		// get the dynamic reply buffer for rowIDs
		cbMax = cRowsRet * sizeof(modField);
		pMsg->GetSgl ( MODIFY_BITS_REPLY_ROWIDS_SGI,		//  index of the SG return list of rowIDs 
					  (void**)&pFieldsRet, &cbMax);			//  pointer, size of return buffer of IDs

		if ((erc = FindField(pCurrentHdr, pFieldName, &oField, &iField)) == ercOK)
		{	
			oField-= sizeof(entryHdr);
			cbField= pCurrentHdr->rgEntryDef[iField].cbField;

			// for each of the row pointers returned...
			for (j= 0; j < cRowsRet; j++) 
			{
				pDest= (U32 *)(pRowsRet[j]);				// pointer to row to modify
				rowIdSave = *((rowID*)pDest);
				// copy the new field into table

				pDest =  (U32 *) ((U32)pDest + oField);		// pointer to field to modify
				currentValue = *pDest;					// save the field value
														
				switch (OpBitField)
				{
					case OpAndBits:
					{
						*pDest &= (*pBitMaskValue);		//	AND field with mask
						break;
					}
					case OpOrBits:
					{
						*pDest |= (*pBitMaskValue);		//	OR field with mask
						break;
					}
					case OpXorBits:
					{
						*pDest ^= (*pBitMaskValue);		//	XOR field with mask
						break;
					}
				} // end switch
				// if the field value has changed, send the listen, otherwise do not

				if (currentValue != *pDest)
				{
					cRowsMod++;
					pCurrentHdr->version++;
 					erc = SearchListenQueue	(pCurrentHdr, (uchar*)(pRowsRet[j]),
											 pFieldName, (U32)1,  OpModifyField);
				}
				if ((pFieldsRet != NULL) && (cbMax != NULL))
				{	// even if the field stayed the same, allow client to see current value
					pFieldsRet->rowModified = rowIdSave;
					pFieldsRet->fieldRet = *pDest;
					pFieldsRet++;
				}
			} // end for each row
		} // endif:  cannot find field to modify
		else
			erc = ercFieldNotFound;
	}  //  endif compareField
	else
		erc = ercKeyNotFound;
	 // end if comparefield
	
	delete [] pRowsRet;
	if (pcRowsModRet != NULL)
		*pcRowsModRet = cRowsMod;
	
  return (erc);
}  // ModifyBitsField


/**********************************************************************************/
//
//	TestAndSetOrClearField - searches for a key field and modifies same or another
//		field within that row.
//		Works for a 32 bit field only
/**********************************************************************************/
//


int TestAndSetOrClearField (char  *ptName,		// tablename
			fieldOpType opField,				// test and set, or clear
			char  *pKeyName,					// key name to find
 			uchar *pKeyValue,				 	// key value to find
			U32	  cbKeyValue,
 			char  *pFieldName,				 	// field name to test and set
    		BOOL  *pTestRet)					// pointer to test result


{
    U32 	j, cRowsRet, iField, oField;
	U32		*pDest;
	U32		*pRowsRet;
	U32  	erc, cRows;
	pTableHeader pCurrentHdr;


	if  ((ptName == NULL) || (pKeyName == NULL) || (pKeyValue == NULL) ||
		 (pFieldName == NULL ))
	  return (ercBadParameter);

	if (GetTableHeader(ptName, NULL, &pCurrentHdr) != ercTableExists)
		return (ercTableNotfound);

	erc = ercOK;
	cRows = pCurrentHdr->cCurrentEntries;		// count of current rows

	// modification of rid not allowed
	if (strcmp(pFieldName, CT_PTS_RID_FIELD_NAME) == 0)
		return (ercNoModRowId);
	if ((pRowsRet = new U32[cRows]) == NULL)
		return (ercNoMoreHeap);
	// find all rows that match the key...	
	if ((CompareField(pCurrentHdr, pKeyName, pKeyValue,	cbKeyValue, pRowsRet,
			(sizeof(U32) * cRows), &cRowsRet)== ercOK) && (cRowsRet != 0))
	{
		// if one or more rows were found where the key compares, ?????
 		//  search the array of field descriptions to find the key field
		iField = 0;				//  iField is the field index
		oField = 0;				//	oField is the byte offset of the field within the row

		if ((erc = FindField(pCurrentHdr, pFieldName, &oField, &iField)) == ercOK)
		{	
 			oField-= sizeof(entryHdr);
			if ( pCurrentHdr->rgEntryDef[iField].cbField > 4)
			// only support 4 byte fields now
			{
				delete [] pRowsRet;
				return (ercBadParameter);
			}
			// cRowsRet:  limit now to 1 until decide how to return information to user
			if (cRowsRet > 1)
				cRowsRet = 1;
				
			// for each of the row pointers returned...
			for (j= 0; j < cRowsRet; j++) 
			{
				pDest= (U32*)((uchar*)(pRowsRet[j]) + oField);	// pointer to field to modify
				if (opField == OpTestSetField)
				{ // if field is set, return false, no change 
					if (*pDest == true)
						*pTestRet = false;
					else // if the field is not set, set it and return true that set
					{
						*pTestRet = true;
						*pDest = true;
					}
				}
				else if (opField ==	OpClearField)
				{ // if field is set, clear, set testRet flag:  change 
					if (*pDest == true)
					{
						*pDest = true;
						*pTestRet = true;
					}	
					else // if the field is not set, return false: no change
					{
						*pTestRet = false;
					}
				}
				if (*pTestRet == true)
				{  // if a change was made to the field, search the listen queue for modify listens;
					pCurrentHdr->version++;
					erc = SearchListenQueue	(pCurrentHdr, (uchar*)(pRowsRet[j]),
										 	 pFieldName, (U32)1,  OpModifyField);
				}						 
			}	// end for

		} // cannot find field to modify
		else
			erc = ercFieldNotFound;

	}  //  endif compareField
	else
		erc = ercKeyNotFound;
	// end if comparefield

  delete [] pRowsRet;
  return (erc);
}  // TestandSetField

/*************************************************************************/
//
//	QueryOrSetRID
//
//  Allows client to query the next rowID to be assigned for a given table
//	or bump it to a given rowID.
//
//	Returns the table id as defined in rowID;
//
/*************************************************************************/


int QueryOrSetRID (char *ptName,			// table name
				 fieldOpType opRID,			// query or set rowID
				 rowID	*pRowID )			// rowID returned or new
		 										

{
 		pTableHeader	pCurrentHdr;		

		// get Table header		
		if (GetTableHeader(ptName, NULL, &pCurrentHdr) != ercTableExists)
		{ return (ercTableNotfound); }
		// check for non null pointer
		if (pRowID == NULL)
		{	return (ercBadParameter); }

		// if query, return the current rowID to the client
		if (opRID == OpQueryRID)
		{  *pRowID = pCurrentHdr->rowId; }

		else
		{	// if setRID, modify the rowID in the table header
			// should we not test for validity???
			if ((opRID == OpSetRID) && (pRowID->LoPart > pCurrentHdr->rowId.LoPart))
			{
				pCurrentHdr->rowId.LoPart = pRowID->LoPart;
				pCurrentHdr->version++;
 			}
			else
			{ return (ercBadParameter); }
		}
		return (ercOK);

} //QueryOrSetRowID



/*******************************************************************************/
//
//	InitializeListenerIDs
//
//	Allocate an array of listenerID blocks.  These blocks are used to locate listeners for
//	a StopListen request.  A free list is kept so that the blocks can be reused.  When there
//	are no available blocks, this routine is called to allocate another array of blocks.
//	The first block is used as a header that links the arrays of allocated blocks.
//
	
int InitializeListenerIDs (U32 cListenIds, U32 tableState)

{

	U32 i, nextListenerId;
	listenerIdMap	*pLastListenerIds;

	if (tableState == INITIALIZE )									// if these are the first listenerIds 
	{																
		pListenerIds = new listenerIdMap[cListenIds + 1];			//	add one for a header
		nextListenerId = 0;
		if (pListenerIds)
		{
			pListenerIds[0].listenerID = cListenIds;				//	header of list:  count allocated in block
			pListenerIds[0].listenMode = ListenIdHeader;			//	header of list:  header ID
			pListenerIds[0].pNext = NULL;							//	pointer to next set of blocks (none now)
			pFreeListenerId = &pListenerIds[1];						//	for initialization, the next block is first of 
		}
		else
			return (ercNoFreeHeaders);
 	}
	else	// tablestate = augment
	{																// otherwise, 
		pLastListenerIds = pListenerIds;							//	go through the listenerID headers (first
		nextListenerId = pListenerIds->listenerID;					//	of block) until the last block found

		while (pLastListenerIds->pNext != NULL)						//  find last block of headers
		{
		  pLastListenerIds = pLastListenerIds->pNext;
		  nextListenerId += pLastListenerIds->listenerID;			//  keep track of # of listenerIds used										
		}
		pLastListenerIds->pNext = new listenerIdMap[cListenIds+1];  // get a new block of ids
		if (pLastListenerIds != NULL)
		{
			pLastListenerIds = pLastListenerIds->pNext;				//  initialize last block header
			pLastListenerIds->pNext = NULL;
			pLastListenerIds->listenerID = cListenIds;
			pLastListenerIds->listenMode = ListenIdHeader;
			pFreeListenerId = &pLastListenerIds[1];					// initialize free list header
		}
		else
			return (ercNoFreeHeaders);
	}
	// link free listener ID blocks starting at the first block as the zeroth block
	// is being used for an array header

	for (i = 0; i < cListenIds ; i++)
	{
		pFreeListenerId[i].pNext = &pFreeListenerId[i+1];
		pFreeListenerId[i].listenerID = ++nextListenerId;			// id is index + 1
		pFreeListenerId[i].listenMode = FreeListener;
	}

	pFreeListenerId[cListenIds-1].pNext = NULL;					// null pointer to next block

#ifdef PTSDEBUG
	pDebugBuffer = (dbgBufEntry *)(new (tBIG|tUNCACHED|tZERO) uchar [cbDebugBuffer]);
	pNextEntry = pDebugBuffer;
#endif

	return ercOK;

}


/*******************************************************************************/
//
//	InitializeHeaps 
// 
//	Initialize the heaps that will contain the variable length fields.  The number
//	of heaps and the size of the entries are defined in the array rgiHeapToSize
//  above.  The heaps are defined as tables of fixed length records of the given 
//	size.  Each heap has a header that describes the entries;  the heap headers 
//	are stored in the table header blob.
//
/*******************************************************************************/



int InitializeHeaps ()

{ 
	U32 i;
	heapHeader	*pHeapHeader;

	for (i = 0; i < iMaxHeaps; i++)
	{
 		pHeapHeader = (heapHeader *)&(pHeaders->rgHeapHdr[i]);		

 	    pHeapHeader->cEntriesAlloc = rgiHeapToCountEntries[i];

		pHeapHeader->cbEntryData = rgiHeapToSize[i];
		pHeapHeader->cbEntry = rgiHeapToSize[i] + sizeof(heapEntryDef);
		pHeapHeader->cCurrentEntries = 0;

		pHeapHeader->pTableData =  AllocFromTableHeap (pHeapHeader->cbEntry, pHeapHeader->cEntriesAlloc);

		if (pHeapHeader->pTableData == NULL)
		{
			pHeapHeader->cbEntryData = 0;
			pHeapHeader->cbEntry = 0;
			rgiHeapToSize[i] = 0;
			return 	ercNoMoreDataHeap;
		}
		
		pHeapHeader->oTableData = (U32)(pHeapHeader->pTableData) - (U32)prgDataBlks;

		// points to first free entry ready for data in data blob
		pHeapHeader->oFreeEntry = (U32)(&(pHeapHeader->pTableData->data)) - (U32)prgDataBlks;	

	}


	 return(ercOK);
}



/***********************************************************************************/
//
//	CreateTableOfTables
//	Initialize the mother of all tables:  TableOfTables
//
/**********************************************************************************/

int CreateTableOfTables (char *ptName, U32	cEntries)

{
	U32  erc;
	rowID	tot_tableId;

	fieldDef	TableOfTable_FieldDefs[] = {
//		"rowID",	8,	ROWID_FT, Persistant_PT,
		"tableName", sizeof(String64), STRING64_FT, Persistant_PT,
		"tableId",	8,	ROWID_FT, Persistant_PT,
	};
	
	erc = DefineTable (ptName, (fieldDef *)&TableOfTable_FieldDefs, sizeof(TableOfTable_FieldDefs),
						cEntries, TableOfTables_PT | Persistant_PT, &tot_tableId);

	// if (erc != ercOK)
	//	need to ?????

	return(ercOK);
}


//**************************************************************************************************
//
//
// $Log: /Gemini/Odyssey/DdmPTS/PtsProcs.cpp $
// 
// 57    2/15/00 12:06p Sgavarre
// up the size of the images... addresses defect DFCT13023
// 
// 56    1/26/00 2:36p Joehler
// Added very long fields for Sherri.
// 
// 55    11/22/99 4:08p Sgavarre
// add tableIDret to Listen payload
// 
// 54    11/21/99 8:28p Jlane
// Remove flush if 155 tables.
// 
// 53    11/10/99 9:28a Joehler
// remove double increment of cbRow in InsertVarLenRow
// 
// 52    11/04/99 1:21p Sgavarre
// add ClearNonPersistFields;  add DID field type;  
// 
// 51    10/28/99 9:50a Sgavarre
// Persist tables to flash;  VLF: if errors, back out changes to tables
// 
// 50    10/14/99 3:16p Jlane
// Don't return erc if key value is null as a temp hack to support
// ALL_ROWS name.
// 
// 49    10/07/99 1:54p Sgavarre
// In enumerate table, if no rows, do not getsgl...  returns error
// 
// 48    10/06/99 4:44p Sgavarre
// add variable length fields
// 
// 47    10/02/99 1:56p Agusev
// Added Unicode types for field defs
// 
// 46    9/18/99 4:40p Sgavarre
// fix bug found by debug heap:  if listen header deleted, tempPointer
// becomes bogus
//
// 45    9/07/99 2:17p Sgavarre
// add temp pointer to SearchListen so pKeyName is not modified
// 
// 44    9/04/99 4:38p Agusev
// Bumped up the size of the PTS heap to 400000 bytes
// Added an assert() for the case when the PTS runs out of available heap
// blocks
// 
// 43    9/03/99 7:17p Ewedel
// Fixed bugs in debug helper ChkFieldDefs().  :-)  Also, removed dead-end
// loops, so that additional errors may be reported (TRN & ewx).
// 
// 42    9/03/99 3:05p Tnelson
// Fixed ChkFieldDefs to be 'const' safe
// 
// 41    8/30/99 9:10a Sgavarre
// update enumtable counter
// 
// 40    8/30/99 8:24a Jhatwich
// added casts for WIN32
// 
// 39    8/28/99 6:00p Sgavarre
// add dynamic sgls for return data;  add vdn type;  add more error
// checking;  change compareRow so is key is rid, inserts tableId in RowId
// is NULL;  move listenreplytype to payload
// 
// 38    8/16/99 4:06p Sgavarre
// clear all of the table headers
// 
// 37    8/16/99 3:18p Sgavarre
// win32 fix
// 
// 36    8/13/99 5:38p Sgavarre
// ModifyBitsinField;  Add listen features:  returnRowsonInitialListen,
// dynamic sgl for rows returned on listen; listens only returned on
// ModifyField when field actually changed;  ListenOnDefineTable returns
// TableOfTables;  Stop listen messages returned with stop status;  send
// listen replies after each row operation instead of after function
// (buffer was bogus otherwise)
// 
// 33    7/23/99 2:52p Sgavarre
// Add QuerySetRID, TestSetOrClearField, Table of tables
// 
// 32    7/12/99 2:04p Sgavarre
// DeleteTable can take the tableId as a parameter.
// Add listen for deleteTable: If listeners on a table, and it is deleted,
// sends last listen replies to all.
// 
// 31    7/06/99 3:11p Hdo
// DefineTable: return TableID when table exist
// 
// 30    7/01/99 6:03p Mpanas
// Make sure advance the src pointer after a
// memcpy() in the InsertRow() code
// 
// 29    6/24/99 12:57p Sgavarre
// Store and retrieve fieldtype from the table header for DefineTable and
// GetTableDef.
// 
// 28    6/22/99 3:54p Jhatwich
// updated for windows
// 
// 27    6/16/99 12:11p Sgavarre
// If ReplywithTable was not set in Listen, the Initial reply type was not
// being returned.
// 
// 26    6/15/99 7:06p Sgavarre
// fix listen for InsertRow and DeleteRow so can return row;
// add null check in SearchListenQueue if no modified row passed in;
// 
// 25    6/08/99 3:55p Sgavarre
// Add dynamic expansion of tables as they grow via InsertRow;
// Support tableID in addition to tablename string; (not supported in
// interface yet)
// Implement all modify listener types:  changes to GetListenHeader,
// ReturnListenHeader, SearchListenQueue, StopListen
// New parameters for  ListenTable to support dynamic buffers and
// listentypes
// New SearchForListenerId, CompareRows, FieldMatch
//
// 24    6/08/99 8:04p Tnelson
// Fix all warnings...
// 
// 23    5/20/99 9:26p Mpanas
// Remove Version 20 change
// 
// 22    5/19/99 9:20a Agusev
// Corrected a problem
// 
// 21    5/18/99 12:20p Sgavarre
// fix ModifyRow so that if the client requested the rowID, the pointer to
// the table would be advanced correctly and the row would be modified
// after the rowId, not starting at the rowID
// 
// 20    5/14/99 1:39p Agusev
// WIN32 special treatment case
// 
// 19    5/11/99 10:54a Jhatwich
// win32
// 
// 18    4/16/99 6:14p Jlane
// Modifications made debugging Listen and enhancing for Dynamic Reply SGL
// items.
// 
// 
// 17    4/05/99 7:18p Jlane
// In CompareFields only decrement cRows for found matches.
// 
// 16    4/03/99 6:28p Jlane
// In SearchListenQueue Don't return ercNoLIsteners it's not a crime not
// to listen.
// 
// 15    3/28/99 12:22p Sgavarre
// fix GetTableDef to return size of user's data, not internal size of row
// 
// 14    3/27/99 5:04p Sgavarre
// moved some of globals back to local
// 
// 13    3/27/99 1:11p Sgavarre
// Check for null in parameters;  Include a tableId in rowID (cttypes.h); 
// Modify deleteRows, InsertRows, ReadRows, ModifyRows/fields to retrun
// count of rows worked on;  Add listen and stopListen code with all
// supporting structures and routines;  modifty GetTableHeaders to take a
// tableId;
// 
// 12    3/17/99 10:15p Jlane
// In InsertRow check for non Null prgIDsRet before returning IDs there.
// 
// 11    3/16/99 7:40p Mpanas
// Fixed pRowIdsRet = NULL problem
// 
// 10    3/12/99 6:31p Mpanas
// Add a way to change the size of the Table allocated
// 
// 9     3/12/99 5:43p Ewedel
// Added error check to verify that table definition does not define more
// fields than PTS internal structures can handle.
// 
// 8     3/11/99 5:57p Ewedel
// Tweaked so that an input field def byte count == 0 is interpreted as
// "just enough space for one of whatever the field's data type is."  Also
// added VSS log.
// 
// 02/10/99	JFL:	Check for NULL before asssigning to *pcIDsRet in Insert.
// 11/10/98			 modify InsertRow, ModifyRow for input rows with rowID
// 11/1/98			 fixup for Metrowerks environment, OOS link
// 10/20/98			 Update with debugged code
// 9/15/98 sgavarre: Create file
/*************************************************************************/



