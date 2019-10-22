#pragma once

#include "plot3d/ScatterPlot3DWnd.h"
#include "plot3d/ShapePreviewWnd.h"

#include "WinLib_resource.h"

//  ScatterPropertiesDlg.h


class ScatterPropertiesDlg : public CDialog
{

public:
	ScatterPropertiesDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~ScatterPropertiesDlg();

private:
   PointCollectionArray  m_dataArray;
   PointCollectionArray *m_pParentDataArray;

   CComboBox m_dataSetCombo;
   CComboBox m_lineWidthCombo;
   CComboBox m_pointSizeCombo;
   CComboBox m_pointTypeCombo;
   CStatic   m_colorStatic;
   CStatic   m_colorStaticLines;
   CStatic   m_previewStatic;

   ShapePreviewWnd m_previewWnd;

   void UpdateProperties( void );

// Dialog Data
	enum { IDD = IDD_S3D_DATA_SET_PROPS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
   virtual BOOL OnInitDialog();
   afx_msg void OnBnClickedSetColorButton();
   afx_msg void OnBnClickedSetColorButtonLines();
   afx_msg void OnPaint();
   afx_msg void OnCbnSelchangeDataSetCombo();
protected:
   virtual void OnOK();
public:
   afx_msg void OnCbnSelchangeLineWidthCombo();
   afx_msg void OnCbnSelchangePointSizeCombo();
   afx_msg void OnCbnSelchangePointTypeCombo();
};