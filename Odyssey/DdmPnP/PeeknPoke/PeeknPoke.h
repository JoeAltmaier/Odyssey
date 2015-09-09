// PeeknPoke.h : main header file for the PEEKNPOKE application
//

#if !defined(AFX_PEEKNPOKE_H__FBA88EFB_D28B_11D2_9BF6_00105A2459CB__INCLUDED_)
#define AFX_PEEKNPOKE_H__FBA88EFB_D28B_11D2_9BF6_00105A2459CB__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// Forward References
class CMainFrame;

/////////////////////////////////////////////////////////////////////////////
// CPeeknPokeApp:
// See PeeknPoke.cpp for the implementation of this class
//

class CPeeknPokeApp : public CWinApp
{
public:
	CPeeknPokeApp();
	static CPeeknPokeApp * GetApp() {return (CPeeknPokeApp *)AfxGetApp(); }
	CMainFrame * GetMainFrame() {return (CMainFrame *)m_pMainWnd; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPeeknPokeApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CPeeknPokeApp)
	afx_msg void OnAppAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PEEKNPOKE_H__FBA88EFB_D28B_11D2_9BF6_00105A2459CB__INCLUDED_)
