#pragma once

#include "resource.h"
#include "afxwin.h"


// AddResultDlg dialog

class AddResultDlg : public CDialog
{
	DECLARE_DYNAMIC(AddResultDlg)

public:
	AddResultDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~AddResultDlg();

   CString result;

// Dialog Data
	enum { IDD = IDD_ADDRESULT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

   void LoadValues();

	DECLARE_MESSAGE_MAP()

public:
   virtual BOOL OnInitDialog();
   void LoadFieldInfo();
   afx_msg void OnCbnSelchangeFields();

protected:
   virtual void OnOK();

public:
   CComboBox m_fields;
   CComboBox m_values;
   CString   m_result;
   CEdit m_bufferFn;
   CEdit m_addPointFn;
   CEdit m_incrementFn;
   CEdit m_expandFn;
   afx_msg void OnBnClickedBuffer();
   afx_msg void OnBnClickedAddpoint();
   afx_msg void OnBnClickedIncrement();
   afx_msg void OnBnClickedField();
   afx_msg void OnBnClickedExpand();
   CButton m_field;
   };
