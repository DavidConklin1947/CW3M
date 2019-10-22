#pragma once
#include "afxwin.h"


// BasemapDlg dialog

class BasemapDlg : public CDialogEx
{
	DECLARE_DYNAMIC(BasemapDlg)

public:
	BasemapDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~BasemapDlg();

// Dialog Data
	enum { IDD = IDD_BASEMAP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CString m_localPath;
   CString m_wmsPath;
   afx_msg void OnBnClickedBrowse();
   CListBox m_wmsList;
   CListBox m_layerList;
   afx_msg void OnBnClickedAddwms();
   virtual BOOL OnInitDialog();
   virtual void OnOK();
   };
