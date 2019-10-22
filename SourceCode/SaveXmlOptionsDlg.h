#pragma once

#include "resource.h"


// SaveXmlOptionsDlg dialog

class SaveXmlOptionsDlg : public CDialog
{
	DECLARE_DYNAMIC(SaveXmlOptionsDlg)

public:
	SaveXmlOptionsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~SaveXmlOptionsDlg();

// Dialog Data
	enum { IDD = IDD_SAVEXMLOPTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   afx_msg void OnBnClickedBrowse();
   CString m_file;

   int m_saveType;      // 0=dont save, 1=save to project file, 2=save to external file

protected:
   virtual void OnOK();
public:
   virtual BOOL OnInitDialog();
   afx_msg void OnBnClickedRadio();
   };
