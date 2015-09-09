// CacheTestDoc.cpp : implementation of the CCacheTestDoc class
//

#include "Simple.h"
#include "stdafx.h"
#include "CacheTest.h"

#include "CacheTestDoc.h"
#include "ConfigStatsDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "CmTest.h"

/////////////////////////////////////////////////////////////////////////////
// CCacheTestDoc

IMPLEMENT_DYNCREATE(CCacheTestDoc, CDocument)

BEGIN_MESSAGE_MAP(CCacheTestDoc, CDocument)
	//{{AFX_MSG_MAP(CCacheTestDoc)
	ON_COMMAND(ID_CACHE_STATS, OnCacheStats)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCacheTestDoc construction/destruction

CCacheTestDoc::CCacheTestDoc()
{
	// TODO: add one-time construction code here

}

CCacheTestDoc::~CCacheTestDoc()
{
}

BOOL CCacheTestDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CCacheTestDoc serialization

void CCacheTestDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CCacheTestDoc diagnostics

#ifdef _DEBUG
void CCacheTestDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CCacheTestDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCacheTestDoc commands

void CCacheTestDoc::OnCacheStats() 
{
	// TODO: Add your command handler code here
	
	CConfigStatsDialog cache_stats_dialog;

	
	// Initialize dialog 
	cache_stats_dialog.m_refresh_rate = 5;
	cache_stats_dialog.m_display_once = 1;
	cache_stats_dialog.m_cache_number = 0;

	if (cache_stats_dialog.DoModal() != IDOK)     	// open dialog box
		return;

	// Save the cache number in the document for the timer
	m_cache_number = cache_stats_dialog.m_cache_number;

	// Make the statistics string empty
	m_statistics.Empty();

	STATUS status = Get_Cache_Stats(&m_statistics, cache_stats_dialog.m_cache_number);
	UpdateAllViews(NULL);	

	if (cache_stats_dialog.m_display_once)
		return;

	// Set a timer to refresh the display 
	UINT success = AfxGetMainWnd()->SetTimer(2, cache_stats_dialog.m_refresh_rate * 1000, NULL);
	if (success == 0)
		DWORD erc = GetLastError();
}
