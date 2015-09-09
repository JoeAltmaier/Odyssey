// CacheFileDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCacheFileDialog dialog

class CCacheFileDialog : public CFileDialog
{
	DECLARE_DYNAMIC(CCacheFileDialog)

public:
	CCacheFileDialog(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);

protected:
	//{{AFX_MSG(CCacheFileDialog)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
