#if !defined(AFX_MSGWND_H__0A486093_D747_11D2_9BF8_00105A2459CB__INCLUDED_)
#define AFX_MSGWND_H__0A486093_D747_11D2_9BF8_00105A2459CB__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MsgWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMsgWnd window

class CMsgWnd : public CSplitterWnd
{
// Construction
public:
	CMsgWnd();

// Attributes
public:
	int GetSplitterWidth() const { return m_cxSplitter; }

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMsgWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMsgWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMsgWnd)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MSGWND_H__0A486093_D747_11D2_9BF8_00105A2459CB__INCLUDED_)
