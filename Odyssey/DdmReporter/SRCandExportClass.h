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
// This class is the StorageRollCall and Export Table Reporter class. 
// 
// Update Log: 
//
// $Log: /Gemini/Odyssey/DdmReporter/SRCandExportClass.h $
// 
// 4     12/02/99 2:50p Vnguyen
// Change paramater for rowID structure from pass by value to pass by
// reference (method InitializePHSRecord) to keep the run-time library
// happy.
// 
// 3     8/16/99 1:05p Vnguyen
// Fix verious compiler errors.  Mostly typo and mis-match parameters.
// 
// 2     8/05/99 10:55a Vnguyen
// 
// 1     8/04/99 5:26p Vnguyen -- Fix up parameters between objects.  Make
// sure they match.
// 
// 08/03/99	Vnguyen	Created.
/*************************************************************************/

#ifndef __SRCandExportClass_h
#define __SRCandExportClass_h

#include "CTTypes.h"
#include "Reporter.h"
	
#define TABLE_SIZE	20	// Initial size for PHS DefineTable

typedef struct {
	String64	SRCandExportTableName;		// Name of Src/Export table to retrieve PHS Table RowID
	void		*pSRCandExportRecord;		// A pointer to a buffer for a SRC/Export row record
	U32 		cbSRCandExportRecordMax;	// Maximum size of buffer to receive row data, cbRowDataRetMax
	String64 	SRCandExportvdnName;		// Actual name of vdnDdm field in SRC/Export table
	String64 	SRCandExportPHSRowIDName;	// String64 rgbFieldName

	rowID		*pSRCandExportRowID;
	rowID		*pPHSRowID;

	String64	PHSTableName;		 		// Name of table to send PHS data to
	fieldDef	*aPHSTableFieldDefs;		// FieldDefs for PHS table just in case we need to create it
	U32 		cbPHSTableFieldDefs;		// size of PHS table fieldDefs, U32 cbrgFieldDefs

	void		*pPHSRecord;				// A pointer to a buffer for a PHS row record
	U32 		cbPHSRecord;				// Size of a PHS row record, cbRowData
	U32 		cbPHSRecordMax;				// Maximum size of buffer to receive row data, cbRowDataRetMax

	U32			*pRefreshRate;
	U32			*pSampleRate;
} SRCandExportData;

class SRCandExport: public Reporter
{
public:
		SRCandExport() {};  // Do nothing constructor

	STATUS InitSRCandExport(SRCandExportData *pData, DID didDdm, VDN vdnDdm, REPORTERCODE ReporterCode);
	
	STATUS CheckPHSReply(void* pContext, STATUS status);

	STATUS CreatePHSRecord(void* pContext, STATUS status);

	STATUS UpdateSRCandExportTable(void* pContext, STATUS status);

	STATUS InitBaseClass(void* pContext, STATUS status);

	// These are abstract functions supplied by the derived classes.
	virtual void InitializePHSRecord(void *m_pPHSRecord, rowID *pSRCandExportRowID) = 0;

//private:
	// Member variable declarations:
	SRCandExportData	*m_pData;
	U32					m_cSRCandExportRecord;	// Place holder for the actual # of bytes read
	U32					m_cPHSRecord;			// Place holder for the actual # of bytes read
	rowID				m_PHSRowID;				// rid of the PHS row record
};


// StatusReporter - This is the StatusReporter class declartation.  
// All this class provides is a default constructor that initializaes
// the base class' message codes used to reset and return ddm data.
// In this case it sets them to PHS_RESET_STATUS and PHS_RETURN_STATUS.
// Derived classes should dedrive from this or from PerformanceReporter
// ONLY! 
class StatusSRCandExportReporter : public SRCandExport
{
	StatusSRCandExportReporter();
	
	#if false
	// These are abstract functions supplied by the derived classes.
	virtual STATUS HandleDdmSampleData(Message*	pMsg) = 0;
	virtual STATUS ReturnTableRefreshData(void* &rpTableData, U32 &rcbTableData) = 0;
	#endif
};


// PerformanceReporter - This is the StatusReporter class declartation.  
// All this class provides is a default constructor that initializaes
// the base class' message codes used to reset and return ddm data.
// In this case it sets them to PHS_RESET_PERFORMANCE and PHS_RETURN_PERFORMANCE.
// Derived classes should dedrive from this or from StatusReporter
// ONLY! 
class PerformanceSRCandExportReporter : public SRCandExport
{
	PerformanceSRCandExportReporter();
	
	#if false
	// These are abstract functions supplied by the derived classes.
	virtual STATUS HandleDdmSampleData(Message*	pMsg) = 0;
	virtual STATUS ReturnTableRefreshData(void* &rpTableData, U32 &rcbTableData) = 0;
	#endif
};

#endif	// __SRCandExportClass_h
