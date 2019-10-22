// MultiSandboxDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Envision.h"
#include "MultiSandboxDlg.h"
#include "EnvEngine\EnvModel.h"
#include "EnvEngine\Policy.h"

#include <fmatrix.hpp>


extern PolicyManager *gpPolicyManager;
extern EnvModel      *gpModel;



// MultiSandboxDlg dialog

IMPLEMENT_DYNAMIC(MultiSandboxDlg, CDialog)
MultiSandboxDlg::MultiSandboxDlg(CWnd* pParent /*=NULL*/)
	: CDialog(MultiSandboxDlg::IDD, pParent),
   m_pSandbox( NULL ),
   m_yearsToRun( 20 ),
   m_percentCoverage( 100 )
{
}

MultiSandboxDlg::~MultiSandboxDlg()
   {
   ASSERT( m_pSandbox == NULL );
   }

void MultiSandboxDlg::DoDataExchange(CDataExchange* pDX)
   {
   CDialog::DoDataExchange(pDX);
   DDX_Text(pDX, IDC_YEARSTORUNEDIT, m_yearsToRun);
   DDX_Text(pDX, IDC_PERCENTCOVERAGEEDIT, m_percentCoverage);
   DDX_Control(pDX, IDC_METAGOALS, m_metagoals);
   DDX_Control(pDX, IDC_POLICIES, m_policies);
   }


BEGIN_MESSAGE_MAP(MultiSandboxDlg, CDialog)
END_MESSAGE_MAP()


// SandboxDlg message handlers

BOOL MultiSandboxDlg::OnInitDialog()
   {
   CDialog::OnInitDialog();

   ASSERT( m_pSandbox == NULL );

   // add metagoals to the check list
   int count = EnvModel::GetMetagoalCount();    // only worry about metagoals for policies
   for ( int i=0; i < count; i++ )
      {
      ENV_EVAL_MODEL *pInfo = EnvModel::GetMetagoalModelInfo( i );
      m_metagoals.AddString( pInfo->name );
      m_metagoals.SetCheck( i, 1 );
      }

   // add policies
   count = gpPolicyManager->GetPolicyCount();
   for ( int i=0; i < count; i++ )
      {
      Policy *pPolicy = gpPolicyManager->GetPolicy( i );
      m_policies.AddString( pPolicy->m_name );
      m_policies.SetCheck( i, 1 );
      }

   UpdateData( false );

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
   }

void MultiSandboxDlg::OnOK() // apply
   {
   UpdateData( TRUE );

   CWaitCursor c;
      
   int metagoalCount = EnvModel::GetMetagoalCount();
   ASSERT( metagoalCount == m_metagoals.GetCount() );

   bool *useMetagoalArray = new bool[ metagoalCount ];
   for ( int i=0; i < metagoalCount; i++ )
      useMetagoalArray[ i ] = m_metagoals.GetCheck( i ) ? true : false;

   int policyCount = gpPolicyManager->GetPolicyCount();
   FloatMatrix oldScoreArray( policyCount, metagoalCount, 0.0f );

   //bool initialized = gpModel->m_currentRun > 0 ? true : false;

   // remember old scores
   for ( int i=0; i < policyCount; i++ )
      {
      Policy *pPolicy = gpPolicyManager->GetPolicy( i );
      for ( int j=0; j < metagoalCount; j++ )
         oldScoreArray.Set(i, j, pPolicy->GetGoalScore( j ) );
      }

   for ( int i=0; i < policyCount; i++ )
      {
      if ( m_policies.GetCheck( i ) )
         {
         Policy *pPolicy = gpPolicyManager->GetPolicy( i );
         CString title;
         title.Format( "Evaluating Policy %s (%i of %i)", (LPCSTR) pPolicy->m_name, i+1, policyCount );
         SetWindowText( title );

         if ( m_pSandbox )
            delete m_pSandbox;

         m_pSandbox = new Sandbox( gpModel, m_yearsToRun, float(m_percentCoverage)/100.0f, true );
         //initialized = true;

         m_pSandbox->CalculateGoalScores( pPolicy, useMetagoalArray );

         delete m_pSandbox;
         m_pSandbox = NULL;

         // update labels in checklist box
         m_metagoals.ResetContent();
         for ( int j=0; j < metagoalCount; j++ )
            {
            ENV_EVAL_MODEL *pInfo = EnvModel::GetMetagoalModelInfo( j );
            float effectiveness = pPolicy->GetGoalScore( j );
            CString str;
            if ( useMetagoalArray[ j ] )
               str.Format( "%s: Old Value=%g, New Value=%g", (LPCSTR) pInfo->name, oldScoreArray.Get(i,j), effectiveness );
            else
               str.Format( "%s: Value=%g", (LPCSTR ) pInfo->name, oldScoreArray.Get(i,j) );

            m_metagoals.AddString( str );
            m_metagoals.SetCheck( j, useMetagoalArray[ j ] ? 1 : 0 );
            }
         }  // end of:  if (m_policies.GetCheck( i ))
      }  // end of: for ( i < policyCount )

   int retVal = MessageBox( "New effectiveness scores computed - do you want to keep these scores?", "Accept New Scores?",
                           MB_YESNO );

   switch( retVal )
      {
      case IDYES:
         break;

      case IDNO:
         {
         for ( int i=0; i < policyCount; i++ )
            {
            Policy *pPolicy = gpPolicyManager->GetPolicy( i );
            //for ( int j=0; j < metagoalCount; j++ )
            //   pPolicy->SetGoalScore( j, oldScoreArray.Get(i,j) );
            }
         }
         break;
      }

   SetWindowText( "Done evaluating policies" );
   
   delete useMetagoalArray;
   }


void MultiSandboxDlg::OnCancel()
   {
   CDialog::OnCancel();
   }
