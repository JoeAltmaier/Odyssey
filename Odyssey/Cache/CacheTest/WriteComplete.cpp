// WriteComplete.cpp : implementation file
//

#include "stdafx.h"
#include "CacheTest.h"
#include "WriteComplete.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWriteComplete dialog


CWriteComplete::CWriteComplete(CWnd* pParent /*=NULL*/)
	: CDialog(CWriteComplete::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWriteComplete)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CWriteComplete::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWriteComplete)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWriteComplete, CDialog)
	//{{AFX_MSG_MAP(CWriteComplete)
		// NOTE: the ClassWizard will add message map macros here
    ON_MESSAGE( WM_WRITE_COMPLETE, OnWriteComplete )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWriteComplete message handlers
