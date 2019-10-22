#pragma once

#include "resource.h"

#include <TabPageSSL.h>
#include "afxwin.h"

class IniFileEditor;


// PPIniLayers dialog

class PPIniLayers : public CTabPageSSL
{
	DECLARE_DYNAMIC(PPIniLayers)

public:
	PPIniLayers(IniFileEditor* pParent);   // standard constructor
	virtual ~PPIniLayers();

// Dialog Data
	enum { IDD = IDD_INILAYERS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

   bool m_isLayersDirty;
   bool m_isLayerDirty;

public:
   IniFileEditor *m_pParent;

   CDragListBox m_layers;
   CString m_labelLayer;
   CString m_pathLayer;
   CString m_pathFieldInfo;
   CStatic m_color;
   int m_type;
   int m_records;
   BOOL m_includeData;
   long m_colorRef;

   void LoadLayer();
   bool StoreChanges();
   void SaveCurrentLayer();

   afx_msg void OnBnClickedBrowselayers();
   afx_msg void OnBnClickedSetcolor();
   afx_msg void OnBnClickedAddlayer();
   afx_msg void OnBnClickedRemovelayer();
   virtual BOOL OnInitDialog();
   afx_msg void OnLbnSelchangeLayers();
   afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
   afx_msg void OnEnChangeLabellayer();

   afx_msg void OnBnClickedRunfieldeditor();
   afx_msg void OnBnClickedBrowsefieldinfo();

   afx_msg void MakeDirty();
   afx_msg void MakeLayerDirty();


   };
