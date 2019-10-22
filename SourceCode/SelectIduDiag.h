#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "resource.h"

// SelectIduDiag dialog

class SelectIduDiag : public CDialog
{
	DECLARE_DYNAMIC(SelectIduDiag)

public:
	SelectIduDiag(CWnd* pParent = NULL);   // standard constructor
	virtual BOOL OnInitDialog();
	virtual ~SelectIduDiag();
	virtual BOOL UpdateData( BOOL bSaveAndValidate = TRUE );

// Dialog Data
	enum { IDD = IDD_SELECTIDU };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	CString m_strIduText;
	CListCtrl m_listView;

	CArray <int, int> m_selectedIDUsTemporary;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedRemoveidu();
public:
	afx_msg void OnBnClickedAddidutext();
public:
	afx_msg void OnBnClickedOk();
public:
	afx_msg void OnBnClickedAddcurrentidu();
};
