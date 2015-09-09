#ifndef __SERIALPORT_H__
#define __SERIALPORT_H__

//#define PWM_COMMWRITE	WM_USER+1	// A break was detected on input.

#define BUFFERSIZE		4096
#define READ_TIMEOUTS	500			//milliseconds

class CPeeknPokeDoc;

class CSerialPort
{														 
public:
	// contruction and destruction
	CSerialPort();
	virtual	~CSerialPort();

	BOOL	InitPort(CPeeknPokeDoc *pOwner, UINT portnr = 2, UINT baud = 57600, char parity = 'N',
					 UINT databits = 8, UINT stopsbits = 1,
					 DWORD dwCommEvents = EV_RXCHAR | EV_CTS, UINT nBufferSize = BUFFERSIZE);

	BOOL	ClosePort(UINT portnr = 2);
	BOOL	WriteABuffer(void *pBuffer, DWORD cSize);
	BOOL	ReadPort(void *pBuffer, signed long *pSize, DWORD signature);
	void	SetDCB(DCB _dcb) { m_DCB = _dcb; }

protected:
	char *FindSignature(char *pData, int cSize, DWORD signature);
	signed long GetCommStatus();
	// handles
	HANDLE	m_hComm;
	CPeeknPokeDoc	*m_pOwner;

	// misc
	UINT		m_nPortNr;
	DCB			m_DCB;
	CWinThread	*ioThread;

	OVERLAPPED	m_ovStatus;
	OVERLAPPED	ovWrite;
	OVERLAPPED	ovRead;
};

#endif __SERIALPORT_H__


