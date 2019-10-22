#pragma once

#include "resource.h"


// SysInfoDlg dialog

class SysInfoDlg : public CDialog
{
	DECLARE_DYNAMIC(SysInfoDlg)

public:
	SysInfoDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~SysInfoDlg();

// Dialog Data
	enum { IDD = IDD_SYSINFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CString m_memUsed;
   CString m_physMemAvail;
   CString m_virtMemAvail;
   CString m_cpu;
   virtual BOOL OnInitDialog();
   };
