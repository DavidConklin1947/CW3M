// PPPolicyScenarios.cpp : implementation file
//

#include "stdafx.h"

#include ".\pppolicyscenarios.h"

#include "EnvEngine\Policy.h"
#include "EnvEngine\Scenario.h"
#include "PolEditor.h"
#include "outcomeEditor.h"

extern PolicyManager   *gpPolicyManager;
extern ScenarioManager *gpScenarioManager;


// PPPolicyScenarios dialog

IMPLEMENT_DYNAMIC(PPPolicyScenarios, CTabPageSSL)

PPPolicyScenarios::PPPolicyScenarios( PolEditor *pParent, Policy *&pPolicy )
	: CTabPageSSL()
   , m_pPolicy( pPolicy )
   , m_pParent( pParent )
{ }

PPPolicyScenarios::~PPPolicyScenarios()
{ }


void PPPolicyScenarios::DoDataExchange(CDataExchange* pDX)
{
CTabPageSSL::DoDataExchange(pDX);
DDX_Control(pDX, IDC_SCENARIOS, m_scenarios);
}


BEGIN_MESSAGE_MAP(PPPolicyScenarios, CTabPageSSL)
   ON_CLBN_CHKCHANGE(IDC_SCENARIOS, &PPPolicyScenarios::OnCheckScenario)   
END_MESSAGE_MAP()


// PPPolicyScenarios message handlers


BOOL PPPolicyScenarios::OnInitDialog()
   {
   CTabPageSSL::OnInitDialog();

   m_scenarios.ResetContent();

   for ( int i=0; i < (int) gpScenarioManager->GetCount(); i++ )
      m_scenarios.AddString( gpScenarioManager->GetScenario( i )->m_name );

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
   }


void PPPolicyScenarios::LoadPolicy()
   {
   if ( this->m_pPolicy == NULL )
      return;

   for ( int i=0; i < (int) gpScenarioManager->GetCount(); i++ )
      {
      Scenario *pScenario =  gpScenarioManager->GetScenario( i );

      BOOL use = pScenario->GetPolicyInfo( m_pPolicy->m_name )->inUse ? 1 : 0;

      m_scenarios.SetCheck( i, use );
      }     
   }


// make sure any changes are stored in the policy
bool PPPolicyScenarios::StoreChanges()
   {
   if ( m_pParent->IsDirty( DIRTY_SCENARIOS ) )
      {      
      for ( int i=0; i < (int) gpScenarioManager->GetCount(); i++ )
         {
         Scenario *pScenario =  gpScenarioManager->GetScenario( i );

         BOOL use = m_scenarios.GetCheck( i );

         pScenario->GetPolicyInfo( m_pPolicy->m_name )->inUse = use ? true : false;
         }
      }

   m_pParent->MakeClean( DIRTY_SCENARIOS );   

   return true;
   }

void PPPolicyScenarios::OnCheckScenario()
   {
   m_pParent->MakeDirty( DIRTY_SCENARIOS );
   }