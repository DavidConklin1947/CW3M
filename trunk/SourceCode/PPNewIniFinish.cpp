// PPNewIniFinish.cpp : implementation file
//

#include "stdafx.h"
#include "Envision.h"
#include "PPNewIniFinish.h"
#include "NewIniFileDlg.h"

#include <Path.h>

// PPNewIniFinish dialog

IMPLEMENT_DYNAMIC(PPNewIniFinish, CPropertyPage)

PPNewIniFinish::PPNewIniFinish(CWnd* pParent /*=NULL*/)
	: CPropertyPage(PPNewIniFinish::IDD)
   , m_runIniEditor(FALSE)
   {

}

PPNewIniFinish::~PPNewIniFinish()
{
}

void PPNewIniFinish::DoDataExchange(CDataExchange* pDX)
{
CPropertyPage::DoDataExchange(pDX);
DDX_Check(pDX, IDC_RUNINIEDITOR, m_runIniEditor);
   }


BEGIN_MESSAGE_MAP(PPNewIniFinish, CPropertyPage)
END_MESSAGE_MAP()


// PPNewIniFinish message handlers
BOOL PPNewIniFinish::OnSetActive()
   {
   CPropertySheet* psheet = (CPropertySheet*) GetParent();   
   psheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH);

   NewIniFileDlg *pParent = (NewIniFileDlg*) GetParent();

   CString msg( _T(" A project file will be created at " ) );
   msg +=  pParent->m_step1.m_pathIni;

   if ( pParent->m_step1.m_pathIni.Right( 1 ).CompareNoCase("\\" ) != 0 &&  pParent->m_step1.m_pathIni.Right( 1 ).CompareNoCase("/" ) != 0  )
      msg += "\\";
   msg += pParent->m_step1.m_projectName;
   msg += ".envx";

   GetDlgItem( IDC_FILES )->SetWindowText( msg );

   return CPropertyPage::OnSetActive();
   }