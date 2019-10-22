#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// LinearDensityDlg dialog

class LinearDensityDlg : public CDialog
{
	DECLARE_DYNAMIC(LinearDensityDlg)

public:
	LinearDensityDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~LinearDensityDlg();

// Dialog Data
	enum { IDD = IDD_LINEDENSITY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CComboBox m_layers;
   CComboBox m_fields;
   float m_distance;
   CProgressCtrl m_progress;
   afx_msg void OnBnClickedBrowse();
   virtual BOOL OnInitDialog();
   virtual void OnOK();
   CString m_thisQuery;
   CString m_toQuery;
   BOOL m_useToQuery;
   BOOL m_useThisQuery;
   };
