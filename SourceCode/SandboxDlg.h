#pragma once

#include "EnvEngine\Policy.h"
#include "Sandbox.h"
#include "resource.h"
#include "afxwin.h"

// SandboxDlg dialog

class SandboxDlg : public CDialog
{
	DECLARE_DYNAMIC(SandboxDlg)

public:
	SandboxDlg( Policy *pPolicy, CWnd* pParent = NULL );   // standard constructor
	virtual ~SandboxDlg();

// Dialog Data
	enum { IDD = IDD_SANDBOXDLG };

protected:
   Policy  *m_pPolicy;      // policy whose efficacies are to be calculated
   Sandbox *m_pSandbox;
   int      m_yearsToRun;
   int      m_percentCoverage;
   bool     m_runCompleted;

public:
   bool    *m_useMetagoalArray;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
   // This dialog must be modal.
   virtual BOOL Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL){ return CDialog::Create(lpszTemplateName, pParentWnd); }

public:
   virtual BOOL OnInitDialog();
   virtual BOOL DestroyWindow();

protected:
   virtual void OnOK();
   virtual void OnCancel();
public:
   CCheckListBox m_metagoals;
};
