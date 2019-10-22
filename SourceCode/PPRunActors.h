#pragma once

#include "resource.h"
#include "afxwin.h"

// PPRunActors dialog

class PPRunActors : public CPropertyPage
{
	DECLARE_DYNAMIC(PPRunActors)

public:
	PPRunActors();
	virtual ~PPRunActors();

// Dialog Data
	enum { IDD = IDD_RUNACTORS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   virtual BOOL OnInitDialog();
   afx_msg void OnBnClickedGenerate();
   int m_count;
   afx_msg void OnBnClickedRandom();
   afx_msg void OnBnClickedUniform();
   afx_msg void OnBnClickedCurrent();
   afx_msg void OnBnClickedViewcurrent();
};
