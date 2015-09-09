// MainFrm.cpp : implementation of the CMainFrame class
//

#include "Simple.h"
#include "stdafx.h"
#include "CacheTest.h"

// Include definitions for classes I create with the wizard.
#include "MainFrm.h"
#include "CacheConfig.h"
#include "WriteSequential.h"
#include "ReadSequential.h"
#include "FlushDialog.h"
#include "CloseFlash.h"
#include "ConfigStatsDialog.h"
#include "StressDialog.h"
#include "StopStressDialog.h"
#include "TraceMon.h"
#include "CacheTestDoc.h"
#include <stdio.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

#endif

#define CmData
#include "CacheTestData.h"

#include "CmTest.h"

// Define message for write complete
#define WM_WRITE_COMPLETE (WM_USER + 1)
#define WM_READ_COMPLETE (WM_USER + 2)
#define WM_FLUSH_COMPLETE (WM_USER + 3)
#define WM_CLOSE_COMPLETE (WM_USER + 4)
#define WM_STRESS_COMPLETE (WM_USER + 5)


/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_FLASH_CONFIG, OnFlashConfig)
	ON_COMMAND(ID_FLASH_TESTCONFIG, OnFlashTestconfig)
	ON_COMMAND(ID_FLASH_WRITETEST, OnFlashWritetest)
    ON_MESSAGE(WM_WRITE_COMPLETE, OnFlashWriteComplete)
    ON_MESSAGE(WM_READ_COMPLETE, OnFlashReadComplete)
    ON_MESSAGE(WM_FLUSH_COMPLETE, OnFlashFlushComplete)
    ON_MESSAGE(WM_CLOSE_COMPLETE, OnFlashCloseComplete)
    ON_MESSAGE(WM_STRESS_COMPLETE, OnFlashStressComplete)
	ON_COMMAND(ID_FLASH_READTEST, OnFlashReadtest)
	ON_COMMAND(ID_FLASH_FLUSHCACHE, OnFlashFlushcache)
	ON_COMMAND(ID_FLASH_CLOSE, OnFlashClose)
	ON_COMMAND(ID_STRESS_TEST, OnStressTest)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_COMMAND(ID_STOP_STRESS_TEST, OnStopStressTest)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.Create(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Remove this if you don't want tool tips or a resizeable toolbar
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFrameWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::OnFlashConfig() 
{
	// TODO: Add your command handler code here
	// turn on tracing.
	TraceLevel = 9;

	// Initialize dialog with some numbers that might be useful for debugging
	CCacheConfig cache_config_dialog;
	cache_config_dialog.m_page_size = 16;
	//cache_config_dialog.m_num_pages = 2000;
	cache_config_dialog.m_num_pages = 0;
	cache_config_dialog.m_hash_table_size = 100;
	cache_config_dialog.m_num_reserve_pages = 64;
	cache_config_dialog.m_memory_size_to_allocate = 10000;
	cache_config_dialog.m_dirty_page_writeback_threshold = 60;
	cache_config_dialog.m_dirty_page_error_threshold = 95;
	cache_config_dialog.m_write_back = -1;
	cache_config_dialog.m_write_through = 0;

	if (cache_config_dialog.DoModal() != IDOK)     	// open dialog box
		return;

	// We can't have both cache_config_dialog.m_num_pages and m_hash_table_size
	if (cache_config_dialog.m_hash_table_size)
		cache_config_dialog.m_num_pages = 0;

	CT_config.version = CM_CONFIG_VERSION;
	CT_config.page_size = cache_config_dialog.m_page_size;
	CT_config.page_table_size = cache_config_dialog.m_num_pages;
	CT_config.hash_table_size = cache_config_dialog.m_hash_table_size;
	CT_config.num_reserve_pages = cache_config_dialog.m_num_reserve_pages;
	CT_config.dirty_page_writeback_threshold = cache_config_dialog.m_dirty_page_writeback_threshold;
	CT_config.dirty_page_error_threshold = cache_config_dialog.m_dirty_page_error_threshold;

	// Save memory size to allocate for stress test.
	CT_memory_size_to_allocate = cache_config_dialog.m_memory_size_to_allocate;

	// Are we using write through cache or write back cache?
	// If the write back button is pressed, then m_write_back = 0 and m_write_through = 1.
	// If the write through button is pressed, then m_write_back = 1 and m_write_through = 0.
	// It seems backwards!
	CT_write_back = cache_config_dialog.m_write_back + 1;
	
	// Initialize cache test.
	STATUS status = Initialize_Test_Cache(&CT_config, CT_memory_size_to_allocate, 
		CT_write_back);

	if (status != NU_SUCCESS)
	{	 
		char cstatus[20];
 		CString completion_text;
		if (status == NU_NO_MEMORY)
			completion_text = "Not enough memory.";
		else
		{
			sprintf(cstatus, "Status = %d", status);
			completion_text = cstatus;
		}
		MessageBox(
			//AfxGetMainWnd()->m_hWnd, //    handle of owner window
			completion_text,	// address of text in message box
			"Initialize Cache Failed", // address of title of message box  
			MB_OK//    style of message box
		   );
	}	
}

void CMainFrame::OnFlashTestconfig() 
{
	// TODO: Add your command handler code here

	DWORD dialog_error = CommDlgExtendedError();
	CFileDialog cache_file_dialog(TRUE, // file open
		NULL, // no file extension,
		NULL, // initial filename
		0, // flags
		NULL, // filters
		AfxGetMainWnd() // parent window
		);

	dialog_error = CommDlgExtendedError();

	cache_file_dialog.m_ofn.lpstrTitle = "Select cache backup file";

	// This buffer must have room for the file name returned.
	cache_file_dialog.m_ofn.lpstrFile = CT_initial_file_name;
	cache_file_dialog.m_ofn.nMaxFileTitle = 256;
	cache_file_dialog.m_ofn.nMaxFile = 256;


	if (cache_file_dialog.DoModal() != IDOK)     	// open dialog box
		return;
	dialog_error = CommDlgExtendedError();
	CT_file_path[0] = 0;
	strcat(CT_file_path, cache_file_dialog.GetPathName( ));

	// Open the cache backing file.
	STATUS status = Device_Open(CT_file_path);
	if (status != NU_SUCCESS)
	{
	}

	return;
	
}

void CMainFrame::OnFlashWritetest() 
{
	// TODO: Add your command handler code here
	CWriteSequential write_sequential_dialog;

	// Initialize dialog with some numbers that might be useful for debugging
	write_sequential_dialog.m_starting_page = 0;
	write_sequential_dialog.m_num_pages = 100;

	if (write_sequential_dialog.DoModal() != IDOK)     	// open dialog box
		return;

	if (CT_config.page_table_size)
	{
		// Make sure the number of pages is not more than 
		// the number of pages mapped by the cache.
		if ((write_sequential_dialog.m_num_pages + write_sequential_dialog.m_starting_page)
			>= CT_config.page_table_size)
		{
			MessageBox(
				//AfxGetMainWnd()->m_hWnd, //    handle of owner window
				"Last page address must be less than number of pages mapped",	// address of text in message box
				"ERROR", // address of title of message box  
				MB_OK//    style of message box
			   );
			return;
		}
	}


	// Start the write test.
	STATUS status = Write_Sequential(write_sequential_dialog.m_starting_page, 
		write_sequential_dialog.m_num_pages,
		WM_WRITE_COMPLETE, // message to post
		AfxGetMainWnd()->m_hWnd	// window to post message to
		);

	if (status != NU_SUCCESS)
	{
	}



}

// We get called when Write_Sequential completes and posts a message.
LONG CMainFrame::OnFlashWriteComplete(WPARAM status, LPARAM num_pages) 
{

	char cnum_pages[50];
	sprintf(cnum_pages, "\nNumber of pages written: %d", num_pages);
	char cstatus[20];
	sprintf(cstatus, "Status = %d", status);
 	CString completion_text;

	if (status != NU_SUCCESS)
	{	 
		completion_text = cstatus;
		completion_text += cnum_pages;
		MessageBox(
			//AfxGetMainWnd()->m_hWnd, //    handle of owner window
			completion_text,	// address of text in message box
			"Flash Write Failed", // address of title of message box  
			MB_OK//    style of message box
		   );
	}
else
	{	 
		completion_text = cnum_pages;
		MessageBox(
			//AfxGetMainWnd()->m_hWnd, //    handle of owner window
			completion_text,	// address of text in message box
			"Flash Write Successful", // address of title of message box  
			MB_OK//    style of message box
		   );
	}

  return 0;

}


// We get called when Read_Sequential completes and posts a message.
LONG CMainFrame::OnFlashReadComplete(WPARAM status, LPARAM num_pages) 
{

	char cnum_pages[50];
	sprintf(cnum_pages, "\nNumber of pages read: %d", num_pages);
	char cstatus[20];
	sprintf(cstatus, "Status = %d", status);
 	CString completion_text;

	if (status != NU_SUCCESS)
	{	 
		completion_text = cstatus;
		completion_text += cnum_pages;
		MessageBox(
			//AfxGetMainWnd()->m_hWnd, //    handle of owner window
			completion_text,	// address of text in message box
			"Flash Read Failed", // address of title of message box  
			MB_OK//    style of message box
		   );
	}
else
	{	 
		completion_text = cnum_pages;
		MessageBox(
			//AfxGetMainWnd()->m_hWnd, //    handle of owner window
			completion_text,	// address of text in message box
			"Flash Read Successful", // address of title of message box  
			MB_OK//    style of message box
		   );
	}

  return 0;

}

// We get called when Flush_Cache completes and posts a message.
LONG CMainFrame::OnFlashFlushComplete(WPARAM status, LPARAM num_pages) 
{

	char cstatus[20];
	sprintf(cstatus, "Status = %d", status);
 	CString completion_text;
	completion_text = cstatus;

	if (status != NU_SUCCESS)
	{	 
		MessageBox(
			//AfxGetMainWnd()->m_hWnd, //    handle of owner window
			completion_text,	// address of text in message box
			"Flash Flush Failed", // address of title of message box  
			MB_OK//    style of message box
		   );
	}
else
	{	 
		MessageBox(
			//AfxGetMainWnd()->m_hWnd, //    handle of owner window
			completion_text,	// address of text in message box
			"Flash Flush Successful", // address of title of message box  
			MB_OK//    style of message box
		   );
	}

  return 0;

}

// We get called when Close_Cache completes and posts a message.
LONG CMainFrame::OnFlashCloseComplete(WPARAM status, LPARAM num_pages) 
{

	char cstatus[20];
	sprintf(cstatus, "Status = %d", status);
 	CString completion_text;
	completion_text = cstatus;

	if (status != NU_SUCCESS)
	{	 
		MessageBox(
			//AfxGetMainWnd()->m_hWnd, //    handle of owner window
			completion_text,	// address of text in message box
			"Flash Close Failed", // address of title of message box  
			MB_OK//    style of message box
		   );
	}
	else
	{	 
		MessageBox(
			//AfxGetMainWnd()->m_hWnd, //    handle of owner window
			completion_text,	// address of text in message box
			"Flash Close Successful", // address of title of message box  
			MB_OK//    style of message box
		   );
	}

	status = Device_Close();
	return 0;

}

// We get called when Stress test completes and posts a message.
LONG CMainFrame::OnFlashStressComplete(WPARAM status, LPARAM num_pages) 
{

	char cstatus[20];
	sprintf(cstatus, "Status = %d", status);
 	CString completion_text;
	completion_text = cstatus;

	if (status != NU_SUCCESS)
	{	 
		MessageBox(
			//AfxGetMainWnd()->m_hWnd, //    handle of owner window
			completion_text,	// address of text in message box
			"Flash Stress Test Flush Failed", // address of title of message box  
			MB_OK//    style of message box
		   );
	}
	else
	{	 
		MessageBox(
			//AfxGetMainWnd()->m_hWnd, //    handle of owner window
			completion_text,	// address of text in message box
			"Flash Stress Test Successful", // address of title of message box  
			MB_OK//    style of message box
		   );
	}

	status = Device_Close();
	return 0;

}



void CMainFrame::OnFlashReadtest() 
{
	// TODO: Add your command handler code here
	
	CReadSequential read_sequential_dialog;

	// Initialize dialog with some numbers that might be useful for debugging
	read_sequential_dialog.m_starting_page = 0;
	read_sequential_dialog.m_num_pages = 100;

	if (read_sequential_dialog.DoModal() != IDOK)     	// open dialog box
		return;

	if (CT_config.page_table_size)
	{
		// Make sure the number of pages is not more than 
		// the number of pages mapped by the cache.
		if ((read_sequential_dialog.m_num_pages + read_sequential_dialog.m_starting_page)
			>= CT_config.page_table_size)
		{
			MessageBox(
				//AfxGetMainWnd()->m_hWnd, //    handle of owner window
				"Last page address must be less than number of pages mapped",	// address of text in message box
				"ERROR", // address of title of message box  
				MB_OK//    style of message box
			   );
			return;
		}
	}

	// Start the write test.
	STATUS status = Read_Sequential(read_sequential_dialog.m_starting_page, 
		read_sequential_dialog.m_num_pages,
		WM_READ_COMPLETE, // message to post
		AfxGetMainWnd()->m_hWnd	// window to post message to
		);

	if (status != NU_SUCCESS)
	{
	}

}

void CMainFrame::OnFlashFlushcache() 
{
	// TODO: Add your command handler code here
	
	CFlushDialog flush_dialog;

	if (flush_dialog.DoModal() != IDOK)     	// open dialog box
		return;

	STATUS status = Flush_Cache(
		WM_FLUSH_COMPLETE, // message to post
		AfxGetMainWnd()->m_hWnd	// window to post message to
		);

}

void CMainFrame::OnFlashClose() 
{
	// TODO: Add your command handler code here
	
	CCloseFlash close_dialog;

	if (close_dialog.DoModal() != IDOK)     	// open dialog box
		return;

	STATUS status = Close_Cache(
		WM_CLOSE_COMPLETE, // message to post
		AfxGetMainWnd()->m_hWnd	// window to post message to
		);
}


void CMainFrame::OnStressTest() 
{
	// TODO: Add your command handler code here
	
	CStressDialog stress_dialog;

	// Set default value for number of threads.
	stress_dialog.m_num_threads = 2;

	if (stress_dialog.DoModal() != IDOK)     	// open dialog box
		return;

	STATUS status = Stress_Test_Cache(
		&CT_config,
		CT_memory_size_to_allocate,
		stress_dialog.m_num_threads,
		WM_STRESS_COMPLETE, // message to post
		AfxGetMainWnd()->m_hWnd	// window to post message to
		);
}

void CMainFrame::OnDestroy() 
{
	// Kill timer if we created one for statistics.
	KillTimer(2);
}

void CMainFrame::OnTimer(UINT nTimerID)
{
	CCacheTestDoc* pDoc = (CCacheTestDoc *)GetActiveDocument();
	ASSERT_VALID(pDoc);

	// Make the statistics string empty
	pDoc->m_statistics.Empty();

	STATUS status = Get_Cache_Stats(&pDoc->m_statistics, pDoc->m_cache_number);

	pDoc->UpdateAllViews(NULL);

	return;
}

void CMainFrame::OnStopStressTest() 
{
	// TODO: Add your command handler code here
	
	CStopStressDialog stress_dialog;

	if (stress_dialog.DoModal() != IDOK)     	// open dialog box
		return;

	stop_stress_test = 1;

}
