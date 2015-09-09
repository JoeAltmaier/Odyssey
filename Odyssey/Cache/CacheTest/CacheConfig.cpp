// CacheConfig.cpp : implementation file
//

#include "stdafx.h"
#include "CacheTest.h"
#include "CacheConfig.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCacheConfig dialog


CCacheConfig::CCacheConfig(CWnd* pParent /*=NULL*/)
	: CDialog(CCacheConfig::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCacheConfig)
	m_page_size = 0;
	m_num_pages = 0;
	m_num_reserve_pages = 0;
	m_memory_size_to_allocate = 0;
	m_dirty_page_error_threshold = 0;
	m_dirty_page_writeback_threshold = 0;
	m_hash_table_size = 0;
	m_write_back = 1;
	m_write_through = 0;
	//}}AFX_DATA_INIT
}


void CCacheConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCacheConfig)
	DDX_Text(pDX, IDC_EDIT1, m_page_size);
	DDX_Text(pDX, IDC_EDIT2, m_num_pages);
	DDX_Text(pDX, IDC_RESERVE, m_num_reserve_pages);
	DDX_Text(pDX, IDC_EDIT3, m_memory_size_to_allocate);
	DDX_Text(pDX, IDC_MAX_DIRTY, m_dirty_page_error_threshold);
	DDX_Text(pDX, IDC_THRESHOLD, m_dirty_page_writeback_threshold);
	DDX_Text(pDX, IDC_EDIT_HASH, m_hash_table_size);
	DDX_Radio(pDX, IDC_WRITE_BACK, m_write_back);
	DDX_Radio(pDX, IDC_WRITE_THROUGH, m_write_through);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCacheConfig, CDialog)
	//{{AFX_MSG_MAP(CCacheConfig)
	ON_BN_CLICKED(IDC_WRITE_BACK, OnWriteBack)
	ON_BN_CLICKED(IDC_WRITE_THROUGH, OnWriteThrough)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCacheConfig message handlers

void CCacheConfig::OnWriteBack() 
{
	// TODO: Add your control notification handler code here
	//m_write_through = 0;
	//m_write_back = 1;
	((CButton *)GetDlgItem(IDC_WRITE_BACK))->SetCheck(BST_CHECKED);
	((CButton *)GetDlgItem(IDC_WRITE_THROUGH))->SetCheck(BST_UNCHECKED);
	
}

void CCacheConfig::OnWriteThrough() 
{
	// TODO: Add your control notification handler code here
	
	//m_write_through = 1;
	//m_write_back = 0;
	((CButton *)GetDlgItem(IDC_WRITE_BACK))->SetCheck(BST_UNCHECKED);
	((CButton *)GetDlgItem(IDC_WRITE_THROUGH))->SetCheck(BST_CHECKED);
}
