#pragma once

#include "Resource.h"

#include <grid\gridctrl.h>



//class AttrGrid : public CGridCtrl
//{
//public:
   
   //virtual void  EndEditing() { m_pParent->MakeDirty(); }
   //afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point) { m_pParent->MakeDirty(); }
//};


// TextToIdDlg dialog

class TextToIdDlg : public CDialog
{
	DECLARE_DYNAMIC(TextToIdDlg)

public:
	TextToIdDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~TextToIdDlg();

// Dialog Data
	enum { IDD = IDD_TEXT_TO_ID };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
   CComboBox m_layers;
   CComboBox m_textField;
   CComboBox m_idField;

   CGridCtrl  m_grid;


protected:
   void LoadFields();

public:
   virtual BOOL OnInitDialog();

   afx_msg void OnBnClickedOk();
   afx_msg void OnCbnSelchangeLayers();
   afx_msg void OnCbnChangeTextField();
};
