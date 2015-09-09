// StopStressDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CacheTest.h"
#include "StopStressDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStopStressDialog dialog


CStopStressDialog::CStopStressDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CStopStressDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStopStressDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CStopStressDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStopStressDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStopStressDialog, CDialog)
	//{{AFX_MSG_MAP(CStopStressDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStopStressDialog message handlers
