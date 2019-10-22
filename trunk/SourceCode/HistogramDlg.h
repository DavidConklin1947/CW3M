#pragma once
#include "afxwin.h"

class MapLayer;
#include <ScatterWnd.h>
#include <FDataObj.h>
#include <histogram.h>
#include <vector>

#include <mtparser\mtparser.h>

using std::vector;

struct PARSER_VAR
   {
   int col;
   MTDOUBLE value;
   };

// HistogramDlg dialog

class HistogramDlg : public CDialog
{
	DECLARE_DYNAMIC(HistogramDlg)

public:
	HistogramDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~HistogramDlg();

// Dialog Data
	enum { IDD = IDD_HISTOGRAM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

   void GenerateHistogram( bool updatePlot );
   int PopulateBins( THistogram<float> &histo, vector<float> &vData, int categoryCol, bool updatePlot );
   void UpdateAttrValues( int attrIndex );

   ScatterWnd m_histoPlot;
   FDataObj   m_histoData;

   ScatterWnd m_plot;
   FDataObj   m_data;

   VData m_curAttrVal;
   float m_curAttrMin;
   float m_curAttrMax;

   float m_mean;
   float m_stdDev;
   float m_areaWtMean;

	DECLARE_MESSAGE_MAP()
public:
   MapLayer *m_pLayer;

   virtual BOOL OnInitDialog();
   CListBox m_fields;
   CComboBox m_categories;
   CComboBox m_attrs;
   
   afx_msg void OnBnClickedSave();
   afx_msg void OnCbnSelchangeCategories();
   int m_binCount;
   afx_msg void OnLbnSelchangeFields();
   afx_msg void OnCbnSelchangeAttr();
   BOOL m_constrain;
   CString m_query;
   CString m_expr;
   CStatic m_noText;
   afx_msg void OnBnClickedButton1();
   afx_msg void OnBnClickedRadio1();
   afx_msg void OnBnClickedRadio2();
   afx_msg void OnLbnDblclkFields();
   CEdit m_exprCtrl;
   };
