#include "stdafx.h"
#include "PeeknPoke.h"
#include "PeeknPokeDoc.h"
#include "MsgWndView.h"
#include "PeeknPokeView.h"

#include "SerialPort.h"
#include "PnPMsg.h"
//
// Constructor
//

CSerialPort::CSerialPort()
{
	m_hComm = NULL;
	m_ovStatus.hEvent = NULL;
	ovWrite.hEvent = NULL;
	ovRead.hEvent = NULL;
	ioThread = NULL;
}

//
// Destructor
//

CSerialPort::~CSerialPort()
{
	if(m_hComm)
		CloseHandle(m_hComm);
	if(ioThread)
	{
		TerminateThread(ioThread->m_hThread, 0);
		delete ioThread;
	}
	ioThread = NULL;
}

//
// InitPort - Initialize the port. This can be port 1 to 4.
//

BOOL CSerialPort::InitPort(CPeeknPokeDoc *pOwner,
						   UINT  portnr,		// portnumber (1..4)
						   UINT  baud,			// baudrate
						   char  parity,		// parity 
						   UINT  databits,		// databits 
						   UINT  stopbits,		// stopbits 
						   DWORD dwCommEvents,	// EV_RXCHAR, EV_CTS etc
						   UINT  writebuffersize)	// size to the writebuffer
{
	if(portnr < 1 || portnr > 6)
		return FALSE;

	m_nPortNr = portnr;
	m_pOwner = pOwner;

	char szPort[5];

	// prepare port strings
	sprintf(szPort, "COM%d", portnr);

	// get a handle to the port
	m_hComm = CreateFile(szPort,						// communication port string (COMX)
					     GENERIC_READ | GENERIC_WRITE,	// read/write types
					     0,								// comm devices must be opened with exclusive access
					     NULL,							// no security attributes
					     OPEN_EXISTING,					// comm devices must use OPEN_EXISTING
					     FILE_FLAG_OVERLAPPED,			// ASync I/O
					     0);							// template must be 0 for comm devices

	if (m_hComm == INVALID_HANDLE_VALUE)
		return FALSE;

	m_ovStatus.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if(m_ovStatus.hEvent == NULL)
		return FALSE;

    // Create overlapped event
    ovWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (ovWrite.hEvent == NULL)
		return FALSE;

	ovRead.Offset = 0;
	ovRead.OffsetHigh = 0;
    // Create overlapped event
    ovRead.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (ovRead.hEvent == NULL)
		return FALSE;

	char szBaud[50];
	if( baud == -1 )
	{
		sprintf(szBaud,"baud=%d parity=%c data=%d stop=%d", 57600, 0, 8, 1);
		if( !BuildCommDCB(szBaud, &m_DCB) )
			return FALSE;
	}
	SetCommState(m_hComm , &m_DCB);

	SetCommMask( m_hComm, EV_RXCHAR );
	SetupComm( m_hComm, BUFFERSIZE, BUFFERSIZE );
	PurgeComm( m_hComm, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR );

	COMMTIMEOUTS	commtimeouts;
	
	// set the timeout values
	commtimeouts.ReadIntervalTimeout         = 0;
	commtimeouts.ReadTotalTimeoutMultiplier  = 2;
	commtimeouts.ReadTotalTimeoutConstant    = 2000;
	commtimeouts.WriteTotalTimeoutMultiplier = 2;
	commtimeouts.WriteTotalTimeoutConstant   = 0;

	SetCommTimeouts(m_hComm, &commtimeouts);

	// Everything was created ok.  Ready to go!
	return TRUE;
}

// 
// ClosePort - Stop and end all communcations
//

BOOL CSerialPort::ClosePort(UINT portnr)
{
	if (m_hComm == NULL)
		return FALSE;

	CloseHandle(m_hComm);
	CloseHandle(m_ovStatus.hEvent);
	CloseHandle(ovWrite.hEvent);
	CloseHandle(ovRead.hEvent);
	m_hComm = NULL;
	m_ovStatus.hEvent = NULL;
	ovWrite.hEvent = NULL;
	ovRead.hEvent = NULL;

	return TRUE;
}

//
// WriteABuffer - Write a buffer to the port
//

BOOL CSerialPort::WriteABuffer(void *pBuffer, DWORD cSize)
{		
	DWORD dwWritten;
	DWORD dwRes;
	BOOL fRes;

	PurgeComm( m_hComm, PURGE_TXCLEAR );
	ovWrite.Offset = 0;
	ovWrite.OffsetHigh = 0;

	if( !WriteFile(m_hComm, pBuffer, cSize, &dwWritten, &ovWrite) )
	{
		if( dwRes=GetLastError() != ERROR_IO_PENDING )
			// WriteFile failed but isn't delayed, report error and abort
			fRes = FALSE;
		else
		{
			// Write is pending
			dwRes = WaitForSingleObject(ovWrite.hEvent, INFINITE);

			switch(dwRes)
			{
            case WAIT_OBJECT_0:
				if( !GetOverlappedResult(m_hComm, &ovWrite, &dwWritten, FALSE) )
				{
					// Error in communication, report it
					m_pOwner->pMWView->DisplayMessage("WriteABuffer GetOverlappedResult error");
					fRes = FALSE;
				}
				else
				{
					if(dwWritten != cSize)
						fRes = FALSE;
					else
						// Write complete
						fRes = TRUE;
				}
                break;

            default:
				// Error in the WaitForSingleObject
				// This indicates a problem with the OVERLAPPED structure's event handle
				m_pOwner->pMWView->DisplayMessage("WriteABuffer WaitForSingleObject error");
				fRes = FALSE;
                break;
			}
		}
	}
	else
	{
		// WriteFile completed
		if(dwWritten != cSize)
			fRes = FALSE;
		else
			// Write complete
			fRes = TRUE;
	}

    return fRes;
}

//
// GetCommStatus
//
signed long CSerialPort::GetCommStatus()
{
	COMSTAT _comStat;
	DWORD dwRes;
	int timeout = 0;

	for( ClearCommError(m_hComm, &dwRes, &_comStat); timeout < 10 && _comStat.cbInQue == 0; Sleep(500) )
	{
		ClearCommError(m_hComm, &dwRes, &_comStat);
		timeout++;
	}

	if( timeout == 10 )
		m_pOwner->pMWView->DisplayMessage("ReadPort timeout\n");

	return _comStat.cbInQue;
}

//
// FindSignature
//
char *CSerialPort::FindSignature(char *pData, int cSize, DWORD signature)
{
	char *pStart = pData;
	BOOL found = FALSE;

	while( pStart != NULL && !found && pStart <= pData)
		if( pStart = (char *)memchr(pStart, (unsigned char)(signature >> 24), cSize) )
		{
			found = TRUE;
			for( int i=0; i < 3 && found; i++ )
				if( pStart[i+1] != (char)(signature >> (2-i)*8) )
				{
					found = FALSE;
					pStart += i+1;
				}
				else
					found = TRUE;
		}
		else
			found = FALSE;

	return pStart;
}

//
// ReadPort - 
//

BOOL CSerialPort::ReadPort(void *pBuffer, signed long *pSize, DWORD signature)
{
    DWORD cBytesRead;
	BOOL fWaitingOnRead = FALSE;
	char *pTempBuffer = NULL;
	signed long cbInQue = GetCommStatus();

	if( cbInQue == 0 )
		return FALSE;

	if( cbInQue > *pSize )
	{
		cBytesRead = cbInQue;
		pTempBuffer = new char[cBytesRead];
	}
	else
	{
		pTempBuffer = (char *)pBuffer;
		cBytesRead = min(*pSize, cbInQue);
	}

    if(!fWaitingOnRead)
    {
		if( !ReadFile(m_hComm, pTempBuffer, cBytesRead, &cBytesRead, &ovRead) )
		{
			if( GetLastError() != ERROR_IO_PENDING )	// Error in communication, report it
			{
				m_pOwner->pMWView->DisplayMessage("ReadPort ERROR_IO_PENDING\n");
				if( pTempBuffer )
					delete [] pTempBuffer;
				return FALSE;
			}
			else
				fWaitingOnRead = TRUE;
		}
		else	// Read completed
		{
			if( cbInQue > *pSize )
			{
				char *pFieldDef;
				char *pStart = FindSignature(pTempBuffer, cBytesRead, signature);
				if( pStart==NULL && cBytesRead > 0)
				{
					memset(pTempBuffer + cBytesRead, 0, 1);
					//CString str = _T(pTempBuffer);
					m_pOwner->pMWView->DisplayMessage(pTempBuffer);
					PurgeComm( m_hComm, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR );
					//delete [] pTempBuffer;
					//pTempBuffer = NULL;
					return FALSE;
				}

				pStart -= sizeof(REPLY);
				memcpy((char*)pBuffer, pStart, *pSize);

				SP_REPLY_PAYLOAD *pPayload = (SP_REPLY_PAYLOAD *)pBuffer;
				switch( pPayload->cmd )
				{
				case REPLY_GET_DEF:
					pFieldDef = new char[pPayload->Data.gtR.cbFieldDefs];
					memcpy(pFieldDef, pStart+sizeof(SP_REPLY_PAYLOAD), pPayload->Data.gtR.cbFieldDefs);
					pPayload->Data.gtR.pFieldDefsRet = (fieldDef *)pFieldDef;
					break;
				case REPLY_ENUM:
					pPayload->Data.etR.pRowsDataRet = new char[pPayload->Data.etR.cbRowsDataRet];
					memcpy(pPayload->Data.etR.pRowsDataRet, pStart+sizeof(SP_REPLY_PAYLOAD),
							pPayload->Data.etR.cbRowsDataRet);
					break;
				case REPLY_READ_ROW:
					pPayload->Data.rrR.pRowDataRet = new char[pPayload->Data.rrR.cbRowDataRet];
					memcpy(pPayload->Data.rrR.pRowDataRet, pStart+sizeof(SP_REPLY_PAYLOAD),
							pPayload->Data.rrR.cbRowDataRet);
					break;
				}

				// Display the message from Tracef shoot out
				if( pStart > pTempBuffer )
				{
					memset(pStart - 1, 0, 1);
					CString str = _T(pTempBuffer);
					m_pOwner->pMWView->DisplayMessage(str);
				}
			}
			else
			{
				SP_REPLY_PAYLOAD *pPayload = (SP_REPLY_PAYLOAD *)pBuffer;
				switch( pPayload->cmd )
				{
				case REPLY_GET_DEF:
					pPayload->Data.gtR.pFieldDefsRet = NULL;
					break;
				case REPLY_ENUM:
					pPayload->Data.etR.pRowsDataRet = NULL;
					break;
				case REPLY_READ_ROW:
					pPayload->Data.rrR.pRowDataRet = NULL;
					break;
				}
			}

			if( (pTempBuffer != pBuffer) && pTempBuffer)
			{
				delete [] pTempBuffer;
				pTempBuffer = NULL;
			}

			return TRUE;
		}
	}

    if(fWaitingOnRead)
    {
		switch( WaitForSingleObject(ovRead.hEvent, READ_TIMEOUTS) )
        {
            case WAIT_OBJECT_0:
				if( !GetOverlappedResult(m_hComm, &ovRead, (unsigned long*)pSize, FALSE) )	// Error in communication, report it
					m_pOwner->pMWView->DisplayMessage("ReadPort GetOverlappedResult error");
				else
				{
					if( cbInQue > *pSize )
					{
						memcpy((char*)pBuffer, pTempBuffer+cBytesRead-*pSize, *pSize);
						delete [] pTempBuffer;
					}
				}
				fWaitingOnRead = FALSE;
                break;

            case WAIT_TIMEOUT:
                break;

            default:
				// Error in the WaitForSingleObject
				// This indicates a problem with the OVERLAPPED structure's event handle
				m_pOwner->pMWView->DisplayMessage("ReadPort WaitForSingleObject error");
				return FALSE;
        }
	}
    return TRUE;
}
