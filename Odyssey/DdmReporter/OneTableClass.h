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
// This class is the One Table Reporter class. This class handles Reporters
// which only have to collect data from Ddm and update PTS Status/Performance
// tables.  This is different from the SRCandExport class in which the SRC
// Export, and/or LoopDescriptor tables also needs rid updating.
// 
// At this time, this class is used by the EVC (and maybe IOP) Status
// Reporter.
//
// Update Log: 
//
// $Log: /Gemini/Odyssey/DdmReporter/OneTableClass.h $
// 
// 1     8/24/99 8:31a Vnguyen
// Initial check-in.
// 
/*************************************************************************/

#ifndef __OneTableClass_h
#define __OneTableClass_h

#include "CTTypes.h"
#include "Reporter.h"
	
#define TABLE_SIZE	20	// Initial size for PHS DefineTable

typedef struct {

	String64	PHSTableName;		 		// Name of table to send PHS data to
	fieldDef	*aPHSTableFieldDefs;		// FieldDefs for PHS table just in case we need to create it
	U32 		cbPHSTableFieldDefs;		// size of PHS table fieldDefs, U32 cbrgFieldDefs

	void		*pPHSRecord;				// A pointer to a buffer for a PHS row record
	U32 		cbPHSRecord;				// Size of a PHS row record, cbRowData
	U32 		cbPHSRecordMax;				// Maximum size of buffer to receive row data, cbRowDataRetMax

	U32			*pRefreshRate;
	U32			*pSampleRate;
} OneTableData;

class OneTable: public Reporter
{
public:
		OneTable() {};  // Do nothing constructor

	STATUS InitOneTable(OneTableData *pData, DID didDdm, REPORTERCODE ReporterCode);
	
	STATUS ReadPHSRecord(void* pContext, STATUS status);
	
	STATUS ReadPHSRecordReply(void* pContext, STATUS status);
			
	STATUS InitBaseClass(void* pContext, STATUS status);

	// These are abstract functions supplied by the derived classes.
	virtual void InitializePHSRecord(void *m_pPHSRecord) = 0;
	virtual rowID GetPHSrid(void *m_pPHSRecord) = 0;

//private:
	// Member variable declarations:
	OneTableData	*m_pData;
	U32				m_cPHSRecord;			// Place holder for the actual # of bytes read
	rowID			m_PHSRowID;				// rid of the PHS row record
};


// StatusReporter - This is the StatusReporter class declartation.  
// All this class provides is a default constructor that initializaes
// the base class' message codes used to reset and return ddm data.
// In this case it sets them to PHS_RESET_STATUS and PHS_RETURN_STATUS.
// Derived classes should dedrive from this or from PerformanceReporter
// ONLY! 
class StatusOneTableReporter : public OneTable
{
	StatusOneTableReporter();
	
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
class PerformanceOneTableReporter : public OneTable
{
	PerformanceOneTableReporter();
	
	#if false
	// These are abstract functions supplied by the derived classes.
	virtual STATUS HandleDdmSampleData(Message*	pMsg) = 0;
	virtual STATUS ReturnTableRefreshData(void* &rpTableData, U32 &rcbTableData) = 0;
	#endif
};

#endif	// __OneTableClass_h
