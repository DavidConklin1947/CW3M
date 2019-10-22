#pragma once


// PPNewIniStep3 dialog

class PPNewIniStep3 : public CPropertyPage
{
	DECLARE_DYNAMIC(PPNewIniStep3)

public:
	PPNewIniStep3(CWnd* pParent = NULL);   // standard constructor
	virtual ~PPNewIniStep3();

// Dialog Data
	enum { IDD = IDD_NEWINI_STEP3 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CString m_pathPolicy;
   afx_msg void OnBnClickedBrowse();

   virtual BOOL OnSetActive();
   };
