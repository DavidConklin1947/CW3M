// PPRunModels.cpp : implementation file
//

#include "stdafx.h"
#include "Envision.h"
#include "PPRunModels.h"
#include "EnvDoc.h"

extern CEnvDoc *gpDoc;


// PPRunModels dialog

IMPLEMENT_DYNAMIC(PPRunModels, CPropertyPage)
PPRunModels::PPRunModels()
	: CPropertyPage(PPRunModels::IDD)
{
}

PPRunModels::~PPRunModels()
{
}

void PPRunModels::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_MODELS, m_models);
}


BEGIN_MESSAGE_MAP(PPRunModels, CPropertyPage)
END_MESSAGE_MAP()


// PPRunModels message handlers

BOOL PPRunModels::OnInitDialog()
   {
   CPropertyPage::OnInitDialog();

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

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
   }



int PPRunModels::SetModelsToUse( void )
   {
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

   return countInUse;
   }
