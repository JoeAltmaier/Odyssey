/*************************************************************************/
// Copyright (C) ConvergeNet Technologies, 1998 
//
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SerialDdm.cpp -- Serial DDM
// 
// Description:
// 
// $Log: /Gemini/Odyssey/DdmPnP/SerialDdm.cpp $
// 
// 22    10/27/99 6:02p Hdo
// Change the stack size to 10K
// 
// 21    9/07/99 12:14p Jlane
// Use DBGPORT and only init if Eval.
// 
// 20    9/03/99 11:03a Hdo
// Add tUNCACHED to all 'new'
// Check ttyhit before call ttyin to avoid hanging in low level driver
// 
// 19    8/31/99 6:01p Hdo
// Add counterTimer - a simple watchdog counter
//
// Revision History:
//		05/25/99 Huy Do: change from NU_Queue to QueueLi
//						 add BuildMessage
//		05/24/99 Huy Do: change from QueueLi to NU_Queue and remove
//						 CreateTask
//		05/13/99 Huy Do: change CREATE_TABLE_MSG from payload to SGL item
//						 in ProcessPacket
//		05/12/99 Huy Do: modify ReplyHandler
//		05/07/99 Huy Do: add QueueLi
//		05/03/99 Huy Do: implement PortReader (State Table Pattern)
/*************************************************************************/

#define _DEBUG

#define _TRACEF
#define TRACE_INDEX TRACE_SDDM

#include "BuildSys.h"
#include "Trace_Index.h"
#include "Odyssey_Trace.h"

#include "SerialDdm.h"
#include "ErrorLog.h"
#include "PnPMsg.h"

CLASSNAME(SerialDdm, SINGLE);	// Class Link Name used by Buildsys.cpp

// Ctor -- Create ourselves -----------------------------------SerialDdm-
//
// Precondition : 
// Postcondition: 
Ddm *SerialDdm::Ctor(DID did)
{
	TRACE_ENTRY(SerialDdm::Ctor);
	return (new SerialDdm(did));
}

void ReadThread(void* pParam) {
	((SerialDdm*)pParam)->PortReader();
}

// SerialDdm -- Constructor -----------------------------------SerialDdm-
//
// Precondition : 
// Postcondition: 
SerialDdm::SerialDdm(DID did): Ddm(did)
{
	TRACE_ENTRY(SerialDdm::SerialDdm);

	ttyinit(COMPORT, BAUDRATE);
#ifdef INCLUDE_EV64120
	ttyinit(DBGPORT, BAUDRATE);
#endif

	ttyioctl(COMPORT, FIOCINT, TTYRX);
#ifdef INCLUDE_EV64120
//	ttyioctl(DBGPORT, FIOCINT, TTYRX);
#endif

	m_State = IDLE_STATE;

	pStack	= new(tZERO|tUNCACHED) U8[10240];
	Kernel::Create_Thread(ctTask, "Port_Reader_Thread", ReadThread,	this, pStack, 10240);
}

// SerialDdm -- Destructor -----------------------------------SerialDdm-
//
// Precondition : 
// Postcondition: 
SerialDdm::~SerialDdm()
{
	TRACE_ENTRY(SerialDdm::~SerialDdm);

	delete [] pStack;
}

// Initialize -- Just what it says ----------------------------SerialDdm-
//
// Precondition : 
// Postcondition: 
STATUS SerialDdm::Initialize(Message *pMsg)
{
	TRACE_ENTRY(SerialDdm::Initialize);

	Kernel::Schedule_Thread(ctTask);

	Reply(pMsg, OK);

	return OK;
}

// Enable -- Start-it-up --------------------------------------SerialDdm-
//
// Precondition : 
// Postcondition: 
STATUS SerialDdm::Enable(Message *pMsg)	// virtual
{
	TRACE_ENTRY(SerialDdm::Enable);

	Reply(pMsg, OK);

	return OK;
}

// Send data to port
//
// Precondition : 
// Postcondition: 
STATUS SerialDdm::SendToPort(void* pData, U32 uSize)
{
	TRACE_ENTRY(SerialDdm::SendToPort);

	CT_ASSERT(pData != NULL, SendToPort);
	unsigned	i;
	unsigned char	ch, *pB = (unsigned char *)pData;

	// Validate the data
	if(pB != NULL && uSize == 0)
		return OS_DETAIL_STATUS_UNKNOWN_ERROR;

	// TO DO
	// Need to have some type of error detection and error recovery
	// For now, this is enough

	for(i = 0; i < uSize; i++)
	{
		ch = *pB;
		WRITEPORT( *pB );
		pB++;
	}

	return TRUE;
}

// Poll the serial port and signal if a message is received.
// State Table Pattern.  The frame is in HDLC format.
//
// Precondition : a buffer has been allocated for the receiving message
// Postcondition: all of the input was read OR
//				  a complete message was read
void SerialDdm::PortReader()
{
	TRACE_ENTRY(SerialDdm::PortReader);

	char ch;
	unsigned position;
	U32 MAX_SIZE_PACKET = 0;
	U8	timerCounter = 0;

	char pHeader[sizeof(SP_PAYLOAD)];
	CT_ASSERT(pHeader != NULL, PortReader);

	char *pTempBuffer = NULL;

	// poll the serial port to see if any data is available
	// Read port until no characters are available
	while( TRUE )
	{
		if( ttyhit(COMPORT) )
		{
			ch = ttyin(COMPORT);
			switch( m_State )
			{
			case IDLE_STATE:
				position = 0;
				memset(pHeader, 0, sizeof(SP_PAYLOAD));
				// The first four bytes is the CMD
				// The next field is the size of the packet
				pHeader[position++] = ch;
				for(int i=0; i < sizeof(U32)+3; i++)
				{
					if( ttyhit(COMPORT) )
						ch = ttyin(COMPORT);
					else
						for( timerCounter = 0; timerCounter < 10; timerCounter++ )
							if( ttyhit(COMPORT) )
								break;	// Get out of the for timerCounter loop
							else
								NU_Sleep(1);
					if( timerCounter == 10 )
						break;
					pHeader[position++] = ch;
				}
				if( timerCounter == 10 )
					continue;

				// Copy the 5th-8th byte into MAX_SIZE_PACKET
				memcpy(&MAX_SIZE_PACKET, pHeader+4, sizeof(U32));
				MAX_SIZE_PACKET = N_TO_H(MAX_SIZE_PACKET);
				if( MAX_SIZE_PACKET <= 0 || (MAX_SIZE_PACKET > sizeof(SP_PAYLOAD) + 64*sizeof(fieldDef)) )
					continue;

				pTempBuffer = new(tZERO|tUNCACHED) char[MAX_SIZE_PACKET];
				memcpy(pTempBuffer, pHeader, position);

				m_State = IN_FRAME_STATE;
				break;
			case IN_FRAME_STATE:
				// Append char to buffer
				pTempBuffer[position++] = ch;
				if( position < MAX_SIZE_PACKET )
					continue;
				ProcessPacket(pTempBuffer);

				m_State = IDLE_STATE;
				delete [] pTempBuffer;
				pTempBuffer = NULL;
				continue;
			default:
				// Invalid state
				CT_ASSERT(FALSE, PortReader);
				m_State = IDLE_STATE;
			} // end switch
		}
		else
		{
			for( timerCounter = 0; timerCounter < 10; timerCounter++ )
				if( ttyhit(COMPORT) )
					break;
				else
					NU_Sleep(1);
			if( timerCounter == 10 )
			{
				if( m_State == IN_FRAME_STATE && pTempBuffer != NULL )
					delete [] pTempBuffer;
				pTempBuffer = NULL;
				m_State = IDLE_STATE;
			}
		} // end if
	} // end while
}


STATUS SerialDdm::ProcessPacket(char* pDataBlock)
{
	TRACE_ENTRY(SerialDdm::ProcessPacket);

	Message *pMsg = NULL;
	char*	pBuffer;

	//These will be deleted in SerialDdm::DoWork()
	SP_PAYLOAD *_pPayload = new(tZERO|tUNCACHED) SP_PAYLOAD;
	fieldDef* _pFieldDef;

	memcpy(_pPayload, pDataBlock, sizeof(SP_PAYLOAD));

	_pPayload->cmd = (CMD)N_TO_H(_pPayload->cmd);
	_pPayload->cbData = N_TO_H(_pPayload->cbData);

	switch( _pPayload->cmd )
	{
		case CREATE_TABLE:
			pMsg = new Message(PNP_CREATE_TABLE);
			CT_ASSERT(pMsg != NULL, ProcessPacket);

			pBuffer = new(tZERO|tUNCACHED) char[_pPayload->cbData-sizeof(SP_PAYLOAD)];
			memcpy(pBuffer, pDataBlock+sizeof(SP_PAYLOAD), _pPayload->cbData-sizeof(SP_PAYLOAD));
			_pFieldDef = (fieldDef *)pBuffer;
			_pPayload->Data.ct.pFieldDefs = _pFieldDef;

			_pPayload->Data.ct.cbFieldDefs = N_TO_H(_pPayload->Data.ct.cbFieldDefs);
			_pPayload->Data.ct.numFieldDefs = N_TO_H(_pPayload->Data.ct.numFieldDefs);
			_pPayload->Data.ct.cEntriesRsv = N_TO_H(_pPayload->Data.ct.cEntriesRsv);
			_pPayload->Data.ct.persistent = N_TO_H(_pPayload->Data.ct.persistent);
			_pPayload->Data.ct.cRow = N_TO_H(_pPayload->Data.ct.cRow);
			for(int i=0; i < _pPayload->Data.ct.numFieldDefs; i++)
			{
				_pPayload->Data.ct.pFieldDefs[i].cbField = N_TO_H( _pPayload->Data.ct.pFieldDefs[i].cbField );
				_pPayload->Data.ct.pFieldDefs[i].iFieldType = (fieldType)N_TO_H( _pPayload->Data.ct.pFieldDefs[i].iFieldType );
				_pPayload->Data.ct.pFieldDefs[i].persistFlags = N_TO_H( _pPayload->Data.ct.pFieldDefs[i].persistFlags );
			}

			// Add data into the CreateTable' SGL
			pMsg->AddSgl(CREATE_TABLE_MSG_SGL,
						_pPayload, _pPayload->cbData, SGL_SEND);
			pMsg->AddSgl(CREATE_TABLE_REPLY_SGL,
						NULL, sizeof(rowID), SGL_REPLY);
			break;

		case GET_TABLE_DEF:
			pMsg = new Message(PNP_GET_TABLE_DEF);
			CT_ASSERT(pMsg != NULL, ProcessPacket);

			_pPayload->Data.gt.FieldDefRetMax = N_TO_H(_pPayload->Data.gt.FieldDefRetMax);
			// Add data into the GetTableDef' SGL
			pMsg->AddSgl(GET_TABLE_DEF_MSG_SGL,
						_pPayload, _pPayload->cbData, SGL_SEND);
			pMsg->AddSgl(GET_TABLE_DEF_REPLY_SGL,
						NULL, sizeof(GET_TABLE_DEF_REPLY_PAYLOAD), SGL_REPLY);
			break;

		case ENUM_TABLE:
			pMsg = new Message(PNP_ENUM_TABLE);
			CT_ASSERT(pMsg != NULL, ProcessPacket);

			_pPayload->Data.et.startRow = N_TO_H(_pPayload->Data.et.startRow);
			_pPayload->Data.et.cbDataRetMax = N_TO_H(_pPayload->Data.et.cbDataRetMax);

			// Add data into the EnumTable' SGL
			pMsg->AddSgl(ENUM_TABLE_MSG_SGL,
						_pPayload, _pPayload->cbData);
			pMsg->AddSgl(GET_TABLE_DEF_REPLY_SGL,
						NULL, sizeof(GET_TABLE_DEF_REPLY_PAYLOAD), SGL_REPLY);
			break;

		case INSERT_ROW:
			pMsg = new Message(PNP_INSERT_ROW);
			CT_ASSERT(pMsg != NULL, ProcessPacket);

			pBuffer = new(tZERO|tUNCACHED) char[_pPayload->cbData-sizeof(SP_PAYLOAD)];
			memcpy(pBuffer, pDataBlock+sizeof(SP_PAYLOAD), _pPayload->cbData-sizeof(SP_PAYLOAD));
			_pPayload->Data.in.pRowData = pBuffer;
			_pPayload->Data.in.cbRowData = N_TO_H(_pPayload->Data.in.cbRowData)+sizeof(rowID);

			// Add data into the InsertRow' SGL
			pMsg->AddSgl(INSERT_ROW_MSG_SGL,
						 _pPayload, _pPayload->cbData);
			pMsg->AddSgl(CREATE_TABLE_REPLY_SGL,
						NULL, sizeof(rowID), SGL_REPLY);
			break;

		case DELETE_TABLE:
			pMsg = new Message(PNP_DELETE_TABLE);
			CT_ASSERT(pMsg != NULL, ProcessPacket);

			// Add data into the DeleteTable' SGL
			pMsg->AddSgl(DELETE_TABLE_MSG_SGL,
						_pPayload, _pPayload->cbData);
			break;

		case DELETE_ROW:
			pMsg = new Message(PNP_DELETE_ROW);
			CT_ASSERT(pMsg != NULL, ProcessPacket);

			pBuffer = new(tZERO|tUNCACHED) char[_pPayload->cbData-sizeof(SP_PAYLOAD)];
			memcpy(pBuffer, pDataBlock+sizeof(SP_PAYLOAD), _pPayload->cbData-sizeof(SP_PAYLOAD));
			_pPayload->Data.dr.pKeyFieldValue = pBuffer;
			_pPayload->Data.dr.cbKeyFieldValue = N_TO_H(_pPayload->Data.dr.cbKeyFieldValue);

			// Add data into the DeleteTable' SGL
			pMsg->AddSgl(DELETE_ROW_MSG_SGL,
						_pPayload, _pPayload->cbData);
			pMsg->AddSgl(DELETE_ROW_REPLY_SGL,
						NULL, sizeof(U32), SGL_REPLY);
			break;

		case READ_ROW:
			pMsg = new Message(PNP_READ_ROW);
			CT_ASSERT(pMsg != NULL, ProcessPacket);

			pBuffer = new(tZERO|tUNCACHED) char[_pPayload->cbData-sizeof(SP_PAYLOAD)];
			memcpy(pBuffer, pDataBlock+sizeof(SP_PAYLOAD), _pPayload->cbData-sizeof(SP_PAYLOAD));
			_pPayload->Data.rr.pKeyFieldValue = pBuffer;
			_pPayload->Data.rr.cbKeyFieldValue = N_TO_H(_pPayload->Data.rr.cbKeyFieldValue);
			_pPayload->Data.rr.cbRowRetMax = N_TO_H(_pPayload->Data.rr.cbRowRetMax);

			// Add data into the DeleteTable' SGL
			pMsg->AddSgl(READ_ROW_MSG_SGL,
						_pPayload, _pPayload->cbData);
			pMsg->AddSgl(READ_ROW_REPLY_SGL,
						NULL, sizeof(READ_ROW_REPLY_PAYLOAD)
						, SGL_REPLY);
			break;

		case MODIFY_ROW:
			pMsg = new Message(PNP_MODIFY_ROW);
			CT_ASSERT(pMsg != NULL, ProcessPacket);

			_pPayload->Data.mr.cbKeyFieldValue = N_TO_H(_pPayload->Data.mr.cbKeyFieldValue);
			_pPayload->Data.mr.cbRowData = N_TO_H(_pPayload->Data.mr.cbRowData);

			pBuffer = new(tZERO|tUNCACHED) char[_pPayload->cbData-sizeof(SP_PAYLOAD)];
			memcpy(pBuffer, pDataBlock+sizeof(SP_PAYLOAD), _pPayload->cbData-sizeof(SP_PAYLOAD));
			_pPayload->Data.mr.pKeyFieldValue = pBuffer;
			_pPayload->Data.mr.pRowData = pBuffer + _pPayload->Data.mr.cbKeyFieldValue;

			// Add data into the DeleteTable' SGL
			pMsg->AddSgl(MODIFY_ROW_MSG_SGL,
						_pPayload, _pPayload->cbData);
			pMsg->AddSgl(MODIFY_ROW_REPLY_SGL,
						NULL, sizeof(U32), SGL_REPLY);
			break;

		case MODIFY_FIELD:
			pMsg = new Message(PNP_MODIFY_FIELD);
			CT_ASSERT(pMsg != NULL, ProcessPacket);

			_pPayload->Data.mf.cbKeyFieldValue = N_TO_H(_pPayload->Data.mf.cbKeyFieldValue);
			_pPayload->Data.mf.cbFieldValue = N_TO_H(_pPayload->Data.mf.cbFieldValue);

			pBuffer = new(tZERO|tUNCACHED) char[_pPayload->cbData-sizeof(SP_PAYLOAD)];
			memcpy(pBuffer, pDataBlock+sizeof(SP_PAYLOAD), _pPayload->cbData-sizeof(SP_PAYLOAD));
			_pPayload->Data.mf.pKeyFieldValue = pBuffer;
			_pPayload->Data.mf.pFieldValue = pBuffer + _pPayload->Data.mr.cbKeyFieldValue;

			// Add data into the DeleteTable' SGL
			pMsg->AddSgl(MODIFY_FIELD_MSG_SGL,
						_pPayload, _pPayload->cbData);
			pMsg->AddSgl(MODIFY_FIELD_REPLY_SGL,
						NULL, sizeof(U32), SGL_REPLY);
			break;

		case LISTEN:
			pMsg = new Message(PNP_LISTEN);
			CT_ASSERT(pMsg != NULL, ProcessPacket);

			break;

		case CONNECT:
			pMsg = new Message(PNP_CONNECT);
			CT_ASSERT(pMsg != NULL, ProcessPacket);

			// Add data into the Connect' SGL
			pMsg->AddSgl(CONNECT_MSG_SGL,
						_pPayload, _pPayload->cbData);
			break;
	}

	// remove packet from the queue
	//InBufferQ.Dequeue();

	// then send it to the PnPDdm
	Send(pMsg);

	return OK;
}

// DoWork
//
// Precondition : 
// Postcondition: 
STATUS SerialDdm::DoWork(Message *pMsg)	// virtual
{
	TRACE_ENTRY(SerialDdm::DoWork);

	SP_PAYLOAD *pInitPayload = NULL;
	SP_REPLY_PAYLOAD replyPayload;
	U32	cbPayload, cbInitData;
	U32 *pcRowDel, *pcRowModified;
	char* pData = NULL;

	GET_TABLE_DEF_REPLY_PAYLOAD *pRetGetDefPayload = NULL;
	ENUM_TABLE_REPLY_PAYLOAD	*pRetEnumPayload = NULL;
	READ_ROW_REPLY_PAYLOAD		*pRetReadRow = NULL;
	STATUS status = pMsg->DetailedStatusCode;

	pMsg->GetSgl(0, &pInitPayload, &cbInitData);
	replyPayload.chID = pInitPayload->chID;
	replyPayload.ErrorCode = H_TO_N(status);
	
	if (pMsg->IsReply())
	{
		switch( pMsg->reqCode )
		{
		case PNP_CREATE_TABLE:
			pMsg->GetSgl(CREATE_TABLE_REPLY_SGL, &m_pReturnedRID, &cbPayload);

			cbPayload = sizeof(SP_REPLY_PAYLOAD);
			pData = new(tZERO|tUNCACHED) char[cbPayload];

			replyPayload.cmd = (REPLY)H_TO_N(REPLY_CREATE);
			replyPayload.cbData = H_TO_N(cbPayload);

			replyPayload.Data.ctR.Table = H_TO_NS(m_pReturnedRID->Table);
			replyPayload.Data.ctR.HiPart = H_TO_NS(m_pReturnedRID->HiPart);
			replyPayload.Data.ctR.LoPart = H_TO_N(m_pReturnedRID->LoPart);

			memmove(pData, &replyPayload, sizeof(SP_REPLY_PAYLOAD));

			delete [] pInitPayload->Data.ct.pFieldDefs;
			break;

		case PNP_GET_TABLE_DEF:
			pMsg->GetSgl(GET_TABLE_DEF_REPLY_SGL, &pRetGetDefPayload, &cbPayload);

			cbPayload = sizeof(SP_REPLY_PAYLOAD) + pRetGetDefPayload->cbFieldDefs;

			replyPayload.cmd = (REPLY)H_TO_N(REPLY_GET_DEF);
			replyPayload.cbData = H_TO_N(cbPayload);
			pData = new(tZERO|tUNCACHED) char[cbPayload];

			if( status == ercOK )
			{
				for(int i=0; i < pRetGetDefPayload->numFieldDefsRet; i++)
				{
					pRetGetDefPayload->pFieldDefsRet[i].cbField =
						H_TO_N(pRetGetDefPayload->pFieldDefsRet[i].cbField);
					pRetGetDefPayload->pFieldDefsRet[i].iFieldType = 
						(fieldType)H_TO_N(pRetGetDefPayload->pFieldDefsRet[i].iFieldType);
					pRetGetDefPayload->pFieldDefsRet[i].persistFlags = 
						H_TO_N(pRetGetDefPayload->pFieldDefsRet[i].persistFlags);
				}
				memmove(pData+sizeof(SP_REPLY_PAYLOAD), pRetGetDefPayload->pFieldDefsRet,
					 	pRetGetDefPayload->cbFieldDefs);
			}
			else
				replyPayload.Data.gtR.pFieldDefsRet = NULL;
			delete [] pRetGetDefPayload->pFieldDefsRet;

			replyPayload.Data.gtR.cbFieldDefs = H_TO_N(pRetGetDefPayload->cbFieldDefs);
			replyPayload.Data.gtR.numFieldDefsRet = H_TO_N(pRetGetDefPayload->numFieldDefsRet);
			replyPayload.Data.gtR.cbRowRet = H_TO_N(pRetGetDefPayload->cbRowRet);
			replyPayload.Data.gtR.cRowsRet = H_TO_N(pRetGetDefPayload->cRowsRet);

			memmove(pData, &replyPayload, sizeof(SP_REPLY_PAYLOAD));
			break;

		case PNP_ENUM_TABLE:
			pMsg->GetSgl(ENUM_TABLE_MSG_REPLY_SGL, &pRetEnumPayload, &cbPayload);

			cbPayload = sizeof(SP_REPLY_PAYLOAD) + pRetEnumPayload->cbRowsDataRet;

			replyPayload.cmd = (REPLY)H_TO_N(REPLY_ENUM);
			replyPayload.cbData = H_TO_N(cbPayload);
			pData = new(tZERO|tUNCACHED) char[cbPayload];

			if( status == ercOK )
				// I didn't convert the data in pRowDataRet
				memmove(pData+sizeof(SP_REPLY_PAYLOAD), pRetEnumPayload->pRowsDataRet,
						 pRetEnumPayload->cbRowsDataRet);
			else
				replyPayload.Data.etR.pRowsDataRet = NULL;
			delete pRetEnumPayload->pRowsDataRet;

			replyPayload.Data.etR.cbRowsDataRet = H_TO_N(pRetEnumPayload->cbRowsDataRet);

			memmove(pData, &replyPayload, sizeof(SP_REPLY_PAYLOAD));

			break;

		case PNP_INSERT_ROW:
			pMsg->GetSgl(INSERT_ROW_REPLY_SGL, &m_pReturnedRID, &cbPayload);

			cbPayload = sizeof(SP_REPLY_PAYLOAD);
			pData = new(tZERO|tUNCACHED) char[cbPayload];

			replyPayload.cmd = (REPLY)H_TO_N(REPLY_INSERT);
			replyPayload.cbData = H_TO_N(cbPayload);

			replyPayload.Data.inR.Table = H_TO_NS(m_pReturnedRID->Table);
			replyPayload.Data.inR.HiPart = H_TO_NS(m_pReturnedRID->HiPart);
			replyPayload.Data.inR.LoPart = H_TO_N(m_pReturnedRID->LoPart);
			memmove(pData, &replyPayload, cbPayload);

			delete [] pInitPayload->Data.in.pRowData;
			break;

		case PNP_DELETE_TABLE:
			pMsg->GetSgl(DELETE_TABLE_REPLY_SGL, &m_pReturnedRID, &cbPayload);

			cbPayload = sizeof(SP_REPLY_PAYLOAD);
			pData = new(tZERO|tUNCACHED) char[cbPayload];

			replyPayload.cmd = (REPLY)H_TO_N(REPLY_DELETE_TABLE);
			replyPayload.cbData = H_TO_N(cbPayload);

			memmove(pData, &replyPayload, sizeof(SP_REPLY_PAYLOAD));
			break;

		case PNP_DELETE_ROW:
			pMsg->GetSgl(DELETE_ROW_REPLY_SGL, &pcRowDel, &cbPayload);

			cbPayload = sizeof(SP_REPLY_PAYLOAD);
			pData = new(tZERO|tUNCACHED) char[cbPayload];

			replyPayload.cmd = (REPLY)H_TO_N(REPLY_DELETE_ROW);
			replyPayload.cbData = H_TO_N(cbPayload);

			replyPayload.Data.drR = H_TO_N(*pcRowDel);

			memmove(pData, &replyPayload, sizeof(SP_REPLY_PAYLOAD));

			delete [] pInitPayload->Data.dr.pKeyFieldValue;
			break;

		case PNP_READ_ROW:
			pMsg->GetSgl(READ_ROW_REPLY_SGL, &pRetReadRow, &cbPayload);

			pRetReadRow->cbRowDataRet = pInitPayload->Data.rr.cbRowRetMax;
			cbPayload = sizeof(SP_REPLY_PAYLOAD) + pRetReadRow->cbRowDataRet;
			pData = new(tZERO|tUNCACHED) char[cbPayload];

			replyPayload.cmd = (REPLY)H_TO_N(REPLY_READ_ROW);
			replyPayload.cbData = H_TO_N(cbPayload);

			if( status == ercOK )
				// I didn't convert the data in pDataRet
				memmove(pData+sizeof(SP_REPLY_PAYLOAD), pRetReadRow->pRowDataRet,
						 pRetReadRow->cbRowDataRet);
			else
				replyPayload.Data.rrR.pRowDataRet = NULL;
			delete pRetReadRow->pRowDataRet;

			replyPayload.Data.rrR.cbRowDataRet = H_TO_N(pRetReadRow->cbRowDataRet);
			replyPayload.Data.rrR.numRowsRet = H_TO_N(pRetReadRow->numRowsRet);

			memmove(pData, &replyPayload, sizeof(SP_REPLY_PAYLOAD));
			delete [] pInitPayload->Data.rr.pKeyFieldValue;
			break;

		case PNP_MODIFY_ROW:
			pMsg->GetSgl(MODIFY_ROW_REPLY_SGL, &pcRowModified, &cbPayload);

			cbPayload = sizeof(SP_REPLY_PAYLOAD);
			pData = new(tZERO|tUNCACHED) char[cbPayload];

			replyPayload.cmd = (REPLY)H_TO_N(REPLY_MODIFY_ROW);
			replyPayload.cbData = H_TO_N(cbPayload);

			replyPayload.Data.mrR = H_TO_N(*pcRowModified);
			//replyPayload.Data.mrR.Table = H_TO_NS(m_pReturnedRID->Table);
			//replyPayload.Data.mrR.HiPart = H_TO_NS(m_pReturnedRID->HiPart);
			//replyPayload.Data.mrR.LoPart = H_TO_N(m_pReturnedRID->LoPart);

			memmove(pData, &replyPayload, sizeof(SP_REPLY_PAYLOAD));
			delete [] pInitPayload->Data.mr.pKeyFieldValue;
			break;

		case PNP_MODIFY_FIELD:
			pMsg->GetSgl(MODIFY_FIELD_REPLY_SGL, &pcRowModified, &cbPayload);

			cbPayload = sizeof(SP_REPLY_PAYLOAD);
			pData = new(tZERO|tUNCACHED) char[cbPayload];

			replyPayload.cmd = (REPLY)H_TO_N(REPLY_MODIFY_FIELD);
			replyPayload.cbData = H_TO_N(cbPayload);

			replyPayload.Data.mfR = H_TO_N(*pcRowModified);
			//replyPayload.Data.mfR.Table = H_TO_NS(m_pReturnedRID->Table);
			//replyPayload.Data.mfR.HiPart = H_TO_NS(m_pReturnedRID->HiPart);
			//replyPayload.Data.mfR.LoPart = H_TO_N(m_pReturnedRID->LoPart);

			memmove(pData, &replyPayload, cbPayload);
			delete [] pInitPayload->Data.mf.pKeyFieldValue;
			break;

		case PNP_LISTEN:
			pMsg->GetSgl(LISTEN_REPLY_SGL, &m_pReturnedRID, &cbPayload);

			cbPayload = sizeof(SP_REPLY_PAYLOAD);
			pData = new(tZERO|tUNCACHED) char[cbPayload];

			replyPayload.cmd = (REPLY)H_TO_N(REPLY_LISTEN);
			replyPayload.cbData = H_TO_N(cbPayload);

			memmove(pData, &replyPayload, sizeof(SP_REPLY_PAYLOAD));
			break;

		case PNP_CONNECT:
			cbPayload = sizeof(SP_REPLY_PAYLOAD);
			pData = new(tZERO|tUNCACHED) char[cbPayload];

			replyPayload.cmd = (REPLY)H_TO_N(REPLY_CONNECT);
			replyPayload.cbData = H_TO_N(cbPayload);

			memmove(pData, &replyPayload, sizeof(SP_REPLY_PAYLOAD));
			break;

		default:
			return OS_DETAIL_STATUS_INAPPROPRIATE_FUNCTION;
		}
	}

	SendToPort(pData, cbPayload);

	delete pInitPayload;
	delete [] pData;
	return OK;
}

// End of file