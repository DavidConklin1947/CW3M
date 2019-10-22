#pragma once
#include "afxwin.h"
#include "dataGrid.h"

// FindDataDlg dialog

class FindDataDlg : public CDialog
{
	DECLARE_DYNAMIC(FindDataDlg)

public:
	FindDataDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~FindDataDlg();

// Dialog Data
	enum { IDD = IDD_FINDDATA };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

   void FindReplace( bool replace );

public:
   void SetGrid( DataGrid *pGrid );
   void AllowReplace( bool );

protected:
   DataGrid *m_pGrid;

   int  m_col;
   long m_row;

   bool  m_allowReplace;
   bool  m_showingMore;
   CRect m_moreRect;       // all these rects are client coords
   CRect m_findRect;
   CRect m_closeRect;
   int   m_moreLessSize;

   BOOL m_matchCase;
   BOOL m_matchEntire;
   BOOL m_findAll;
   BOOL m_searchThis;
   BOOL m_searchSelected;
   BOOL m_searchAll;
   BOOL m_searchUp;
   CComboBox m_fields;
   CString m_findString;
   CString m_replaceString;
   CString m_field;

   afx_msg void OnBnClickedOk();
   afx_msg void OnBnClickedReplace();
   afx_msg void OnBnClickedCancel();
   afx_msg void OnBnClickedMoreLess();
   virtual BOOL OnInitDialog();
   };
