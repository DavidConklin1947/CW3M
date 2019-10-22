#pragma once

#include "Resource.h"

// SaveToDlg dialog

class SaveToDlg : public CDialog
{
	DECLARE_DYNAMIC(SaveToDlg)

public:
	SaveToDlg(LPCTSTR text, LPCTSTR defPath, CWnd* pParent = NULL);   // standard constructor
	virtual ~SaveToDlg();

// Dialog Data
	enum { IDD = IDD_SAVETO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   BOOL m_saveThisSession;
   BOOL m_saveToDisk;
   CString m_text;
   CString m_path;

   virtual BOOL OnInitDialog();
   afx_msg void OnBnClickedBrowse();
   };
