#pragma once
#include "afxwin.h"
#include "resource.h"



// WizardDlg dialog

class WizardDlg : public CDialog
{
	DECLARE_DYNAMIC(WizardDlg)

public:
	WizardDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~WizardDlg();

// Dialog Data
	enum { IDD = IDD_WIZARD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CButton m_btnExploreIDU;
   virtual BOOL OnInitDialog();
   afx_msg BOOL OnTtnNeedText(UINT id, NMHDR *pNMHDR, LRESULT *pResult);
   };
