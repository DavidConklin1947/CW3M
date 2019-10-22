#pragma once

#include "resource.h"

#include <TabPageSSL.h>
#include <cubicSpline.h>
#include "afxwin.h"

class IniFileEditor;
struct ENV_EVAL_MODEL;


// PPIniModels dialog

class PPIniModels : public CTabPageSSL
{
	DECLARE_DYNAMIC(PPIniModels)

public:
	PPIniModels(IniFileEditor *pParent);   // standard constructor
	virtual ~PPIniModels();

// Dialog Data
	enum { IDD = IDD_INIMODELS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
   
   void LoadModel();
   int  SaveModel();

   bool m_isModelDirty;

public:
   bool StoreChanges();

public:
   IniFileEditor *m_pParent;
   ENV_EVAL_MODEL *m_pInfo;

   CCheckListBox m_models;
   CString m_label;
   CString m_path;
   int m_id;
   //BOOL m_useSelfInterested;
   //BOOL m_useAltruism;
   BOOL m_showInResults;
   BOOL m_showUnscaled;
   BOOL m_useCol;
   CString m_field;
   CString m_initFnInfo;
   BOOL m_cdf;
   float m_minRaw;
   float m_maxRaw;
   CString m_rawUnits;
   CubicSplineWnd m_spline;

   afx_msg void OnBnClickedBrowse();
   afx_msg void OnBnClickedAdd();
   afx_msg void OnBnClickedRemove();
   afx_msg void OnBnClickedUsecol();
   afx_msg void OnBnClickedCdf();
   afx_msg void OnEnChangeLabel();

   afx_msg void OnLbnChkchangeModels();

   virtual BOOL OnInitDialog();

   afx_msg void MakeDirty();

   afx_msg void OnLbnSelchangeModels();
   };
