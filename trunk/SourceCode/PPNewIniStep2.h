#pragma once


// PPNewIniStep2 dialog

class PPNewIniStep2 : public CPropertyPage
{
	DECLARE_DYNAMIC(PPNewIniStep2)

public:
	PPNewIniStep2(CWnd* pParent = NULL);   // standard constructor
	virtual ~PPNewIniStep2();

// Dialog Data
	enum { IDD = IDD_NEWINI_STEP2 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
   CString m_pathLayer;
   afx_msg void OnBnClickedBrowse();
   virtual BOOL OnSetActive();
   };
