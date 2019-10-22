#pragma once

#include "afxwin.h"

class GraphWnd;


// PPPlotLines dialog

class WINLIBSAPI PPPlotLines : public CPropertyPage
{
	DECLARE_DYNAMIC(PPPlotLines)

public:
	PPPlotLines( GraphWnd *pGraph );
	virtual ~PPPlotLines();

   GraphWnd *m_pGraph;
   int       m_lineNo;


// Dialog Data
	enum { IDD = IDD_PLOTLINES };

protected:
   void    LoadLineInfo( void );

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
   bool m_isDirty;

   void EnableControls( void );

public:
   afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
   virtual BOOL OnInitDialog();
   afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);

   CListBox m_lines;

   afx_msg void OnLbnSelchangeLine();
   afx_msg void OnEnChangeLinelabel();
   afx_msg void OnCbnSelchangeLinecolor();
   afx_msg void OnCbnSelchangeLinestyle();
   afx_msg void OnCbnSelchangeLinewidth();
   afx_msg void OnCbnSelchangeMarker();
   afx_msg void OnEnChangeMarkerincr();
   afx_msg void OnBnClickedShowline();
   afx_msg void OnBnClickedSmooth();
   afx_msg void OnBnClickedShowmarker();
   //CComboBox m_color;
   //CComboBox m_lineStyle;
   //CComboBox m_lineWidth;
   //CComboBox m_markerSymbol;
   afx_msg void OnEnChangeLegendheight();
   afx_msg void OnEnChangeTension();
   float m_tension;
   };
