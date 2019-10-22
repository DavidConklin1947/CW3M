#pragma once
#include "afxcmn.h"
#include "resource.h"

class MapLayer;

// DatabaseInfoDlg dialog

class DatabaseInfoDlg : public CDialog
{
	DECLARE_DYNAMIC(DatabaseInfoDlg)

public:
	DatabaseInfoDlg(MapLayer *, CWnd* pParent = NULL);   // standard constructor
	virtual ~DatabaseInfoDlg();

// Dialog Data
	enum { IDD = IDD_IDUINFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

   void LoadFields();

public:
   CTreeCtrl m_fields;
   CString m_path;
   CString m_name;
   MapLayer *m_pLayer;

   afx_msg void OnBnClickedSave();
   virtual BOOL OnInitDialog();
   };
