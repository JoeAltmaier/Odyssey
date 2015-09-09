/*************************************************************************
*
* This material is a confidential trade secret and proprietary 
* information of ConvergeNet Technologies, Inc. which may not be 
* reproduced, used, sold or transferred to any third party without the 
* prior written consent of ConvergeNet Technologies, Inc.  This material 
* is also copyrighted as an unpublished work under sections 104 and 408 
* of Title 17 of the United States Code.  Law prohibits unauthorized 
* use, copying or reproduction.
*
* Description:
* 
* Update Log: 
* 06/11/99 Dipam Patel: Create file
*
*************************************************************************/

#ifndef __QUEUE_OPERATIONS_H__
#define __QUEUE_OPERATIONS_H__

#pragma pack(4)

#include "CtTypes.h"

#include "assert.h"

#include "DdmOsServices.h"
#include "PtsCommon.h"
#include "ReadTable.h"
#include "Listen.h"
#include "Rows.h"
#include "Table.h"

#include "CommandQueue.h"
#include "StatusQueue.h"


#ifndef WIN32
typedef void* HANDLE;
#endif

typedef void (DdmServices::*pCmdCallback_t)(HANDLE handle, void *pData);
#ifndef INITIALIZECALLBACK
typedef void (DdmServices::*pInitializeCallback_t)(STATUS status);
#endif


#ifdef WIN32
#define CMDCALLBACK(clas,method)	(pCmdCallback_t) method
#elif defined(__ghs__)  // Green Hills
#define CMDCALLBACK(clas,method)	(pCmdCallback_t) &clas::method
#else	// MetroWerks
#define CMDCALLBACK(clas,method)	(pCmdCallback_t)&method
#endif

#ifndef INITIALIZECALLBACK
#ifdef WIN32
#define INITIALIZECALLBACK(clas,method)	(pInitializeCallback_t) method
#elif defined(__ghs__)  // Green Hills
#define INITIAlIZECALLBACK(clas,method)	(pInitializeCallback_t) &clas::method
#else	// MetroWerks
#define INITIALIZECALLBACK(clas,method)	(pInitializeCallback_t)&method
#endif
#endif

class CmdServer : DdmServices {
public:	
//************************************************************************
//	CONSTRUCTOR
//
//		queueName		- name of the queue to be used (Status Que 
//						name generated internally)
//		sizeofCmdData	- Max size for cmd data 
//		sizeofStatusData- Max size for status(event) data (Can be 0)
//		pParentDdm		- parent Ddm pointer
//		cmdInsertionCallback	- address of the handler routine which
//								will be called when any cmd is inserted
//								in the cmd queue. 
//
//	Note:	The queueName, sizeofCmdData and sizeofStatusData should 
//			match the ones used by the corresponding CmdSender object
//************************************************************************
	CmdServer(
		String64			queueName,
		U32					sizeofCmdData,
		U32					sizeofStatusData,
		DdmServices			*pParentDdm,
		pCmdCallback_t		cmdInsertionCallback);
#ifdef VAR_CMD_QUEUE
	CmdServer(
		String64			queueName,
		DdmServices			*pParentDdm,
		pCmdCallback_t		cmdInsertionCallback);
#endif

//************************************************************************
//	DESTRUCTOR
//
//	This object should never be deleted. You should call the csrvTerminate()
//	method which will internally take care of deletion.
//************************************************************************
	~CmdServer();


//************************************************************************
//		csrvInitialize
//
//	Will Define the Command Queue and Status Queues
//	Also a listener will be registered on the CQ for any inserts
//	into the CQ
//
//	objectInitializedCallback	- will be called when the object is 
//								completely initialized. After this any
//								method on the object can be called safely.
//************************************************************************
	STATUS csrvInitialize(pInitializeCallback_t	objectInitializedCallback);




//************************************************************************
//		csrvReportCmdStatus:
//	Functionality:
//		Allows reporting of Command Status
//
//	cmdHandle	- Handle corresponding to cmd for which status is being
//				reported. 
//	statusCode	- opcode for the cmd status
//	pResultData	- any result data associated with the status (can be NULL)
//	pCmdData	- the original cmd data corresponding to status
//
//************************************************************************
	STATUS csrvReportCmdStatus(
		HANDLE				cmdHandle,	
		STATUS				statusCode, 
		void				*pResultData, 
		void				*pCmdData);
#ifdef VAR_CMD_QUEUE
	STATUS csrvReportCmdStatus(
		HANDLE				cmdHandle,	
		STATUS				statusCode, 
		void				*pResultData, 
		U32					cbResultData,
		void				*pCmdData,
		U32					cbCmdData);
#endif

//************************************************************************
//		csrvReportEventStatus:
//	Functionality:
//		Allows reporting of Events
//
//	eventCode	- opcode specifying the event
//	pResultData	- any result data associated with the event 
//					(can be NULL, if sizeofStatusData = 0)
//
//************************************************************************
	STATUS csrvReportEvent(
		STATUS				statusCode, 
		void				*pResultData);
#ifdef VAR_CMD_QUEUE
	STATUS csrvReportEvent(
		STATUS				statusCode, 
		void				*pResultData,
		U32					cbResultData);
#endif

//************************************************************************
//		csrvTerminate
//	Functionality:
//		This routine is mandatory. It should be used instead of
//		delete and delete should never be called.
//
//************************************************************************
	void csrvTerminate();


private:
	U32				m_isInitialized;
#ifdef VAR_CMD_QUEUE
	bool			m_varSizeData;
#endif
	String64		m_CQTableName;
	String64		m_SQTableName;
	U32				m_sizeofCmdData;
	U32				m_sizeofStatusData;

	DdmServices		*m_pDdmServices;
	void			*m_pCallersContext;
	pCmdCallback_t	m_CQInsertRowCallback;


	U32				m_CQListenerType;
	U32				m_CQListenerId;			// to stop listen
	U32*			m_pCQListenTypeRet;
	U32				m_CQSizeOfModifiedRecord;
	void			*m_pCQModifiedRecord;	// for listeners 
	void			*m_pCQTableDataRet;		// table data
	U32				m_sizeofCQTableDataRet;	// size of table data

	U32				m_useTerminate;
	TSListen		*m_pCQListenObject;
	pInitializeCallback_t	m_objectInitializedCallback;

	enum {
		csrvCQ_TABLE_DEFINED_STATE = 1,
		csrvSQ_TABLE_DEFINED_STATE,
		csrvCQ_LISTENER_REGISTERED_STATE,
		csrvSQ_LISTENER_REGISTERED_STATE,
		csrvSQ_ROW_INSERTED_STATE,
		csrvSQ_ROW_DELETED_STATE,
		csrvCQ_ROW_DELETED_STATE
#ifdef VAR_CMD_QUEUE
		,csrvCQ_VL_ROW_READ_STATE
#endif
	};

	typedef struct{
		U32				state;
		U32				rowsDeleted;
		void			*pData;		
		rowID			newRowId;
#ifdef VAR_CMD_QUEUE
		U32				value;
		U32				value1;
#endif
	} CONTEXT;

	typedef struct _OUTSTANDING_CMD{
		rowID				cmdRowId;
		void				*pCmdData;
		_OUTSTANDING_CMD	*pNextCmd;
	} OUTSTANDING_CMD;

	OUTSTANDING_CMD			*pHead;
	OUTSTANDING_CMD			*pTail;


private:

	//************************************************************************
	//		csrvReportStatus:
	//	Functionality:
	//		Allows reporting of events and command status
	//
	//	cmdHandle	- Handle corresponding to cmd for which status is being
	//				reported. If reporting event then cmdHandle=NULL
	//	type		- SQ_COMMAND_STATUS or SQ_EVENT_STATUS
	//	statusCode	- opcode for the status
	//	pResultData	- any result data associated with the status (can be NULL)
	//	pCmdData	- the original cmd data corresponding to status (NULL for events)
	//
	//************************************************************************
	STATUS csrvReportStatus(
		HANDLE				cmdHandle,	// NULL for events
		SQ_STATUS_TYPE		type,		// SQ_COMMAND_STATUS or SQ_EVENT_STATUS
		STATUS				statusCode, 
		void				*pResultData, 
		void				*pCmdData);
#ifdef VAR_CMD_QUEUE
	STATUS csrvReportStatus(
		HANDLE				cmdHandle,	// NULL for events
		SQ_STATUS_TYPE		type,		// SQ_COMMAND_STATUS or SQ_EVENT_STATUS
		STATUS				statusCode, 
		void				*pResultData, 
		U32					cbResultData,
		void				*pCmdData,
		U32					cbCmdData);
#endif

		STATUS csrvReplyHandler(void* _pContext, STATUS status );
		STATUS csrvCQDefineTable();
		STATUS csrvSQDefineTable();
#ifdef VAR_CMD_QUEUE
		STATUS csrvCQDefineVLTable();
		STATUS csrvSQDefineVLTable();
#endif
		STATUS csrvDeleteRow(
					String64	tableName,
					rowID		*pRowId,
					void		*pContext);
		STATUS csrvRegisterListener(
			String64		tableName,
			U32 			listenerType, 
			U32				*pListenerId,
			U32**			ppListenTypeRet,
			void**			ppModifiedRecord,
			U32*			pSizeofModifiedRecord,
			void			*pContext,
			pTSCallback_t	pCallback);


		void csrvAddOutstandingCmd(OUTSTANDING_CMD	*pOutstandingCmd);
		void csrvCheckIfCmdOutstanding(StatusQueueRecord *pSQRecord);
		void csrvGetOutstandingCmd(HANDLE	handle, OUTSTANDING_CMD **ppOutstandingCmd);
		void csrvDeleteOutstandingCmd(OUTSTANDING_CMD *pOutstandingCmd);
		void csrvDeleteAllOutstandingCmds();

};
#endif