// PPBasics.cpp : implementation file
//

#include "stdafx.h"
#include "Flow.h"
#include "PPBasics.h"
#include "afxdialogex.h"


// PPBasics dialog

IMPLEMENT_DYNAMIC(PPBasics, CPropertyPage)

PPBasics::PPBasics( FlowModel *pFlow )
	: CPropertyPage(PPBasics::IDD)
   , m_pFlow( pFlow )
{

}

PPBasics::~PPBasics()
{
}

void PPBasics::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(PPBasics, CPropertyPage)
   ON_EN_CHANGE(IDC_INPUTPATH, &PPBasics::OnEnChangeInputpath)
END_MESSAGE_MAP()


// PPBasics message handlers


void PPBasics::OnEnChangeInputpath()
   {
   // TODO:  If this is a RICHEDIT control, the control will not
   // send this notification unless you override the CPropertyPage::OnInitDialog()
   // function and call CRichEditCtrl().SetEventMask()
   // with the ENM_CHANGE flag ORed into the mask.

   // TODO:  Add your control notification handler code here
   }


BOOL PPBasics::OnInitDialog()
   {
   CPropertyPage::OnInitDialog();

   // TODO:  Add extra initialization here

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
   }
