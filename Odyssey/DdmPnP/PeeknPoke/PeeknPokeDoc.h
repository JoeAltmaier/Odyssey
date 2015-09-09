// PeeknPokeDoc.h : interface of the CPeeknPokeDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_PEEKNPOKEDOC_H__FBA88F01_D28B_11D2_9BF6_00105A2459CB__INCLUDED_)
#define AFX_PEEKNPOKEDOC_H__FBA88F01_D28B_11D2_9BF6_00105A2459CB__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "StringEx.h"
#include "PortConfig.h"
#include "Parser.h"

class CPeeknPokeView;

class MsgWndView;

class CPeeknPokeDoc : public CDocument
{
protected: // create from serialization only
	CPeeknPokeDoc();
	DECLARE_DYNCREATE(CPeeknPokeDoc)

// Attributes
	CStringEx	m_TextLine;
	CStringEx	m_CmdArray[256];
	CStringList	m_ReadBuffer;
	CStringList m_LineList;

	int			iCmdIndex;

	BOOL		m_File;
	BOOL		m_Connected;
	UINT		m_ComPort;
	int			m_Baud;
	int			m_Data;
	int			m_Parity;
	int			m_StopBit;

	CSerialPort	m_Port;

	CParser m_Parser;
	CPeeknPokeView* pPnPView;
	MsgWndView* pMWView;

public:
	CStringList*	GetLineList(){ return &m_LineList; }
	MsgWndView*		GetMsgView(){ return pMWView; }
	CPeeknPokeView*	GetPnPView(){ return pPnPView; }
	CPeeknPokeDoc*	GetPnPDoc()	{ return this; }

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPeeknPokeDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	//void DisplayMessage();
	virtual ~CPeeknPokeDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CPeeknPokeDoc)
	afx_msg void OnConfigPort();
	afx_msg void OnFileOpen();
	afx_msg void OnConnect();
	afx_msg void OnUpdateConnect(CCmdUI* pCmdUI);
	afx_msg void OnDisconnect();
	afx_msg void OnUpdateDisconnect(CCmdUI* pCmdUI);
	afx_msg void OnCompile();
	afx_msg void OnUpdateCompile(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PEEKNPOKEDOC_H__FBA88F01_D28B_11D2_9BF6_00105A2459CB__INCLUDED_)
