// PortConfig.cpp : implementation file
//

#include "stdafx.h"
#include "PeeknPoke.h"
#include "PeeknPokeDoc.h"
#include "PortConfig.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
/////////////////////////////////////////////////////////////////////////////
// PortConfig dialog


PortConfig::PortConfig(CWnd* pParent /*=NULL*/)
	: CDialog(PortConfig::IDD, pParent)
{
	//{{AFX_DATA_INIT(PortConfig)
	//m_Baud = _T("");
	//m_ComPort = -1;
	//m_Data = _T("");
	//m_Parity = _T("");
	//m_StopBit = _T("");
	//}}AFX_DATA_INIT
}

void PortConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PortConfig)
	//DDX_CBString(pDX, IDC_BAUD, m_Baud);
	DDX_Radio(pDX, IDC_COM1, m_ComPort);
	//DDX_CBString(pDX, IDC_DATA, m_Data);
	//DDX_CBString(pDX, IDC_PARITY, m_Parity);
	//DDX_CBString(pDX, IDC_STOP_BITS, m_StopBit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(PortConfig, CDialog)
	//{{AFX_MSG_MAP(PortConfig)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PortConfig message handlers

BOOL PortConfig::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here
	//CPeeknPokeDoc* pDoc;//  = GetDocument();
	CSerialPort	m_Port;

	for(int i=0; i < 6; i++)
	{
		UINT IDC_COM;
		switch( i )
		{
			case 0:
				IDC_COM = IDC_COM1;
				break;
			case 1:
				IDC_COM = IDC_COM2;
				break;
			case 2:
				IDC_COM = IDC_COM3;
				break;
			case 3:
				IDC_COM = IDC_COM4;
				break;
			case 4:
				IDC_COM = IDC_COM5;
				break;
			case 5:
				IDC_COM = IDC_COM6;
				break;
		}

		if(m_Port.InitPort( (CPeeknPokeDoc*)((CFrameWnd*)AfxGetApp()->m_pMainWnd)->GetActiveDocument(), i+1) )
		{
			CheckRadioButton(IDC_COM1, IDC_COM6, IDC_COM);
			//strTemp.Format("%d", m_dcb.BaudRate);
			//( (CComboBox*)GetDlgItem(IDC_BAUD) )->SelectString(0, strTemp);
			//strTemp.Format("%d", m_dcb.ByteSize);
			//( (CComboBox*)GetDlgItem(IDC_DATA) )->SelectString(0, strTemp);
			//( (CComboBox*)GetDlgItem(IDC_PARITY) )->SetCurSel(m_dcb.Parity);
			//( (CComboBox*)GetDlgItem(IDC_STOP_BITS) )->SetCurSel(m_dcb.StopBits);
			//( (CComboBox*)GetDlgItem(IDC_FLOW_CONTROL) )->SelectString(0, strTemp);
			m_Port.ClosePort(i+1);
		}
		else
			((CButton*)GetDlgItem(IDC_COM))->EnableWindow( FALSE );
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void PortConfig::OnPaint() 
{
	// TODO: Add your message handler code here
	// Do not call CDialog::OnPaint() for painting messages
}

void PortConfig::OnOK() 
{
	// TODO: Add extra validation here
	switch( GetCheckedRadioButton(IDC_COM1, IDC_COM4) )
	{
		case IDC_COM1:
			m_ComPort = 1;
			break;
		case IDC_COM2:
			m_ComPort = 2;
			break;
		case IDC_COM3:
			m_ComPort = 3;
			break;
		case IDC_COM4:
			m_ComPort = 4;
			break;
		case IDC_COM5:
			m_ComPort = 5;
			break;
		case IDC_COM6:
			m_ComPort = 6;
			break;
	}

	//((CComboBox*)GetDlgItem(IDC_BAUD))->GetWindowText(m_Baud);

	//((CComboBox*)GetDlgItem(IDC_PARITY))->GetWindowText(m_Parity);

	//((CComboBox*)GetDlgItem(IDC_DATA))->GetWindowText(m_Data);

	//((CComboBox*)GetDlgItem(IDC_STOP_BITS))->GetWindowText(m_StopBit);
	
	CDialog::OnOK();
}
