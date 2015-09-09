// CacheFileDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CacheTest.h"
#include "CacheFileDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCacheFileDialog

IMPLEMENT_DYNAMIC(CCacheFileDialog, CFileDialog)

CCacheFileDialog::CCacheFileDialog(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
		DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
		CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
}


BEGIN_MESSAGE_MAP(CCacheFileDialog, CFileDialog)
	//{{AFX_MSG_MAP(CCacheFileDialog)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

