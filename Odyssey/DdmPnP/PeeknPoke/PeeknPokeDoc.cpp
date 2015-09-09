// PeeknPokeDoc.cpp : implementation of the CPeeknPokeDoc class
//

#include "stdafx.h"

#include "PeeknPoke.h"
#include "PeeknPokeDoc.h"
//#include "ProgressStatusBar.h"
#include "MainFrm.h"
#include "PeeknPokeView.h"
#include "MsgWndView.h"

#include "SerialPort.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPeeknPokeDoc

IMPLEMENT_DYNCREATE(CPeeknPokeDoc, CDocument)

BEGIN_MESSAGE_MAP(CPeeknPokeDoc, CDocument)
	//{{AFX_MSG_MAP(CPeeknPokeDoc)
	ON_COMMAND(ID_CONFIG_PORT, OnConfigPort)
	ON_COMMAND(ID_CONNECT, OnConnect)
	ON_UPDATE_COMMAND_UI(ID_CONNECT, OnUpdateConnect)
	ON_COMMAND(ID_DISCONNECT, OnDisconnect)
	ON_UPDATE_COMMAND_UI(ID_DISCONNECT, OnUpdateDisconnect)
	ON_COMMAND(ID_COMPILE, OnCompile)
	ON_UPDATE_COMMAND_UI(ID_COMPILE, OnUpdateCompile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPeeknPokeDoc construction/destruction

CPeeknPokeDoc::CPeeknPokeDoc()
{
	m_Connected = FALSE;
	m_File = FALSE;
	iCmdIndex = 0;

	m_ComPort = -1;
	m_Baud = -1;
	m_Data = -1;
	m_Parity = -1;
	m_StopBit = -1;
	POSITION pos = GetFirstViewPosition();
	pPnPView = (CPeeknPokeView *)GetNextView(pos);

	m_Parser.SetDoc( this );
}

CPeeknPokeDoc::~CPeeknPokeDoc()
{
	if(m_Connected == TRUE)
		m_Port.ClosePort(m_ComPort);

	m_LineList.RemoveAll();
	m_ReadBuffer.RemoveAll();
}

void CPeeknPokeDoc::Serialize(CArchive& ar)
{
}

BOOL CPeeknPokeDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CPeeknPokeDoc diagnostics

#ifdef _DEBUG
void CPeeknPokeDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CPeeknPokeDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPeeknPokeDoc commands

void CPeeknPokeDoc::OnConfigPort() 
{
	// TODO: Add your command handler code here
	PortConfig	configDlg;
	DCB dcb;

	// display dialog box
	if( configDlg.DoModal() == IDOK )
	{
		m_ComPort = configDlg.m_ComPort+1;
		char *szPort = new char[50];
		COMMCONFIG	ComSettings;

		m_ComPort = configDlg.m_ComPort+1;
		sprintf(szPort, "COM%d", m_ComPort);
		if( ::CommConfigDialog(szPort, AfxGetMainWnd()->m_hWnd, &ComSettings) )
		{
			m_Port.SetDCB(ComSettings.dcb);
			m_Baud = ComSettings.dcb.BaudRate;
			m_Data = ComSettings.dcb.ByteSize;
			m_Parity = ComSettings.dcb.Parity;
			m_StopBit = ComSettings.dcb.StopBits;
		}
		else
		{
			dcb.BaudRate = m_Baud = 57600;
			dcb.fDtrControl = 3;
			dcb.ByteSize = m_Data = 8;
			dcb.Parity = m_Parity = 0;
			dcb.StopBits = m_StopBit = 0;
			m_Port.SetDCB(dcb);
		}
		delete [] szPort;
	}
	else
	{
		m_ComPort = 2;
		dcb.BaudRate = m_Baud = 57600;
		dcb.fDtrControl = 3;
		dcb.ByteSize = m_Data = 8;
		dcb.Parity = m_Parity = 0;
		dcb.StopBits = m_StopBit = 0;
		m_Port.SetDCB(dcb);
	}
	UpdateAllViews( NULL );
}

BOOL CPeeknPokeDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	// TODO: Add your specialized creation code here
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	
	m_ReadBuffer.RemoveAll();
   	BeginWaitCursor();
	CStdioFile file(lpszPathName, CFile::modeRead | CFile::typeText);

	CString	strLine;

	while (file.ReadString(strLine) != NULL)
		//	Add to the buffer
		m_ReadBuffer.AddTail(strLine+"\n");

	file.Close();
	m_File = TRUE;
	EndWaitCursor();

	return TRUE;
}

BOOL CPeeknPokeDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	// TODO: Add your specialized code here and/or call the base class
	CStdioFile	file(lpszPathName, CFile::modeCreate | CFile::modeWrite | CFile::typeText);

	for(int i=0; i < iCmdIndex; i++)
		file.Write( m_CmdArray[i], m_CmdArray[i].GetLength() );

	file.Flush();
	file.Close();
	return TRUE;
}

void CPeeknPokeDoc::OnConnect() 
{
	if( m_Connected == TRUE )
		return;

	// TODO: Add your command handler code here
	pMWView->DisplayMessage("Connecting...");

	if( m_ComPort < 1 || m_ComPort > 4  )
	{
		m_ComPort = 2;
		m_Baud = 57600;
		m_Parity = 'N';
		m_Data = 8;
		m_StopBit = 1;
	}
	
	if( m_Port.InitPort(this, m_ComPort, m_Baud, m_Parity, m_Data, m_StopBit) )
	{
		if( m_Parser.Connect() )
		{
			m_Connected = TRUE;
			pMWView->DisplayMessage("DONE");
		}
		else
		{
			pMWView->DisplayMessage("The system is not ready.  Check your system/cable!");
			m_Port.ClosePort();
		}
	}
	else
	{
		AfxMessageBox(IDS_CONNECTFAILED);
		pMWView->DisplayMessage("InitPort FAILED");
		m_Port.ClosePort();
	}
	SetModifiedFlag(FALSE);
}

void CPeeknPokeDoc::OnUpdateConnect(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_Connected);
}

void CPeeknPokeDoc::OnDisconnect() 
{
	if( m_Connected == FALSE )
		return;

	// TODO: Add your command handler code here
	pMWView->DisplayMessage("Disconnecting...");

	m_Connected = FALSE;
	m_Port.ClosePort(m_ComPort);

	pMWView->DisplayMessage("DONE");
	SetModifiedFlag(FALSE);
}

void CPeeknPokeDoc::OnUpdateDisconnect(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(!m_Connected);
}

void CPeeknPokeDoc::OnCompile() 
{
	// TODO: Add your command update UI handler code here
	BOOL Error = FALSE;
	int i = 0;

	for( POSITION pos = m_ReadBuffer.GetHeadPosition(); pos != NULL; i++ )
	{
		if( !m_Parser.CompileLine( m_ReadBuffer.GetNext(pos), i) )
			Error = TRUE;
	}

	if( Error )
		return ;
	if( AfxMessageBox(IDS_COMPILE_BOX, MB_ICONQUESTION|MB_OKCANCEL ) == IDOK )
	{
		CString Line;

		#if 0
		CProgressStatusBar *pStatus = CPeeknPokeApp::GetApp()->GetMainFrame()->GetStatusBar();
		if( pStatus )
		{
			Line.LoadString(IDS_RUNNING);
			pStatus->SetProgressLabel(Line);

			CProgressCtrl *pProgress = pStatus->GetProgressCtrl();
			if( pProgress )
			{
				pProgress->SetRange(0, 10);
				pProgress->SetStep( 1 );
				while( pProgress->StepIt() != 10 )
					for( LONG l=0; l < 200000; l++ )
						;
			}
		}
		#endif

		BeginWaitCursor();
		// Run the script
		for( pos = m_ReadBuffer.GetHeadPosition(); pos != NULL; i++ )
		{
			Line = m_ReadBuffer.GetNext(pos);
			m_LineList.AddTail(">" + Line );
			m_Parser.ProcessCommand( Line );
			//Sleep(500);
		}
		EndWaitCursor();
	}
}

void CPeeknPokeDoc::OnUpdateCompile(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_File);
}
