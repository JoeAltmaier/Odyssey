#if !defined(AFX_CONFIGSTATSDIALOG_H__1FD75933_53CB_11D2_97EF_00104B242C42__INCLUDED_)
#define AFX_CONFIGSTATSDIALOG_H__1FD75933_53CB_11D2_97EF_00104B242C42__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ConfigStatsDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConfigStatsDialog dialog

class CConfigStatsDialog : public CDialog
{
// Construction
public:
	CConfigStatsDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConfigStatsDialog)
	enum { IDD = IDD_DIALOG7 };
	UINT	m_refresh_rate;
	UINT	m_display_once;
	UINT	m_cache_number;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigStatsDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConfigStatsDialog)
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGSTATSDIALOG_H__1FD75933_53CB_11D2_97EF_00104B242C42__INCLUDED_)
