// WriteSequential.cpp : implementation file
//

#include "stdafx.h"
#include "CacheTest.h"
#include "WriteSequential.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWriteSequential dialog


CWriteSequential::CWriteSequential(CWnd* pParent /*=NULL*/)
	: CDialog(CWriteSequential::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWriteSequential)
	m_starting_page = 0;
	m_num_pages = 0;
	//}}AFX_DATA_INIT
}


void CWriteSequential::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWriteSequential)
	DDX_Text(pDX, IDC_EDIT1, m_starting_page);
	DDX_Text(pDX, IDC_EDIT2, m_num_pages);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWriteSequential, CDialog)
	//{{AFX_MSG_MAP(CWriteSequential)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWriteSequential message handlers
