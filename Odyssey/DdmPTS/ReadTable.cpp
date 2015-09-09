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
// This file is the declaration of an object interface to Read an entire
// Table from the Persistant Table Service.
// 
// $Log: /Gemini/Odyssey/DdmPTS/ReadTable.cpp $
// 
// 14    7/22/99 3:11p Hdo
// 
// 13    7/21/99 7:02p Hdo
// Change the return type of Send() from STATUS to void
// Call the user callback routine when error
// Add error checking
// 
// 12    7/14/99 2:17p Sgavarre
// Add parameter to GetTableDef that specifies the number of field per row
// 
// 11    6/22/99 3:55p Jhatwich
// updated for windows
// 
// 10    6/07/99 8:05p Tnelson
// Fix all warnings...
// 
// 9     5/11/99 10:54a Jhatwich
// win32
// 
// 8     4/03/99 6:32p Jlane
// Multiple changes to interface, messaage payloads and message handlers
// to avoid having to add reply payloads as this destroys SGL items.  We
// do this by allocating space for reply data in the msg payload.
// 
// 7     3/19/99 7:04p Ewedel
// Removed obsolete context values, fixed to return the count of rows read
// by deriving it from the bytes read by EnumTable.
// 
// 6     3/17/99 7:22p Ewedel
// Corrected some new/delete usage problems.
// 01/21/99	JFL	Compiled (Except for callback madness) and checked in.  
// 12/28/98	JFL	Created.
//
/*************************************************************************/

#include "Trace_Index.h"
#define	TRACE_INDEX		TRACE_PTS
#include "Odyssey_Trace.h"

#include "ReadTable.h"
 
/**************************************************************************/
// Initialize - Specify parameters for the TSReadTable operation.
//
// Parameters:
//	pDdmServices	- Pointer to Caller's Ddm.  Used for reply dispatch.
//	TableName		- The name of the table on which you want to listen. 
//	ppTableDataRet	- returned table data.
//  pcRowsRet		- returned # of rows read
//	pClientCallback	- Caller's function to be executed upon listen reply.
//	pContext		- Context pointer that is passed to the Callback method
//
/**************************************************************************/
STATUS TSReadTable::Initialize(
								DdmServices*	pDdmServices,
								String64		TableName,			// Name of table to read.
								void**			ppTableDataRet,		// returned table data.
								U32*			pcRowsRet,			// returned # of rows read
								pTSCallback_t	pClientCallback,	// Client's callback function.
								void*			pClientContext		// Passed to callback function.
							 )
{
	TRACE_ENTRY(TSReadTable::Initialize);

	// Initialize our base class.
	if( pDdmServices == NULL )
		return ercBadParameter;
	SetParentDdm(pDdmServices);

	// Stash parameters in member variables.
	m_pDdmServices = pDdmServices;
	if( TableName == NULL )
		return ercBadParameter;

	strcpy( m_TableName, TableName );
	m_ppTableDataRet = ppTableDataRet;
	m_pcRowsRet = pcRowsRet;
	m_pClientCallback = pClientCallback;
	m_pClientContext = pClientContext;

	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// Send() - reads the entire table specified and returns the table data.
// This is done in several steps.  The first step performed by this method
// which will initialize() and send() a GetTableDef object to find out
// how big the table is and thus how much memory must be allocated for it.
// NOTE: Memory for the table will be allocated and it's the responsibility
// of the caller to dispose of that memory when they're finished using it.
/*************************************************************************/
void TSReadTable::Send()
{
	TRACE_ENTRY(TSReadTable::Send);

	STATUS			status;

	m_GetTableDef = new TSGetTableDef;

	status = m_GetTableDef->Initialize( this,
							m_TableName,
							NULL,			// prgFieldDefsRet - We dont want em.
							0,				// cbrgFieldDefsret
							NULL,			// pcFieldDefsRet - we don't want it.
							&m_cbRow,		// Size of table row in bytes.
							&m_cRows,		// Number of table rows.
							&m_cFieldsRow,	// number of fields per row
							NULL,			// pfpersistant - We don't care.
							TSCALLBACK(TSReadTable, HandleGetTableDefReply),	// our callback.
							NULL			// passed to the above routine
							);

	if( status != OS_DETAIL_STATUS_SUCCESS )
	{
		TRACEF(TRACE_PTS, ("TSReadTable::Send status=%d\n", status));
		if( m_pDdmServices )
			(m_pDdmServices->*m_pClientCallback)( m_pClientContext, status );
		delete this;
	}
	else
		m_GetTableDef->Send();
}


/************************************************************************/
// HandleGetTableDefReply - Handles the reply from the GetTableDef
// operation initiated in Send().  Calculates the size of the buffer
// needed to hold the table allocates a buffer and then initiates an 
// enumerate table operation to read the table rows into the buffer.
/************************************************************************/							  
STATUS TSReadTable::HandleGetTableDefReply(void *pClientContext, STATUS status)
{
	TRACE_ENTRY(TSReadTable::HandleGetTableDefReply);
#ifndef WIN32
	#pragma unused(pClientContext)
#endif

	if (status != OS_DETAIL_STATUS_SUCCESS) 
	{
		TRACEF(TRACE_PTS, ("TSReadTable::HandleGetTableDefReply status=%d\n", status));
		if( m_pDdmServices )
			(m_pDdmServices->*m_pClientCallback)( m_pClientContext, status );
		delete this;
		return status;
	}

	// calculate how much memory we'll need for the table's data.
	m_numBytes = m_cbRow * m_cRows;

	// Allocate PCI visible memory for the table's data
	m_pTableDataPCI = new(tPCI) char[m_numBytes];
	if (!m_pTableDataPCI)
	{
		TRACEF(TRACE_PTS, ("TSReadTable::HandleGetTableDefReply status=ercNoMoreHeap\n"));
		if( m_pDdmServices )
			(m_pDdmServices->*m_pClientCallback)( m_pClientContext, ercNoMoreHeap );
		delete this;
		return ercNoMoreHeap;
	}

	// Allocate an Enumerate Table obhject.
	m_EnumTable = new TSEnumTable;

	// Initialize the enumerate table operation.
	status = m_EnumTable->Initialize( this,
						m_TableName,
						0,					// Starting row number.
						m_pTableDataPCI,	// Returned data buffer.
						m_numBytes,			// max size of returned data.
						&m_cbTableActual,	// pointer to # of returned bytes.
						TSCALLBACK(TSReadTable, HandleEnumTableReply),
						NULL
						);

	if( status != OS_DETAIL_STATUS_SUCCESS )
	{
		TRACEF(TRACE_PTS, ("TSReadTable::HandleGetTableDefReply status=%d\n", status));
		if( m_pDdmServices )
			(m_pDdmServices->*m_pClientCallback)( m_pClientContext, status );
		return status;
	}

	m_EnumTable->Send();

	// (If there was any error delete our allocated buffer.
	if (status != OS_DETAIL_STATUS_SUCCESS)
	{
		TRACEF(TRACE_PTS, ("TSReadTable::HandleGetTableDefReply status=%d\n", status));
		if( m_pDdmServices )
			(m_pDdmServices->*m_pClientCallback)( m_pClientContext, status );
		delete [] m_pTableDataPCI;
		delete this;
		return status;
	}

	// return our status.
	return status;
}


/************************************************************************/
// HandleEnumTableReply - Handles the reply from the EnumTable operation
// initiated in HandleGetTableDefReply().  Deletes this and then calls 
// the user's specified callback with the user specified context.
/************************************************************************/							  
STATUS TSReadTable::HandleEnumTableReply(void *pClientContext, STATUS status)
{
	TRACE_ENTRY(TSReadTable::HandleEnumTableReply);
#ifndef WIN32
	#pragma unused(pClientContext)
#endif

	// (If there was any error delete our allocated buffer.
	if (status != OS_DETAIL_STATUS_SUCCESS)
	{
		TRACEF(TRACE_PTS, ("TSReadTable::HandleGetTableDefReply status=%d\n", status));
		delete [] *m_ppTableDataRet;
		*m_ppTableDataRet = NULL;
	}
	else // if there was no error then copy the dat out of PCI memory.
	{
		// Allocate non-PCI-visible memory for the table's data
		*m_ppTableDataRet = new char[m_numBytes];
		memcpy( *m_ppTableDataRet, m_pTableDataPCI, m_numBytes );

		delete [] m_pTableDataPCI;

      //  return the count of rows read, which we must compute from the
      //  count of bytes read as reported by EnumTable divided by
      //  the count of bytes per row as reported by GetTableDef.  Phew!
      *m_pcRowsRet = m_cbTableActual / m_cbRow;
	};

	// lastly, call the user's callback.
	status = (m_pDdmServices->*m_pClientCallback)( m_pClientContext, status );

	// Delete ourselves
	delete this;

	return status;
}

