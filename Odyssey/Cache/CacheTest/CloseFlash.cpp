// CloseFlash.cpp : implementation file
//

#include "stdafx.h"
#include "CacheTest.h"
#include "CloseFlash.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCloseFlash dialog


CCloseFlash::CCloseFlash(CWnd* pParent /*=NULL*/)
	: CDialog(CCloseFlash::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCloseFlash)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCloseFlash::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCloseFlash)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCloseFlash, CDialog)
	//{{AFX_MSG_MAP(CCloseFlash)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCloseFlash message handlers
