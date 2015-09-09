// ConfigStatsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CacheTest.h"
#include "ConfigStatsDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConfigStatsDialog dialog


CConfigStatsDialog::CConfigStatsDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CConfigStatsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigStatsDialog)
	m_refresh_rate = 0;
	m_cache_number = 0;
	//}}AFX_DATA_INIT
}


void CConfigStatsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigStatsDialog)
	DDX_Text(pDX, IDC_EDIT1, m_refresh_rate);
	DDX_Text(pDX, IDC_EDIT2, m_cache_number);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigStatsDialog, CDialog)
	//{{AFX_MSG_MAP(CConfigStatsDialog)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigStatsDialog message handlers

void CConfigStatsDialog::OnRadio1() 
{
	// TODO: Add your control notification handler code here

	m_display_once = 1;
	//CWnd::GetDlgItem()->SetCheck(BST_CHECKED);
}

void CConfigStatsDialog::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	
	m_display_once = 0;
}
