#pragma once
#include "resource.h"


// NewPolicyDlg dialog

class NewPolicyDlg : public CDialog
{
	DECLARE_DYNAMIC(NewPolicyDlg)
   
public:
	NewPolicyDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~NewPolicyDlg();

// Dialog Data
	enum { IDD = IDD_NEWPOLICY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CString m_description;
   virtual BOOL OnInitDialog();
   BOOL m_isActorPolicy;
protected:
   virtual void OnOK();
};
