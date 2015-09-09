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
// $Log: /Gemini/Odyssey/DdmPTS/ReadTable.h $
// 
// 9     7/21/99 7:02p Hdo
// Change the return type of Send() from STATUS to void
// Call the user callback routine when error
// Add error checking
// 
// 8     7/14/99 2:17p Sgavarre
// Add parameter to GetTableDef that specifies the number of field per row
// 
// 7     5/11/99 10:54a Jhatwich
// win32
// 
// 6     4/03/99 6:32p Jlane
// Multiple changes to interface, messaage payloads and message handlers
// to avoid having to add reply payloads as this destroys SGL items.  We
// do this by allocating space for reply data in the msg payload.
// 
// 5     3/19/99 7:02p Ewedel
// Added new data member m_cbTableActual for recording actual buffer size
// returned by EnumTable.  Added vss Log: keyword.
//
// 01/21/99	JFL	Compiled (Except for callback madness) and checked in.  
// 12/28/98	JFL	Created.
//
/*************************************************************************/
#ifndef _ReadTable_h_
#define _ReadTable_h_

#include "Ddm.h"
#include "Table.h"

// The Table Serivce callback is (currently) of the form:
// STATUS Callback(void* pContext, STATUS status).
//  
typedef STATUS (DdmServices::*pTSCallback_t)(void*, STATUS);

class TSReadTable : public DdmServices
{
public:
		// Initialize all parameters necessary to get the Table Def'n.
		STATUS Initialize(	DdmServices*	pDdmServices,
							String64		TableName,			// Name of table to read.
							void*			*ppTableDataRet,	// returned table data.
							U32				*pcRowsRet,			// returned # of rows read
							pTSCallback_t	ClientCallback,		// Client's callback function.
							void*			pClientContext		// Passed to callback function.
						 );
		
		void Send();
		
		// This object's reply handler.
		STATUS HandleGetTableDefReply(void *pClientContext, STATUS status);
		STATUS HandleEnumTableReply(void *pClientContext, STATUS status);
				
private:
	DdmServices*	m_pDdmServices;				// Calling Ddm.
	String64		m_TableName;		// Name of table to read.
	void**			m_ppTableDataRet;	// returned table data.
	U32*			m_pcRowsRet;		// returned # of rows read
	pTSCallback_t	m_pClientCallback;	// Client's callback function.
	void*			m_pClientContext;	// Client's callback param.
	TSGetTableDef*	m_GetTableDef;		// Get Table def'n object.
	TSEnumTable*	m_EnumTable;		// Enumerate Table object.
	U32				m_cbRow;			// Number of bytes per table row.
	U32				m_cRows;			// Number of table rows.
	U32				m_cFieldsRow;		// Number of fields per row.
	U32				m_numBytes;			// calculated size of table data.
	U32				m_cbTableActual;  // actual size returned by EnumTable
	void*			m_pTableDataPCI;	// PCI-visible table buffer.
	
};

#endif // _ReadTable_h_

