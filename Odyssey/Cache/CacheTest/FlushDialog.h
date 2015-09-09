#if !defined(AFX_FLUSHDIALOG_H__02098865_4D4F_11D2_A15A_444553540000__INCLUDED_)
#define AFX_FLUSHDIALOG_H__02098865_4D4F_11D2_A15A_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// FlushDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFlushDialog dialog

class CFlushDialog : public CDialog
{
// Construction
public:
	CFlushDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFlushDialog)
	enum { IDD = IDD_DIALOG5 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFlushDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFlushDialog)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FLUSHDIALOG_H__02098865_4D4F_11D2_A15A_444553540000__INCLUDED_)
