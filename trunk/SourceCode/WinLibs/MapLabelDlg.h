#pragma once
#include "afxwin.h"

#include <map.h>
#include "WinLib_resource.h"

// MapLabelDlg dialog

class MapLabelDlg : public CDialog
{
	DECLARE_DYNAMIC(MapLabelDlg)

public:
	MapLabelDlg(Map *pMap, CWnd* pParent = NULL);   // standard constructor
	virtual ~MapLabelDlg();

   Map      *m_pMap;
   MapLayer *m_pLayer;
   LOGFONT   m_logFont;
   COLORREF  m_color;
   CFont     m_font;
   bool      m_isDirty;

// Dialog Data
	enum { IDD = IDD_MAPLABELS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   void LoadFields();

	DECLARE_MESSAGE_MAP()
public:
   afx_msg void OnBnClickedSetfont();
   afx_msg void OnBnClickedShowlabels();
   afx_msg void OnBnClickedOk();
   CStatic m_sample;
   CComboBox m_layers;
   CComboBox m_fields;
   virtual BOOL OnInitDialog();
   afx_msg void OnCbnEditchangeLayers();
   BOOL m_showLabels;
   };
