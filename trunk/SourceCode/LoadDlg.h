#pragma once


// LoadDlg dialog

class LoadDlg : public CDialog
{
	DECLARE_DYNAMIC(LoadDlg)

public:
	LoadDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~LoadDlg();

// Dialog Data
	enum { IDD = IDD_LOAD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   afx_msg void OnBnClickedBrowse();
   CString m_name;
   CString m_path;
protected:
   virtual void OnOK();
public:
   virtual BOOL OnInitDialog();
   };
