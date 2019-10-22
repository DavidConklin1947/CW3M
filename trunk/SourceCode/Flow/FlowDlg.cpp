// FlowDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Flow.h"
#include "FlowDlg.h"

#include "PPBasics.h"



// FlowDlg

IMPLEMENT_DYNAMIC(FlowDlg, CTreePropSheetEx)


FlowDlg::FlowDlg(FlowModel *pFlow, LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CTreePropSheetEx(pszCaption, pParentWnd, iSelectPage)
   , m_basicPage( pFlow )
   , m_outputsPage( pFlow )
   {
   SetTreeViewMode(TRUE, FALSE, FALSE ); 
   
   AddPage( &m_basicPage );
   AddPage( &m_outputsPage );
   }

FlowDlg::~FlowDlg()
{
}


BEGIN_MESSAGE_MAP(FlowDlg, CTreePropSheetEx)
END_MESSAGE_MAP()


// FlowDlg message handlers


//BOOL FlowDlg::OnInitDialog()
//   {
//   BOOL bResult = CTreePropSheetEx::OnInitDialog();
//
//   return bResult;
//   }
//