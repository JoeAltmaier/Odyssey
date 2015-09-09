// ReadSequential.cpp : implementation file
//

#include "stdafx.h"
#include "CacheTest.h"
#include "ReadSequential.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CReadSequential dialog


CReadSequential::CReadSequential(CWnd* pParent /*=NULL*/)
	: CDialog(CReadSequential::IDD, pParent)
{
	//{{AFX_DATA_INIT(CReadSequential)
	m_starting_page = 0;
	m_num_pages = 0;
	//}}AFX_DATA_INIT
}


void CReadSequential::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CReadSequential)
	DDX_Text(pDX, IDC_EDIT1, m_starting_page);
	DDX_Text(pDX, IDC_EDIT2, m_num_pages);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CReadSequential, CDialog)
	//{{AFX_MSG_MAP(CReadSequential)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReadSequential message handlers
