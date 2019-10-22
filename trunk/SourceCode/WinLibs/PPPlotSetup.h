#pragma once

#include "WinLib_resource.h"

class GraphWnd;


// PPPlotSetup dialog

class WINLIBSAPI PPPlotSetup : public CPropertyPage
{
	DECLARE_DYNAMIC(PPPlotSetup)

public:
	PPPlotSetup( GraphWnd *pGraph );
	virtual ~PPPlotSetup();

   GraphWnd *m_pGraph;

// Dialog Data
	enum { IDD = IDD_PLOTSETUP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   virtual BOOL OnInitDialog();
   virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

   bool  Apply( bool redraw=true );
   afx_msg void OnBnClickedFonttitle();
   afx_msg void OnBnClickedFontxaxis();
   afx_msg void OnBnClickedFontyaxis();
   afx_msg void OnEnChangeCaption();
   afx_msg void OnEnChangeTitle();
   afx_msg void OnEnChangeXaxis();
   afx_msg void OnEnChangeYaxis();
   afx_msg void OnBnClickedShowhorz();
   afx_msg void OnBnClickedShowvert();
   afx_msg void OnBnClickedGrooved();
   afx_msg void OnBnClickedRaised();
   afx_msg void OnBnClickedSunken();
   };
