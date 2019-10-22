#pragma once
#include "afxwin.h"

#include <vdata.h>
#include "resource.h"


// AreaQueryDlg dialog

class AreaQueryDlg : public CDialog
{
	DECLARE_DYNAMIC(AreaQueryDlg)

public:
	AreaQueryDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~AreaQueryDlg();

   int m_col;
   VData m_dataValue;

// Dialog Data
	enum { IDD = IDD_AREAQUERY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CComboBox m_fields;
   CEdit m_value;
   virtual BOOL OnInitDialog();
protected:
   virtual void OnOK();
};
