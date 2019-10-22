#if !defined(AFX_MAPLAYERDLG_H__1F0032A3_21AA_11D3_95A7_00A076B0010A__INCLUDED_)
#define AFX_MAPLAYERDLG_H__1F0032A3_21AA_11D3_95A7_00A076B0010A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MapLayerDlg.h : header file
//

#include <colorpicker.h>
#include "WinLib_resource.h"
#include "afxwin.h"
#include "afxcmn.h"

class MapWindow;
class MapLayer;

/////////////////////////////////////////////////////////////////////////////
// MapLayerDlg dialog

int WINLIBSAPI RunMapLayerDlg( MapWindow*, MapLayer *pLayer, CWnd *pParent=NULL );



class MapLayerDlg : public CDialog
{
friend int WINLIBSAPI RunMapLayerDlg( MapWindow *pMapWnd, MapLayer *pLayer, CWnd *pParent );

// Construction
protected:
	MapLayerDlg(MapWindow *pMapWnd, MapLayer *pLayer, CWnd* pParent = NULL);   // standard constructor

   MapWindow *m_pMapWnd;
   MapLayer *m_pLayer;
   long m_outlineColor;
   long m_bkgrColor;

// Dialog Data
	enum { IDD = IDD_MAPLAYERPROPS };
	CColorPicker	m_outlineColorBtn;
   CComboBox m_outlineWidth;
	CString	m_features;
   CString  m_totalArea;
   CString  m_avgArea;
   CString  m_areaRange;
   CString  m_fields;
	CString	m_file;
	CString	m_layerName;
	CString	m_type;
	BOOL	m_visible;
	BOOL	m_showOutlines;
   BOOL  m_varWidth;

// Overrides
	// ClassWizard generated virtual function overrides
public:
	//virtual BOOL DestroyWindow();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
   void UpdateInterface( void );
   void UpdateFieldInfo( void );
   void LoadFields();

	// Generated message map functions
	afx_msg void OnShowoutlines();
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
   afx_msg void OnBnClickedEditfieldinfo();
protected:
   virtual void OnOK();
public:
   afx_msg void OnBnClickedEditlabelinfo();
   afx_msg void OnBnClickedAddfield();
   afx_msg void OnBnClickedRemovefield();
   CComboBox m_fieldsCombo;
   CComboBox m_typeCombo;
   int m_width;
   int m_decimals;
   afx_msg void OnCbnSelchangeType();
   afx_msg void OnCbnSelchangeFieldcombo();
   CString m_actualExtents;
   CSliderCtrl m_transparencySlider;
   CEdit m_percentTransparency;
   afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
   afx_msg void OnBnClickedLibReorderCols();
   };

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAPLAYERDLG_H__1F0032A3_21AA_11D3_95A7_00A076B0010A__INCLUDED_)
