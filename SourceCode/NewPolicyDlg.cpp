// NewPolicyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Envision.h"
#include "NewPolicyDlg.h"
#include ".\newpolicydlg.h"


// NewPolicyDlg dialog

IMPLEMENT_DYNAMIC(NewPolicyDlg, CDialog)
NewPolicyDlg::NewPolicyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(NewPolicyDlg::IDD, pParent)
   , m_description(_T(""))
   , m_isActorPolicy(FALSE)
   {
}

NewPolicyDlg::~NewPolicyDlg()
{
}

void NewPolicyDlg::DoDataExchange(CDataExchange* pDX)
{
CDialog::DoDataExchange(pDX);
DDX_Text(pDX, IDC_DESCRIPTION, m_description);
DDX_Radio(pDX, IDC_ACTORPOLICY, m_isActorPolicy);
}


BEGIN_MESSAGE_MAP(NewPolicyDlg, CDialog)
END_MESSAGE_MAP()


// NewPolicyDlg message handlers

BOOL NewPolicyDlg::OnInitDialog()
   {
   CDialog::OnInitDialog();

   CheckDlgButton( IDC_ACTORPOLICY, 1 );

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
   }

void NewPolicyDlg::OnOK()
   {
   UpdateData( TRUE );
   CDialog::OnOK();
   }
