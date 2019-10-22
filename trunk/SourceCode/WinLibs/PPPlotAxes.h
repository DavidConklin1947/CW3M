#pragma once

#include "WinLib_resource.h"

// PPPlotAxes dialog

class GraphWnd;


class WINLIBSAPI PPPlotAxes : public CPropertyPage
{
	DECLARE_DYNAMIC(PPPlotAxes)

public:
	PPPlotAxes( GraphWnd *);
	virtual ~PPPlotAxes();

   GraphWnd *m_pGraph;
   int       m_axis;
   CEdit     m_scaleMin;
   CEdit     m_scaleMax;
   CEdit     m_ticMinor;
   CEdit     m_ticMajor;
   CEdit     m_decimals;
   CButton m_autoScale;
   CButton m_autoIncr;
   CComboBox  m_axes;

// Dialog Data
	enum { IDD = IDD_PLOTAXES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
   virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
public:
   virtual BOOL OnInitDialog();

   bool  Apply( bool redraw=true );
   afx_msg void OnEnChangeScalemin();
   afx_msg void OnEnChangeScalemax();
   afx_msg void OnEnChangeTicmajor();
   afx_msg void OnEnChangeTicminor();
   afx_msg void OnEnChangeTicdecimals();
   };
