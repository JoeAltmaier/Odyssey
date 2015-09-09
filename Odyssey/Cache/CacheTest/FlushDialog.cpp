// FlushDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CacheTest.h"
#include "FlushDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFlushDialog dialog


CFlushDialog::CFlushDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CFlushDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFlushDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CFlushDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFlushDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFlushDialog, CDialog)
	//{{AFX_MSG_MAP(CFlushDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFlushDialog message handlers
