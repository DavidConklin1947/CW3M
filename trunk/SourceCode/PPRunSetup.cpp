// PPRunSetup.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "PPRunSetup.h"
#include "EnvDoc.h"

extern CEnvDoc *gpDoc;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// PPRunSetup property page

IMPLEMENT_DYNCREATE(PPRunSetup, CPropertyPage)

PPRunSetup::PPRunSetup() : CPropertyPage(PPRunSetup::IDD)
//, m_culturalFrequency(0)
//, m_policyFrequency(0)
//, m_evalFrequency(0)
   {
   }

PPRunSetup::~PPRunSetup()
{
}

void PPRunSetup::DoDataExchange(CDataExchange* pDX)
{
//CPropertyPage::DoDataExchange(pDX);
//DDX_Text(pDX, IDC_CULTURALFREQUENCY, m_culturalFrequency);
//DDV_MinMaxInt(pDX, m_culturalFrequency, 0, 100);
//DDX_Text(pDX, IDC_POLICYFREQUENCY, m_policyFrequency);
//DDV_MinMaxInt(pDX, m_policyFrequency, 0, 100);
//DDX_Text(pDX, IDC_EVALFREQUENCY, m_evalFrequency);
//DDV_MinMaxInt(pDX, m_evalFrequency, 0, 100);
DDX_Control(pDX, IDC_MODELS, m_models);
DDX_Control(pDX, IDC_APS, m_aps);
}


BEGIN_MESSAGE_MAP(PPRunSetup, CPropertyPage)
   //ON_BN_CLICKED(IDC_ENABLECULTURALMETAPROCESS, OnBnClicked)
   //ON_BN_CLICKED(IDC_ENABLEPOLICYMETAPROCESS, OnBnClicked)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PPRunSetup message handlers

BOOL PPRunSetup::OnInitDialog() 
   {
	CPropertyPage::OnInitDialog();
/*
   if ( m_culturalEnabled && m_culturalFrequency > 0 )
      CheckDlgButton( IDC_ENABLECULTURALMETAPROCESS, BST_CHECKED );
   else
      CheckDlgButton( IDC_ENABLECULTURALMETAPROCESS, BST_UNCHECKED );

   if ( m_policyEnabled && m_policyFrequency > 0 )
      CheckDlgButton( IDC_ENABLEPOLICYMETAPROCESS, BST_CHECKED );
   else
      CheckDlgButton( IDC_ENABLEPOLICYMETAPROCESS, BST_UNCHECKED );
*/
   // models
   m_models.ResetContent();
   int count = EnvModel::GetModelCount();
   for ( int i=0; i < count; i++ )
      {
      ENV_EVAL_MODEL *pModel = EnvModel::GetModelInfo( i );

      m_models.AddString( pModel->name );

      if ( pModel->use )
         m_models.SetCheck( i, TRUE );
      else
         m_models.SetCheck( i, FALSE );
      }

   m_models.SetCurSel( 0 );

   // autonomous processes
   m_aps.ResetContent();
   count = EnvModel::GetAutonomousProcessCount();
   for ( int i=0; i < count; i++ )
      {
      ENV_AUTO_PROCESS *pAP = EnvModel::GetAutonomousProcessInfo( i );

      m_aps.AddString( pAP->name );

      if ( pAP->use )
         m_aps.SetCheck( i, TRUE );
      else
         m_aps.SetCheck( i, FALSE );
      }

   m_aps.SetCurSel( 0 );

   //CSpinButtonCtrl *pSpin = (CSpinButtonCtrl*) GetDlgItem( IDC_EVALFREQSPIN );
   //pSpin->SetRange( 0, 100 );

   //EnableControls();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
   }
/*
void PPRunSetup::EnableControls(void)
   {
   m_culturalEnabled = FALSE;
   m_policyEnabled   = FALSE;

   if ( IsDlgButtonChecked( IDC_ENABLECULTURALMETAPROCESS ) )
      m_culturalEnabled = TRUE;
   
   GetDlgItem( IDC_CULTURALFREQUENCY )->EnableWindow( m_culturalEnabled );
   GetDlgItem( IDC_CULTURALFREQUENCY_LABEL )->EnableWindow( m_culturalEnabled );

   if ( IsDlgButtonChecked( IDC_ENABLEPOLICYMETAPROCESS ) )
      m_policyEnabled = TRUE;
   
   
   GetDlgItem( IDC_POLICYFREQUENCY )->EnableWindow( m_policyEnabled );
   GetDlgItem( IDC_POLICYFREQUENCY_LABEL )->EnableWindow( m_policyEnabled );
   }


void PPRunSetup::OnBnClicked()
   {
   EnableControls();
   }
*/

int PPRunSetup::SetModelsToUse( void )
   {
   // evaluative models
   int count = m_models.GetCount();
   ASSERT( count == EnvModel::GetModelCount() );

   int countInUse = 0;

   for ( int i=0; i < count; i++ )
      {
      ENV_EVAL_MODEL *pModel = EnvModel::GetModelInfo( i );

      if ( m_models.GetCheck( i ) )
         {
         pModel->use = true;
         countInUse++;
         }
      else
         pModel->use = false;
      }

   count = m_aps.GetCount();
   ASSERT( count == EnvModel::GetAutonomousProcessCount() );

   for ( int i=0; i < count; i++ )
      {
      ENV_AUTO_PROCESS *pAP = EnvModel::GetAutonomousProcessInfo( i );

      if ( m_aps.GetCheck( i ) )
         {
         pAP->use = true;
         countInUse++;
         }
      else
         pAP->use = false;
      }

   return countInUse;
   }
