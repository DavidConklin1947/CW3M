#pragma once
#include "resource.h"
#include "afxcmn.h"
#include "afxwin.h"

// SplashDlg dialog

class SplashDlg : public CDialog
{
	DECLARE_DYNAMIC(SplashDlg)

public:
	SplashDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~SplashDlg();

// Dialog Data
	enum { IDD = IDD_USER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
   virtual void OnOK();
public:
   CString m_name;
   CString m_mru1;
   CString m_mru2;
   CString m_mru3;
   CString m_mru4;
   virtual BOOL OnInitDialog();
   CComboBox m_files;
   CString m_filename;

   bool m_isNewProject;
   bool m_runIniEditor;

   afx_msg void OnCbnSelchangeFile();
protected:
   virtual void OnCancel();
   bool CreateNewProject();
   bool OpenProject();
   bool SaveRegistryKeys();

public:
   CBitmapButton m_cancelButton;
   afx_msg void OnStartup();
   afx_msg void OnBnClickedNew();
   afx_msg void OnBnClickedOpen();
   };
