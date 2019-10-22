h// NewIniFileDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "NewIniFileDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// NewIniFileDlg

IMPLEMENT_DYNAMIC(NewIniFileDlg, CPropertySheet)

NewIniFileDlg::NewIniFileDlg(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}


NewIniFileDlg::NewIniFileDlg(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
   {
   AddPage( &m_step1 );
   AddPage( &m_step2 );
//   AddPage( &m_step3 );
   AddPage( &m_finish );

   SetWizardMode();
   }


BEGIN_MESSAGE_MAP(NewIniFileDlg, CPropertySheet)
   ON_COMMAND (ID_APPLY_NOW, OnApplyNow)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// NewIniFileDlg message handlers


void NewIniFileDlg::OnApplyNow() 
   {
	//SaveSetup();
   }
