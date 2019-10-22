// SandboxDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Envision.h"
#include "EnvEngine\EnvModel.h"
#include ".\sandboxdlg.h"

extern EnvModel *gpModel;

// SandboxDlg dialog

IMPLEMENT_DYNAMIC(SandboxDlg, CDialog)
SandboxDlg::SandboxDlg( Policy *pPolicy, CWnd* pParent /*=NULL*/ )
:  CDialog(SandboxDlg::IDD, pParent),
   m_pPolicy( pPolicy ),
   m_pSandbox( NULL ),
   m_yearsToRun( 20 ),
   m_percentCoverage( 100 ),
   m_runCompleted( false ),
   m_useMetagoalArray( NULL )
   {
   }

SandboxDlg::~SandboxDlg()
   {
   ASSERT( m_pSandbox == NULL );

   if ( m_useMetagoalArray != NULL )
      delete [] m_useMetagoalArray;
   }

void SandboxDlg::DoDataExchange(CDataExchange* pDX)
   {
   CDialog::DoDataExchange(pDX);
   DDX_Text(pDX, IDC_YEARSTORUNEDIT, m_yearsToRun);
   DDX_Text(pDX, IDC_PERCENTCOVERAGEEDIT, m_percentCoverage);
   DDX_Control(pDX, IDC_METAGOALS, m_metagoals);
   }


BEGIN_MESSAGE_MAP(SandboxDlg, CDialog)
END_MESSAGE_MAP()


// SandboxDlg message handlers

BOOL SandboxDlg::OnInitDialog()
   {
   CDialog::OnInitDialog();

   ASSERT( m_pSandbox == NULL );

   SetWindowText( m_pPolicy->m_name );

   // add metagoals to the check list
   int count = EnvModel::GetMetagoalCount();    // only worry about metagoals for policies
   for ( int i=0; i < count; i++ )
      {
      ENV_EVAL_MODEL *pInfo = EnvModel::GetMetagoalModelInfo( i );
      float effectiveness = m_pPolicy->GetGoalScore( i );
      CString str;
      str.Format( "%s (current value: %g)", (LPCSTR) pInfo->name, effectiveness );
      m_metagoals.AddString( str );
      m_metagoals.SetCheck( i, 1 );
      }

   UpdateData( false );

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
   }

BOOL SandboxDlg::DestroyWindow()
   {
   return CDialog::DestroyWindow();
   }

void SandboxDlg::OnOK() // apply
   {
   if ( ! m_runCompleted )
      {
      UpdateData( TRUE );

      CWaitCursor c;
      
      if ( gpModel->m_currentRun > 0 ) // assume that this means the init run has been called for the models
         m_pSandbox = new Sandbox( gpModel, m_yearsToRun, float(m_percentCoverage)/100.0f, false );
      else
         m_pSandbox = new Sandbox( gpModel, m_yearsToRun, float(m_percentCoverage)/100.0f, true );
      
      int metagoalCount = EnvModel::GetMetagoalCount();
      ASSERT( metagoalCount == m_metagoals.GetCount() );

      ASSERT( m_useMetagoalArray == NULL );
      m_useMetagoalArray = new bool[ metagoalCount ];
      for ( int i=0; i < metagoalCount; i++ )
         m_useMetagoalArray[ i ] = m_metagoals.GetCheck( i ) ? true : false;

      m_pSandbox->CalculateGoalScores( m_pPolicy, m_useMetagoalArray );

      // update labels in checklist box
      m_metagoals.ResetContent();
      for ( int i=0; i < metagoalCount; i++ )
         {
         ENV_EVAL_MODEL *pInfo = EnvModel::GetMetagoalModelInfo( i );
         float effectiveness = m_pPolicy->GetGoalScore( i );
         CString str;
         str.Format( "%s (current value: %g)", (LPCSTR) pInfo->name, effectiveness );
         m_metagoals.AddString( str );
         m_metagoals.SetCheck( i, m_useMetagoalArray[ i ] ? 1 : 0 );
         }

      SetDlgItemText( IDOK, "&Apply" );
      MessageBox( "New effectiveness scores computed - select <Apply> to keep these scores", MB_OK );
      m_runCompleted = true;

      delete m_pSandbox;
      m_pSandbox = NULL;
      }
   else  // runCompleted = true
      {
      CDialog::OnOK();
      }
   }

void SandboxDlg::OnCancel()
   {
   CDialog::OnCancel();
   }
