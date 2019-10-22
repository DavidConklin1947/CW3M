#pragma once
#include "afxwin.h"
#include "resource.h"


// ExportDataDlg dialog

class ExportDataDlg : public CDialog
{
	DECLARE_DYNAMIC(ExportDataDlg)

public:
	ExportDataDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~ExportDataDlg();

// Dialog Data
	enum { IDD = IDD_EXPORTDATADIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
   CStringArray m_dataStringArray;
   CListBox     m_dataList;
   CComboBox    m_delimiterCombo;

   virtual BOOL OnInitDialog();

public:
   afx_msg void OnBnClickedExportbutton();
};
