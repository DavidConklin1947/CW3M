#pragma once
#include "afxwin.h"


// ConstraintsDlg dialog

class ConstraintsDlg : public CDialog
{
	DECLARE_DYNAMIC(ConstraintsDlg)

public:
	ConstraintsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~ConstraintsDlg();

// Dialog Data
	enum { IDD = IDD_CONSTRAINTS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

   void LoadConstraints();

	DECLARE_MESSAGE_MAP()
public:
   virtual BOOL OnInitDialog();
   virtual void OnOK();
   CListBox m_constraints;
   afx_msg void OnBnClickedAdd();
   afx_msg void OnBnClickedRemove();
   afx_msg void OnBnClickedEdit();
   };
