#pragma once

#include <PtrArray.h>



class ScenarioPage
{
public:
   CString m_name;       // caption for this window
   CWnd   *m_pWnd;       // container for controls on this page
   PtrArray< CWnd > m_controlArray;
};

// ScenarioWizard

class ScenarioWizard : public CWnd
{
	DECLARE_DYNAMIC(ScenarioWizard)

public:
	ScenarioWizard();
	virtual ~ScenarioWizard();

protected:
	DECLARE_MESSAGE_MAP()

   PtrArray< ScenarioPage > m_scenarioPageArray;
   int m_currentPage;

public:
   afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
   
   //afx_msg void OnEditCopy();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
   afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
};


