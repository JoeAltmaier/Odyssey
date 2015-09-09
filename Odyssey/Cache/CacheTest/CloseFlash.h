#if !defined(AFX_CLOSEFLASH_H__32C849B3_5235_11D2_97EF_00104B242C42__INCLUDED_)
#define AFX_CLOSEFLASH_H__32C849B3_5235_11D2_97EF_00104B242C42__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// CloseFlash.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCloseFlash dialog

class CCloseFlash : public CDialog
{
// Construction
public:
	CCloseFlash(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCloseFlash)
	enum { IDD = IDD_DIALOG6 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCloseFlash)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCloseFlash)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLOSEFLASH_H__32C849B3_5235_11D2_97EF_00104B242C42__INCLUDED_)
