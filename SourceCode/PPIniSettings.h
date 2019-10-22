#pragma once

#include "resource.h"

#include <TabPageSSL.h>
#include "afxwin.h"
#include "afxcmn.h"
#include <grid\gridctrl.h>

class IniFileEditor;



// PPIniSettings dialog

class PPIniSettings : public CTabPageSSL
{
	DECLARE_DYNAMIC(PPIniSettings)

public:
	PPIniSettings( IniFileEditor* );   // standard constructor
	virtual ~PPIniSettings();
   
   IniFileEditor *m_pParent;

   // Dialog Data
	enum { IDD = IDD_INISETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
	DECLARE_MESSAGE_MAP()
   afx_msg void OnBnClickedMapoutputs();

   BOOL m_loadShared;
   BOOL m_debug;
   BOOL m_mapOutputs;
   BOOL m_noBuffer;
   int  m_interval;         // default simulation period (years)
   int  m_defaultPeriod;
   //CListCtrl m_vars;

public:
   bool StoreChanges();

   virtual BOOL OnInitDialog();
   
   afx_msg void MakeDirty();
   afx_msg void OnBnClickedAdd();
   afx_msg void OnBnClickedRemove();
   CGridCtrl m_varGrid;
   int m_mapUnits;
   };
