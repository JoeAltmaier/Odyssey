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

#ifndef __CMD_SENDER_H__
#define __CMD_SENDER_H__

#pragma pack(4)

#include "assert.h"
#include "CtTypes.h"

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


typedef void (DdmServices::*pCmdCompletionCallback_t)(
							STATUS			completionCode,
							void			*pResultData,
							void			*pCmdData,
							void			*pCmdContext);

typedef void (DdmServices::*pEventCallback_t)(
							STATUS			eventCode,
							void			*pEventData);

#ifndef INITIALIZECALLBACK
typedef void (DdmServices::*pInitializeCallback_t)(STATUS status);
#ifdef WIN32
#define INITIALIZECALLBACK(clas,method)	(pInitializeCallback_t) method
#elif defined(__ghs__)  // Green Hills
#define INITIAlIZECALLBACK(clas,method)	(pInitializeCallback_t) &clas::method
#else	// MetroWerks
#define INITIALIZECALLBACK(clas,method)	(pInitializeCallback_t)&method
#endif
#endif



#ifdef WIN32
#define CMD_COMPLETION_CALLBACK(clas,method)	(pCmdCompletionCallback_t) method
#elif defined(__ghs__)  // Green Hills
#define CMD_COMPLETION_CALLBACK(clas,method)	(pCmdCompletionCallback_t) &clas::method
#else	// MetroWerks
#define CMD_COMPLETION_CALLBACK(clas,method)	(pCmdCompletionCallback_t)&method
#endif

#ifdef WIN32
#define EVENT_CALLBACK(clas,method)	(pEventCallback_t) method
#elif defined(__ghs__)  // Green Hills
#define EVENT_CALLBACK(clas,method)	(pEventCallback_t) &clas::method
#else	// MetroWerks
#define EVENT_CALLBACK(clas,method)	(pEventCallback_t)&method
#endif



class CmdSender : DdmServices {

public:
//************************************************************************
//	CONSTRUCTOR
//
//		queueName		- name of the queue to be used (Status Que 
//						name generated internally)
//		sizeofCmdData	- Max size for cmd data
//		sizeofStatusData- Max size for status(event) data
//		pParentDdm		- parent Ddm pointer
//
//	Note:	The queueName, sizeofCmdData and sizeofStatusData should 
//			match the ones used by the corresponding CmdServer object
//************************************************************************
	CmdSender(
			String64			queueName,
			U32					sizeofCmdData,
			U32					sizeofStatusData,
			DdmServices			*pParentDdm);

#ifdef VAR_CMD_QUEUE
	CmdSender(
			String64			queueName,
			DdmServices			*pParentDdm);
#endif

//************************************************************************
//	DESTRUCTOR
//
//	This object should never be deleted. You should call the csndrTerminate()
//	method which will internally take care of deletion.
//************************************************************************
	~CmdSender();


//************************************************************************
//		csndrInitialize
//
//	Will attempt to Define the Cmd/Status Queues if not already defined.
//
//	objectInitializedCallback	- will be called when the object is 
//								completely initialized. After this any
//								method on the object can be called safely.
//************************************************************************
	STATUS csndrInitialize(pInitializeCallback_t	objectInitializedCallback);

//************************************************************************
//	csndrExecute
//		Executes any command. The result of the command will be reported
//		in the completionCallback.
//		Note: The Cmd Server object processing the cmd is required to
//		call the csrvReportStatus() method.
//
//	pCmdData			- the cmd data buffer
//	completionCallback	- the Handler which will be called on cmd completion
//	pContext			- any cmd context data
//
//	Return:
//		OK		- cmd submitted successfully
//************************************************************************
	STATUS csndrExecute(
			void						*pCmdData,
			pCmdCompletionCallback_t	completionCallback,
			void						*pContext);

#ifdef VAR_CMD_QUEUE
	STATUS csndrExecute(
			void						*pCmdData,
			U32							cbCmdData,
			pCmdCompletionCallback_t	completionCallback,
			void						*pContext);
#endif
//************************************************************************
//	csndrCheckAndExecute
//		Executes any command only if the cmdData is different from
//		any existing cmd data packet in PTS. This is useful for failover
//		when one Master is talking to another master.
//
//	pCmdData			- the cmd data buffer
//	completionCallback	- the Handler which will be called on cmd completion
//	pContext			- any cmd context data
//
//	Return:
//		OK		- cmd submitted successfully
//************************************************************************
	STATUS csndrCheckAndExecute(
			void						*pCmdData,			// including param data
			pCmdCompletionCallback_t	completionCallback,
			void						*pCmdContext);

//************************************************************************
//	csndrEnumReplyHandler
//		Reply handler for the enum call to get all existing cmds in our CQ
//
//	status				- success/failure
//	pContext			- any cmd context data
//
//	Return:
//		OK		- cmd submitted successfully
//************************************************************************
	STATUS csndrEnumReplyHandler(void* _pContext, STATUS status);


//************************************************************************
//	csndrRegisterForEvents
//		Simply Registers the event callback. Any events in the status queue
//		will be reported by calling the eventCallback.
//		Events are generated by the CmdServer object.
//
//	Return:
//		OK		- on successful registration
//************************************************************************
	STATUS csndrRegisterForEvents(pEventCallback_t	eventCallback);

//************************************************************************
//	csndrTerminate
//
//	Functionality:
//		This routine is mandatory. It should be used instead of
//		delete. DELETE should never be used.
//************************************************************************
	void csndrTerminate();


private:
	U32				m_isInitialized;
#ifdef VAR_CMD_QUEUE
	bool			m_varSizeData;
#endif
	String64		m_CQTableName;
	String64		m_SQTableName;
	U32				m_sizeofCQParams;
	U32				m_sizeofSQParams;
	U32				m_totalStatusDataSize;

	DdmServices		*m_pDdmServices;

	U32				m_SQListenerType;
	U32				m_SQListenerId;			// to stop listen
	U32*			m_pSQListenTypeRet;
	U32				m_SQSizeOfModifiedRecord;
	void			*m_pSQModifiedRecord;	// for listeners to stuff

	U32				m_useTerminate;
	TSListen		*m_pSQListenObject;


	pEventCallback_t		m_eventCallback;	
	pInitializeCallback_t	m_objectInitializedCallback;


	enum {
		csndrCQ_ROW_INSERTED_STATE = 100,
		csndrSQ_ROW_INSERTED_STATE,
		csndrSQ_LISTENER_REGISTERED_STATE,
		csndrCQ_TABLE_DEFINED_STATE,
		csndrSQ_TABLE_DEFINED_STATE
#ifdef VAR_CMD_QUEUE
		, csndrSQ_VL_ROW_READ_STATE
#endif
	};

	typedef struct _CONTEXT{
		U32							state;
		void						*pData;
		void						*pData1;
		U32							value;
		U32							value1;
		rowID						newRowId;
		pCmdCompletionCallback_t	pCallback;
		void						*pCmdContext;
	} CONTEXT;

	typedef struct _OUTSTANDING_CMD{
		rowID						cmdRowId;
		pCmdCompletionCallback_t	completionCallback;
		_OUTSTANDING_CMD			*pNextCmd;
		void						*pCmdContext;
	} OUTSTANDING_CMD;

	OUTSTANDING_CMD			*pHead;
	OUTSTANDING_CMD			*pTail;

private:

	typedef struct TABLE_CONTEXT{
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

		STATUS csndrCQDefineTable();
		STATUS csndrSQDefineTable();
#ifdef VAR_CMD_QUEUE
		STATUS csndrCQDefineVLTable();
		STATUS csndrSQDefineVLTable();
#endif
		STATUS csndrProcessStatus(void *pModifiedSQRecord);
		STATUS csndrRegisterListener(
			String64		tableName,
			U32 			listenerType, 
			U32				*pListenerId,
			U32**			ppListenTypeRet,
			void**			ppModifiedRecord,
			U32*			pSizeofModifiedRecord,
			void			*pContext,
			pTSCallback_t	pCallback);
		STATUS csndrReplyHandler(void* _pContext, STATUS status );
		void csndrAddOutstandingCmd(OUTSTANDING_CMD	*pOutstandingCmd);
		void csndrCheckIfCmdOutstanding(StatusQueueRecord *pSQRecord);
#ifdef VAR_CMD_QUEUE
		void csndrCheckIfCmdOutstanding(SQRecord *pSQRecord);
#endif
		void csndrGetOutstandingCmd(rowID	*pCmdRowId, OUTSTANDING_CMD **ppOutstandingCmd);
		void csndrDeleteOutstandingCmd(OUTSTANDING_CMD *pOutstandingCmd);
		void csndrDeleteAllOutstandingCmds();
		// utility methods
		STATUS utilEnumTable(
			char			*tableName,
			void			**ppTableDataRet,		// where you want data
			U32				*pSizeofTableDataRet,	// where you want size of ret data
			U32				*pNumberOfRows,
			void			*pOriginalContext,
			pTSCallback_t	pCallback);
		STATUS utilEnumerate(
			void			*_pContext,
			STATUS			status);
		STATUS utilEnumReplyHandler(
			void			*_pContext, 
			STATUS			status);


};
#endif