/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: BsaIsm.h
//
// Description:
// This class is the BSA ISM used to Translate BSA messages to the Initiator. 
// 
// Update Log:
//	$Log: /Gemini/Odyssey/IsmBsa/BsaIsm.h $
// 
// 9     10/20/99 6:51p Mpanas
// Fix memory leak, reorganize to use
// RequestDefault() and ReplyDefault()
// instead of DoWork()
// 
// 8     8/24/99 3:25p Vnguyen
// Add code to start and stop PHS Reporter in Enable() and
// Quiesce().  Add code to support 
// PHS_RETURN_RESET_PERFORMANCE request.
// 
// 7     5/10/99 3:11p Mpanas
// first cut of the Reporter changes
// 
// 6     4/26/99 12:25p Mpanas
// Changes to support Block address instead of Byte addess
// Include IDLUN in the SCSI Payload instead of signature
// Move all methods into base class
// 
// 5     3/18/99 3:22p Mpanas
// Move config data struct to BsaConfig.h
//
// 10/15/98 Michael G. Panas: Create file
// 02/12/99 Michael G. Panas: convert to new Oos model
/*************************************************************************/

#ifndef __BsaIsm_h
#define __BsaIsm_h

#include "Nucleus.h"
#include "Ddm.h"
#include "BsaConfig.h"
#include "BsaStatus.h"
#include "BsaPerformance.h"

typedef enum {
	BSA_ACTION_FINISH					= 1,
	BSA_ACTION_TEST_UNIT				= 2
} BSA_ACTION;

// BSA ISM internal context
typedef struct _BSA_Context {
	BSA_ACTION		action;
	void			*buffer;			// pointer to buffer (if used)
	Message			*pMsg;				// pointer to original message
	Message			*p_sMsg;			// pointer to message for this drive
} BSA_CONTEXT;


class BsaIsm: public Ddm {
public:

	BSA_CONFIG config;
	
	VDN		myVd;
	
	DID		myDid;
	bool	fStatusReporterActive;		// True if the Status Reporter has been started for this Did
	bool	fPerformanceReporterActive; // True if the Performance Reporter has been started for this Did
										// The reason we need these two flags is because we start the
										// reporters in enable() and stop them in quiesce().  At this
										// time I am not sure if there is a matching one to one calling
										// sequence between these two methods.  An alternative
										// is to start them in the Initialize method, and stop them
										// in the destructor method.
	
	BsaIsm(DID did);

	static
	Ddm *Ctor(DID did);

	STATUS Initialize(Message *pMsg);
	STATUS Enable(Message *pMsg);
	STATUS Quiesce(Message *pMsg);
	
	// BSA Methods
	void	BSA_Build_SCSI_RW_Message(Message *p_sMsg, SCSI_COMMANDS Cmd,
							Message *pMsg);
	void	BSA_Build_SCSI_Message(Message *pMsg, SCSI_COMMANDS Cmd,
							void *pData, long length);
	STATUS	BSA_Send_Message(BSA_CONTEXT *p_context);

protected:

	STATUS RequestDefault(Message *pMsg);
	STATUS ReplyDefault(Message *pMsg);
	
	// Bsa Status Record
	BSA_ISMStatus		m_Status;
	
	// Bsa Performance Record
	BSA_ISMPerformance	m_Perf;
	
};

#endif