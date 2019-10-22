#pragma once


// PPRunData dialog

class PPRunData : public CPropertyPage
{
	DECLARE_DYNAMIC(PPRunData)

public:
	PPRunData();   // standard constructor
	virtual ~PPRunData();

// Dialog Data
	enum { IDD = IDD_RUNDATA };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   BOOL m_collectActorData;
   BOOL m_collectLandscapeScoreData;
};
