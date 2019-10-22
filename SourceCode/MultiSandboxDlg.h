#pragma once

#include "resource.h"
#include "sandbox.h"


// MultiSandboxDlg dialog

class MultiSandboxDlg : public CDialog
{
	DECLARE_DYNAMIC(MultiSandboxDlg)

public:
	MultiSandboxDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~MultiSandboxDlg();

// Dialog Data
	enum { IDD = IDD_MULTISANDBOXDLG };


protected:
   Sandbox *m_pSandbox;
   int      m_yearsToRun;
   int      m_percentCoverage;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
   // This dialog must be modal.
   virtual BOOL Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL){ return CDialog::Create(lpszTemplateName, pParentWnd); }

public:
   virtual BOOL OnInitDialog();

protected:
   virtual void OnOK();
   virtual void OnCancel();
public:
   CCheckListBox m_metagoals;
   CCheckListBox m_policies;
};
