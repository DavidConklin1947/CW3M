#pragma once
#include "afxwin.h"
#include "resource.h"


// PolicyColorDlg dialog

class PolicyColorDlg : public CDialog
{
	DECLARE_DYNAMIC(PolicyColorDlg)

public:
	PolicyColorDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~PolicyColorDlg();

// Dialog Data
	enum { IDD = IDD_SETPOLICYCOLORS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CComboBox m_red;
   CComboBox m_green;
   CComboBox m_blue;

   afx_msg void OnBnClickedOk();
   virtual BOOL OnInitDialog();
};
