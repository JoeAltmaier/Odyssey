//************************************************************************
// FILE:		ProcessRaidUtility.cpp
//
// PURPOSE:		Implemenets the abstract base class that will be used 
//				to represent RAID utilities running in the O2K 
//				box.
//************************************************************************

#include "ProcessRaidUtility.h"
#include "SsapiResponder.h"
#include "ProcessManager.h"

//************************************************************************
// BuildYourValueSet:
//
// PURPOSE:		The method is responsible for adding all data to be 
//				transfered to a client into its value set. All derived 
//				objects must override this method if they have data members
//				they want client proxies to have. 
//
// NOTE:		If a derived object overrides this this method, it MUST call
//				it in its base class BEFORE doing any work!.
//
// RETURN:		true:		success
//************************************************************************

bool 
ProcessRaidUtility::BuildYourValueSet(){
	
	Process::BuildYourValueSet();

	return true;
}


//************************************************************************
// BuildYourselfFromYourValueSet:
//
// PURPOSE:		Populates data members based on the underlying value set
//
// NOTE:		All subclasses that override this method must call to it
//				somewhere in the overriding method
//
// RETURN:		true:		success
//************************************************************************

bool 
ProcessRaidUtility::BuildYourselfFromYourValueSet(){

	Process::BuildYourselfFromYourValueSet();
	
	return true;
}


//******************************************************************************
// BuildYourselfFromPtsRow:
//
// PURPOSE:		Populates data members based on the PTS row
//******************************************************************************

void 
ProcessRaidUtility::BuildYourselfFromPtsRow( RAID_ARRAY_UTILITY *pRow ){

	m_id				= DesignatorId( pRow->thisRID, (U16)GetClassType() );
	m_priority			= pRow->priority;
	m_state				= pRow->status;
	m_percentComplete	= pRow->percentComplete;

}


//******************************************************************************
// Pause:
//
// PURPOSE:		Attempts to pause the process
//******************************************************************************

bool 
ProcessRaidUtility::Pause( SsapiResponder *pResponder ){

	return true;
}


//******************************************************************************
// Resume:
//
// PURPOSE:		Attempts to resume the process
//******************************************************************************

bool 
ProcessRaidUtility::Resume( SsapiResponder *pResponder ){

	return true;
}


//******************************************************************************
// Abort:
//
// PURPOSE:		Attempts to abort the process
//******************************************************************************

bool 
ProcessRaidUtility::Abort( SsapiResponder *pResponder ){

	RMSTR_CMND_INFO					*pCmdInfo;
	RMSTR_ABORT_UTIL_INFO			*pUtilInfo;
	STATUS							status;

	pCmdInfo = (RMSTR_CMND_INFO *)new char[sizeof(RMSTR_CMND_INFO)];
	memset(pCmdInfo,0,sizeof(RMSTR_CMND_INFO));

	pCmdInfo->opcode = RMSTR_CMND_ABORT_UTIL;
	pUtilInfo = (RMSTR_ABORT_UTIL_INFO *)&pCmdInfo->cmdParams;

	pUtilInfo->utilRowId = m_id.GetRowId().GetRowID();

	status = m_pRaidCommandQ->csndrExecute( pCmdInfo,
											(pCmdCompletionCallback_t)METHOD_ADDRESS(ProcessManager,RaidCommandCompletionReply),
											pResponder);
	if( status == OK ){
		((ProcessManager *)GetManager())->AddOutstandingRequest();
	}
	else{
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED );
	}

	delete pCmdInfo;
	return status == OK? true : false;
}


//******************************************************************************
// ChangePriority:
//
// PURPOSE:		Attempts to change priority of the process
//******************************************************************************

bool 
ProcessRaidUtility::ChangePriority( U32 newPriority, SsapiResponder *pResponder ){

	RMSTR_CMND_INFO					*pCmdInfo;
	RMSTR_CHANGE_PRIORITY_INFO		*pUtilInfo;
	STATUS							status;

	pCmdInfo = (RMSTR_CMND_INFO *)new char[sizeof(RMSTR_CMND_INFO)];
	memset(pCmdInfo,0,sizeof(RMSTR_CMND_INFO));

	pCmdInfo->opcode = RMSTR_CMND_CHANGE_UTIL_PRIORITY;
	pUtilInfo = (RMSTR_CHANGE_PRIORITY_INFO *)&pCmdInfo->cmdParams;

	pUtilInfo->utilRowId = m_id.GetRowId().GetRowID();
	pUtilInfo->newPriority = (RAID_UTIL_PRIORITY)newPriority;


	status = m_pRaidCommandQ->csndrExecute( pCmdInfo,
											(pCmdCompletionCallback_t)METHOD_ADDRESS(ProcessManager,RaidCommandCompletionReply),
											pResponder);
	if( status == OK ){
		((ProcessManager *)GetManager())->AddOutstandingRequest();
	}
	else{
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED );
	}

	delete pCmdInfo;
	return status == OK? true : false;
}


//******************************************************************************
// Start:
//
// PURPOSE:		Common code for Start() method of all utils. OOP "Template Method" 
//				pattern.
//******************************************************************************

bool 
ProcessRaidUtility::Start( ObjectManager *pManager, SsapiResponder *pResponder, RAID_UTIL_NAME utilName ){

	RMSTR_CMND_INFO					*pCmdInfo;
	RMSTR_START_UTIL_INFO			*pUtilInfo;
	RAID_UTIL_POLICIES				policy;
	STATUS							status;

	pCmdInfo = (RMSTR_CMND_INFO *)new char[sizeof(RMSTR_CMND_INFO)];
	memset(pCmdInfo,0,sizeof(RMSTR_CMND_INFO));
	memset(&policy, 0, sizeof(RAID_UTIL_POLICIES));

	pCmdInfo->opcode = RMSTR_CMND_START_UTIL;
	pUtilInfo = (RMSTR_START_UTIL_INFO *)&pCmdInfo->cmdParams;

	pUtilInfo->targetRowId = m_ownerId.GetRowId().GetRowID();
	pUtilInfo->utilityName = utilName;
	pUtilInfo->priority	   = (RAID_UTIL_PRIORITY)m_priority;
	pUtilInfo->updateRate  = 1;
	pUtilInfo->policy	   = policy;

	status = m_pRaidCommandQ->csndrExecute( pCmdInfo,
											(pCmdCompletionCallback_t)METHOD_ADDRESS(ProcessManager,RaidCommandCompletionReply),
											pResponder);
	
	delete pCmdInfo;
	return status == OK? true : false;
}


//************************************************************************
// ModifyObject:
//
// PURPOSE:		Modifes contents of the object
//
// NOTE:		We only allow to change priority
//************************************************************************

bool 
ProcessRaidUtility::ModifyObject( ValueSet &objectValues, SsapiResponder *pResponder ){

	U32			priority;

	if( !objectValues.GetU32( SSAPI_PROCESS_FID_PRIORITY, &priority ) ){
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALIDPARM_EXCEPTION_NO_PRIORITY );
		return true;
	}

	return ChangePriority( priority, pResponder );
}


//************************************************************************
// DeleteObject:
//
// PURPOSE:		Deletes the object from the system
//
// NOTE:		Must be overridden by objects that can be deleted
//************************************************************************

bool 
ProcessRaidUtility::DeleteObject( ValueSet &objectValues, SsapiResponder *pResponder ){

	return Abort( pResponder );
}
