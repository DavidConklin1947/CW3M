#pragma once
#include "afxwin.h"


#include <maplayer.h>
#include <ugctrl.h>


// AreaSummaryDlg dialog

class AreaSummaryDlg : public CDialog
{
	DECLARE_DYNAMIC(AreaSummaryDlg)

public:
	AreaSummaryDlg(MapLayer*, CWnd* pParent = NULL);   // standard constructor
	virtual ~AreaSummaryDlg();

// Dialog Data
	enum { IDD = IDD_AREASUMMARY };

   MapLayer *m_pLayer;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

   void UpdateAreas();

   CUGCtrl m_grid;


	DECLARE_MESSAGE_MAP()
public:
   CComboBox m_fields;
   afx_msg void OnBnClickedSaveas();
   virtual BOOL OnInitDialog();
protected:
   virtual void OnOK();
public:
   CString m_query;
   };
