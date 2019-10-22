#pragma once

#include "winlib_resource.h"
#include <afxdialogex.h>

// ReportBox dialog


class WINLIBSAPI ReportBox : public CDialogEx
{
	DECLARE_DYNAMIC(ReportBox)

public:
	ReportBox(CWnd* pParent = NULL);   // standard constructor
	virtual ~ReportBox();

// Dialog Data
	enum { IDD = IDD_ERRORREPORT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CString m_msg;
   CString m_hdr;
   int m_flags;
   BOOL m_output;
   BOOL m_noShow;
   virtual void OnOK();
   virtual BOOL OnInitDialog();
   };
