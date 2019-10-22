// PPRunPolicies.cpp : implementation file
//

#include "stdafx.h"
#include "Envision.h"
#include "EnvEngine\Policy.h"
#include ".\pprunpolicies.h"
#include "EnvDoc.h"

extern CEnvDoc       *gpDoc;
extern PolicyManager *gpPolicyManager;


// PPRunPolicies dialog

IMPLEMENT_DYNAMIC(PPRunPolicies, CPropertyPage)
PPRunPolicies::PPRunPolicies()
	: CPropertyPage(PPRunPolicies::IDD)
{
}

PPRunPolicies::~PPRunPolicies()
{
}

void PPRunPolicies::DoDataExchange(CDataExchange* pDX)
{
   CPropertyPage::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_POLICIES, m_policies);
}


BEGIN_MESSAGE_MAP(PPRunPolicies, CPropertyPage)
   ON_BN_CLICKED(IDC_SELECTALL, OnBnClickedSelectall)
   ON_BN_CLICKED(IDC_SELECTNONE, OnBnClickedSelectnone)
   ON_BN_CLICKED(IDC_SHOWMYPOLICIES, OnBnClickedShowmypolicies)
   ON_BN_CLICKED(IDC_SHOWSHAREDPOLICIES, OnBnClickedShowsharedpolicies)
END_MESSAGE_MAP()


// PPRunPolicies message handlers

BOOL PPRunPolicies::OnInitDialog()
   {
   CPropertyPage::OnInitDialog();

   CheckDlgButton( IDC_SHOWMYPOLICIES, 1 );
   CheckDlgButton( IDC_SHOWSHAREDPOLICIES, 1 );
   LoadPolicies();

   return TRUE;
   }


void PPRunPolicies::LoadPolicies()
   {
   m_policies.ResetContent();

   int count = gpPolicyManager->GetPolicyCount();
   for ( int i=0; i < count; i++ )
      {
      Policy *pPolicy = gpPolicyManager->GetPolicy( i );
      
      // only show if its an shared policy or one of my policies
      if ( ShowPolicy( pPolicy ) )
         {
         int index = m_policies.AddString( pPolicy->m_name );
         m_policies.SetItemData( index, (DWORD_PTR) pPolicy );

         if ( pPolicy->m_use )
            m_policies.SetCheck( index, TRUE );
         else
            m_policies.SetCheck( index, FALSE );
         }
      }

   m_policies.SetCurSel( 0 );

   return;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
   }


int PPRunPolicies::SetPoliciesToUse( void )
   {
   int count = m_policies.GetCount();

   int countInUse = 0;

   for ( int i=0; i < count; i++ )
      {
      Policy *pPolicy = (Policy*) m_policies.GetItemData( i );
      if ( m_policies.GetCheck( i ) )
         {
         pPolicy->m_use = true;
         countInUse++;
         }
      else
         pPolicy->m_use = false;        
      }

   return countInUse;
   }

void PPRunPolicies::OnBnClickedSelectall()
   {
   int count = m_policies.GetCount();

   for ( int i=0; i<count; i++ )
      m_policies.SetCheck( i, TRUE );
   }

void PPRunPolicies::OnBnClickedSelectnone()
   {
   int count = m_policies.GetCount();

   for ( int i=0; i<count; i++ )
      m_policies.SetCheck( i, FALSE );
   }


bool PPRunPolicies::ShowPolicy( Policy *pPolicy )
   {
   bool showMyPolicies     = IsDlgButtonChecked( IDC_SHOWMYPOLICIES ) ? true : false;
   bool showSharedPolicies = IsDlgButtonChecked( IDC_SHOWSHAREDPOLICIES ) ? true : false;
   bool showPolicy = false;

   if ( pPolicy->m_isShared && showSharedPolicies ) 
      showPolicy = true;
   if ( pPolicy->m_originator.CompareNoCase( gpDoc->m_userName ) == 0 && showMyPolicies )
       showPolicy = true;

   return showPolicy;
   }


void PPRunPolicies::OnBnClickedShowmypolicies()
   {
   LoadPolicies();
   }


void PPRunPolicies::OnBnClickedShowsharedpolicies()
   {
   LoadPolicies();
   }
