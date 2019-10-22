#pragma once
#include "afxwin.h"

#include "resource.h"


// SelectTableDlg dialog

class SelectTableDlg : public CDialog
{
	DECLARE_DYNAMIC(SelectTableDlg)

public:
	SelectTableDlg(LPCTSTR mdbFile, CWnd* pParent = NULL);   // standard constructor
	virtual ~SelectTableDlg();

// Dialog Data
	enum { IDD = IDD_SELECTTABLE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CListBox m_tables;

   CString m_mdbFile;
   CString m_table;


   virtual BOOL OnInitDialog();
protected:
   virtual void OnOK();
   };
