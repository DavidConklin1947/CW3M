#pragma once


// PPNewIniStep1 dialog

class PPNewIniStep1 : public CPropertyPage
{
	DECLARE_DYNAMIC(PPNewIniStep1)

public:
	PPNewIniStep1(CWnd* pParent = NULL);   // standard constructor
	virtual ~PPNewIniStep1();

// Dialog Data
	enum { IDD = IDD_NEWINI_STEP1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CString m_projectName;
   CString m_pathIni;
   afx_msg void OnBnClickedBrowse();
   virtual BOOL OnSetActive();
   };
