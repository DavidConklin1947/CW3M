#pragma once

#include "WinLib_resource.h"


// NameDlg dialog

class WINLIBSAPI NameDlg : public CDialog
{
	DECLARE_DYNAMIC(NameDlg)

public:
	NameDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~NameDlg();

// Dialog Data
	enum { IDD = IDD_NAME };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

   // Generated message map functions
   virtual BOOL OnInitDialog();

public:
   CString m_name;
   CString m_title;
   };
