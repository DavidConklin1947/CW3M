// PPIniMetagoals.cpp : implementation file
//
// PPIniMetagoals.cpp : implementation file
//

#include "stdafx.h"
#include "PPIniMetagoals.h"
#include "IniFileEditor.h"
#include "EnvDoc.h"
#include "EnvEngine\EnvModel.h"
#include "MapLayer.h"

extern CEnvDoc  *gpDoc;
extern MapLayer *gpCellLayer;



// PPIniMetagoals dialog

IMPLEMENT_DYNAMIC(PPIniMetagoals, CTabPageSSL)

PPIniMetagoals::PPIniMetagoals(IniFileEditor *pParent )
	: CTabPageSSL(PPIniMetagoals::IDD, pParent)
   , m_pParent( pParent )
   , m_label(_T(""))
   , m_useSelfInterested(FALSE)
   , m_useAltruism(FALSE)
{ }

PPIniMetagoals::~PPIniMetagoals()
{ }


void PPIniMetagoals::DoDataExchange(CDataExchange* pDX)
{
CTabPageSSL::DoDataExchange(pDX);
DDX_Control(pDX, IDC_METAGOALS, m_metagoals);
DDX_Control(pDX, IDC_MODELS, m_models);
DDX_Text(pDX, IDC_LABEL, m_label);
DDX_Check(pDX, IDC_SELFISH, m_useSelfInterested);
DDX_Check(pDX, IDC_ALTRUISTIC, m_useAltruism);
}


BEGIN_MESSAGE_MAP(PPIniMetagoals, CTabPageSSL)
   ON_BN_CLICKED(IDC_ADD, &PPIniMetagoals::OnBnClickedAdd)
   ON_BN_CLICKED(IDC_REMOVE, &PPIniMetagoals::OnBnClickedRemove)
   ON_BN_CLICKED(IDC_SELFISH, &PPIniMetagoals::MakeDirty)
   ON_BN_CLICKED(IDC_ALTRUISTIC, &PPIniMetagoals::MakeDirty)
   ON_CLBN_CHKCHANGE(IDC_METAGOALS, &PPIniMetagoals::OnLbnChkchangeMetagoals)
   ON_LBN_SELCHANGE(IDC_METAGOALS, &PPIniMetagoals::OnLbnSelchangeMetagoals)
   ON_EN_CHANGE(IDC_LABEL, &PPIniMetagoals::OnEnChangeLabel)
   ON_CBN_SELCHANGE(IDC_MODELS, &PPIniMetagoals::OnCbnSelchangeModel)
END_MESSAGE_MAP()


// PPIniMetagoals message handlers

BOOL PPIniMetagoals::OnInitDialog()
   {
   CTabPageSSL::OnInitDialog();

   // populate models combo
   int count = EnvModel::GetModelCount();
   for ( int i=0; i < count; i++ )
      m_models.AddString( EnvModel::GetModelInfo( i )->name );

   if ( count > 0 )
      m_models.SetCurSel( 0 );
   else
      m_models.SetCurSel( -1 );

   // populate metagoals checklistbox
   count = gpDoc->m_model.GetMetagoalCount();
   m_pInfo = NULL;

   for ( int i=0; i < count; i++ )
      {
      m_pInfo = gpDoc->m_model.GetMetagoalInfo( i );
      m_metagoals.AddString( m_pInfo->name );
      }

   if ( count > 0 )
      m_metagoals.SetCurSel( 0 );
   else
      m_metagoals.SetCurSel( -1 );

   LoadMetagoal();
   
   m_addRemove = false;

   return TRUE;  // return TRUE unless you set the focus to a control
   }


void PPIniMetagoals::LoadMetagoal()
   {
   m_pInfo = NULL;
   int index = m_metagoals.GetCurSel();

   if ( index < 0 )
      return;

   m_pInfo = gpDoc->m_model.GetMetagoalInfo( index );

   m_label = m_pInfo->name;
   m_useSelfInterested = m_pInfo->decisionUse & 1 ? TRUE : FALSE;
   m_useAltruism       = m_pInfo->decisionUse & 2 ? TRUE : FALSE;
      
   if ( m_pInfo->pEvalModel == NULL )
      m_models.SetCurSel( -1 );
   else
      {
      int modelIndex = m_models.FindStringExact( 0, m_pInfo->pEvalModel->name );
      m_models.SetCurSel( modelIndex );
      }

   UpdateData( 0 );
   EnableCtrls();
   }


int PPIniMetagoals::SaveMetagoal()
   {
   if ( m_pInfo == NULL )
      return 1;

   UpdateData( 1 );

   m_pInfo->name = m_label;

   m_pInfo->decisionUse = 0;
   if ( m_useSelfInterested )
      m_pInfo->decisionUse = 1;
   if ( m_useAltruism )
      m_pInfo->decisionUse += 2;

   if ( m_pInfo->decisionUse == 0 )
      {
      MessageBox( "Invalid metagoal - at least one decision use box must be checked");
      MakeDirty();
      return 0;
      }

   if ( m_useAltruism )
      {
      // make sure an eval model is selected
      int modelIndex = m_models.GetCurSel();

      if ( modelIndex < 0 )
         {
         MessageBox( "Invalid Metagoal - a model must be selected for altruistic decision uses" );
         MakeDirty();
         return 0;
         }

      m_pInfo->pEvalModel = EnvModel::GetModelInfo( modelIndex );
      }

   MakeClean();

   return 1;
   }


bool PPIniMetagoals::StoreChanges()
   {
   SaveMetagoal();
   return true;
   }

void PPIniMetagoals::AddEvalModel( LPCTSTR string )
   {
   m_models.AddString( string );
   }


void PPIniMetagoals::RemoveEvalModel( int index )
   {
   m_models.DeleteString( index );

   // if this corrresponds to an altruitic metagoal, remove that metagoal
   ////?????
   }



void PPIniMetagoals::MakeDirty()
   {
   m_isMetagoalDirty = true;
   m_pParent->MakeDirty( INI_METAGOALS );
   }


void PPIniMetagoals::MakeClean()
   {
   m_isMetagoalDirty = false;
   m_pParent->MakeClean( INI_METAGOALS );
   }


void PPIniMetagoals::OnLbnChkchangeMetagoals()
   {
   //MakeDirty();
   }


void PPIniMetagoals::OnBnClickedAdd()
   {
   METAGOAL *pGoal = new METAGOAL;
   pGoal->decisionUse = DU_SELFINTEREST;
   pGoal->pEvalModel = NULL;
   pGoal->name = "New Goal";
   gpDoc->m_model.AddMetagoal( pGoal );

   m_addRemove = true;

   int index = m_metagoals.AddString( "New Goal" );
   m_metagoals.SetCurSel( index );
   LoadMetagoal();
   MakeDirty();
   }


void PPIniMetagoals::OnBnClickedRemove()
   {
   int index = m_metagoals.GetCurSel();

   if ( index < 0 )
      return;

   m_metagoals.DeleteString( index );

   gpDoc->m_model.RemoveMetagoal(index);

   if ( gpDoc->m_model.GetMetagoalCount() > 0 )
      m_metagoals.SetCurSel( 0 );
   else
      m_metagoals.SetCurSel( -1 );

   m_addRemove = true;

   LoadMetagoal();
   MakeDirty();
   }


void PPIniMetagoals::OnEnChangeLabel()
   {
   UpdateData();

   int index = this->m_metagoals.GetCurSel();

   if ( index >= 0 )
      {
      m_metagoals.InsertString( index, m_label );
      m_metagoals.DeleteString( index+1 );
      m_metagoals.SetCurSel( index );

      ENV_EVAL_MODEL *pInfo = EnvModel::GetModelInfo( index );

      if ( pInfo != NULL )
         pInfo->name = m_label;
   
      MakeDirty();
      }
   }


void PPIniMetagoals::OnLbnSelchangeMetagoals()
   {
   if( m_isMetagoalDirty )
      SaveMetagoal();

   LoadMetagoal();
   }



void PPIniMetagoals::OnCbnSelchangeModel()
   {
   int modelIndex = m_models.GetCurSel();

   if ( modelIndex < 0 )
      m_pInfo->pEvalModel = NULL;
   else
      m_pInfo->pEvalModel = EnvModel::GetModelInfo( modelIndex );
   
   MakeDirty();
   }


void PPIniMetagoals::EnableCtrls( void )
   {
   UpdateData( 1 );
   int modelCount = m_models.GetCount();
   int metagoalCount = m_models.GetCount();

   GetDlgItem( IDC_SELFISH  )->EnableWindow( metagoalCount > 0 ? TRUE : FALSE );
   GetDlgItem( IDC_ALTRUISTIC )->EnableWindow( modelCount > 0 && metagoalCount > 0 ? TRUE : FALSE );

   GetDlgItem( IDC_STATICMODEL )->EnableWindow( m_useAltruism );
   GetDlgItem( IDC_MODELS )->EnableWindow( m_useAltruism );   
   }