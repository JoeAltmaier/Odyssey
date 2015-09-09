// StressDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CacheTest.h"
#include "StressDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStressDialog dialog


CStressDialog::CStressDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CStressDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStressDialog)
	m_num_threads = 0;
	//}}AFX_DATA_INIT
}


void CStressDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStressDialog)
	DDX_Text(pDX, IDC_EDIT1, m_num_threads);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStressDialog, CDialog)
	//{{AFX_MSG_MAP(CStressDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStressDialog message handlers
