#pragma once


// PPNewIniFinish dialog

class PPNewIniFinish : public CPropertyPage
{
	DECLARE_DYNAMIC(PPNewIniFinish)

public:
	PPNewIniFinish(CWnd* pParent = NULL);   // standard constructor
	virtual ~PPNewIniFinish();

// Dialog Data
	enum { IDD = IDD_NEWINI_FINISH };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   BOOL m_runIniEditor;

   BOOL OnSetActive();
   };
