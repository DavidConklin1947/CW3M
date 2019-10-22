// PPIniModels.cpp : implementation file
//

#include "stdafx.h"
#include "PPIniModels.h"
#include "IniFileEditor.h"
#include "EnvDoc.h"
#include "EnvEngine\EnvModel.h"
#include "EnvEngine\EnvConstants.h"
#include "EnvEngine\EnvLoader.h"

#include <MapLayer.h>
#include <DirPlaceholder.h>
#include <PathManager.h>


extern CEnvDoc  *gpDoc;
extern MapLayer *gpCellLayer;



// PPIniModels dialog

IMPLEMENT_DYNAMIC(PPIniModels, CTabPageSSL)

PPIniModels::PPIniModels(IniFileEditor *pParent )
	: CTabPageSSL(PPIniModels::IDD, pParent)
   , m_pParent( pParent )
   , m_label(_T(""))
   , m_path(_T(""))
   , m_id(0)
   //, m_useSelfInterested(FALSE)
   //, m_useAltruism(FALSE)
   , m_showInResults(FALSE)
   , m_showUnscaled(FALSE)
   , m_useCol(FALSE)
   , m_field(_T(""))
   , m_initFnInfo(_T(""))
   , m_cdf(FALSE)
   , m_minRaw(0)
   , m_maxRaw(0)
   , m_rawUnits(_T(""))
{ }

PPIniModels::~PPIniModels()
{ }


void PPIniModels::DoDataExchange(CDataExchange* pDX)
{
CTabPageSSL::DoDataExchange(pDX);
DDX_Control(pDX, IDC_MODELS, m_models);
DDX_Text(pDX, IDC_LABEL, m_label);
DDX_Text(pDX, IDC_PATH, m_path);
DDX_Text(pDX, IDC_ID, m_id);
//DDX_Check(pDX, IDC_SELFISH, m_useSelfInterested);
//DDX_Check(pDX, IDC_ALTRUISTIC, m_useAltruism);
DDX_Check(pDX, IDC_SHOWINRESULTS, m_showInResults);
DDX_Check(pDX, IDC_SHOWUNSCALED, m_showUnscaled);
DDX_Check(pDX, IDC_USECOL, m_useCol);
DDX_Text(pDX, IDC_FIELD, m_field);
DDX_Text(pDX, IDC_INITSTR, m_initFnInfo);
DDX_Check(pDX, IDC_CDF, m_cdf);
DDX_Text(pDX, IDC_MIN, m_minRaw);
DDX_Text(pDX, IDC_MAX, m_maxRaw);
DDX_Text(pDX, IDC_UNITS, m_rawUnits);
}


BEGIN_MESSAGE_MAP(PPIniModels, CTabPageSSL)
   ON_BN_CLICKED(IDC_BROWSE, &PPIniModels::OnBnClickedBrowse)
   ON_BN_CLICKED(IDC_ADD, &PPIniModels::OnBnClickedAdd)
   ON_BN_CLICKED(IDC_REMOVE, &PPIniModels::OnBnClickedRemove)
   ON_BN_CLICKED(IDC_USECOL, &PPIniModels::OnBnClickedUsecol)

//   ON_BN_CLICKED(IDC_SELFISH, &PPIniModels::MakeDirty)
//   ON_BN_CLICKED(IDC_ALTRUISTIC, &PPIniModels::MakeDirty)
   ON_BN_CLICKED(IDC_SHOWINRESULTS, &PPIniModels::MakeDirty)
   ON_BN_CLICKED(IDC_SHOWUNSCALED, &PPIniModels::MakeDirty)

   ON_BN_CLICKED(IDC_CDF, &PPIniModels::OnBnClickedCdf)

   ON_CLBN_CHKCHANGE(IDC_MODELS, &PPIniModels::OnLbnChkchangeModels)
   ON_LBN_SELCHANGE(IDC_MODELS, &PPIniModels::OnLbnSelchangeModels)

   ON_EN_CHANGE(IDC_LABEL, &PPIniModels::OnEnChangeLabel)
   ON_EN_CHANGE(IDC_PATH, &PPIniModels::MakeDirty)
   ON_EN_CHANGE(IDC_ID, &PPIniModels::MakeDirty)
   ON_EN_CHANGE(IDC_FIELD, &PPIniModels::MakeDirty)
   ON_EN_CHANGE(IDC_INITSTR, &PPIniModels::MakeDirty)
   ON_EN_CHANGE(IDC_MIN, &PPIniModels::MakeDirty)
   ON_EN_CHANGE(IDC_MAX, &PPIniModels::MakeDirty)
   ON_EN_CHANGE(IDC_UNITS, &PPIniModels::MakeDirty)
END_MESSAGE_MAP()


// PPIniModels message handlers

BOOL PPIniModels::OnInitDialog()
   {
   CTabPageSSL::OnInitDialog();

   int count = gpDoc->m_model.GetModelCount();
   m_pInfo = NULL;

   for ( int i=0; i < count; i++ )
      {
      m_pInfo = gpDoc->m_model.GetModelInfo( i );
      m_models.AddString( m_pInfo->name );

      m_models.SetCheck( i, m_pInfo->use ? 1 : 0 );
      }

   if ( count > 0 )
      m_models.SetCurSel( 0 );
   else
      m_models.SetCurSel( -1 );

   // set up trandfer function mapping window
   RECT rect;
   GetDlgItem( IDC_PLACEHOLDER )->GetWindowRect( &rect );  // in screen coords
   GetDlgItem( IDC_PLACEHOLDER )->ShowWindow( SW_HIDE );
   ScreenToClient( &rect );
   m_spline.Create( NULL, "", WS_CHILD | WS_BORDER | WS_VISIBLE, rect, this, 10000 );

   int width  = rect.right - rect.left;
   int height = rect.bottom - rect.top;

   // first point
   m_spline.AddControlPoint( CPoint( 0, height ) );

   // intermediate points
   for ( int i=0; i < 3; i++ )
      m_spline.AddControlPoint( CPoint( (i+1)*width/4, height-(i+1)*height/4 ) );

   m_spline.AddControlPoint( CPoint( width, 0 ) );
   m_spline.AllowAddPoints( false );

   LoadModel();

   return TRUE;  // return TRUE unless you set the focus to a control
   }


void PPIniModels::LoadModel()
   {
   m_pInfo = NULL;
   int index = m_models.GetCurSel();

   if ( index < 0 )
      return;

   m_pInfo = gpDoc->m_model.GetModelInfo( index );

   m_label = m_pInfo->name;
   m_path  = m_pInfo->path;
   m_id    = m_pInfo->id;

   m_useCol  = m_pInfo->col >= 0 ? TRUE : FALSE;
   m_field   = m_pInfo->fieldName;
   m_initFnInfo = m_pInfo->initInfo;

   //m_useSelfInterested = m_pInfo->decisionUse   & 1 ? TRUE : FALSE;
   //m_useAltruism       = m_pInfo->decisionUse   & 2 ? TRUE : FALSE;
   m_showInResults     = m_pInfo->showInResults & 1 ? TRUE : FALSE;
   m_showUnscaled      = m_pInfo->showInResults & 2 ? TRUE : FALSE;

   //m_cdf(FALSE)
   m_minRaw = m_pInfo->rawScoreMin;
   m_maxRaw = m_pInfo->rawScoreMax;
   m_rawUnits = m_pInfo->rawScoreUnits;
      
   UpdateData( 0 );

   m_models.SetCheck( index, m_pInfo->use ? 1 : 0 );
   }


int PPIniModels::SaveModel()
   {
   m_isModelDirty = false;

   if ( m_pInfo == NULL )
      return 1;

   int index = m_models.GetCurSel();

   UpdateData( 1 );

   m_pInfo->name = m_label;
   m_pInfo->path = m_path;
   m_pInfo->id   = m_id;
   m_pInfo->use  = m_models.GetCheck( index ) ? true : false;

   if ( m_useCol )
      {
      m_pInfo->col = gpCellLayer->GetFieldCol( m_field );
      m_pInfo->fieldName = m_field;
      }
   else
      {
      m_pInfo->col = -1;
      m_pInfo->fieldName.Empty();
      }
   
   m_pInfo->initInfo = m_initFnInfo;

   //m_pInfo->decisionUse = 0;
   //if ( m_useSelfInterested )
   //   m_pInfo->decisionUse = 1;
   //if ( m_useAltruism )
   //   m_pInfo->decisionUse += 2;

   m_pInfo->showInResults = 0;
   if ( m_showInResults )
      m_pInfo->showInResults = 1;
   if ( m_showUnscaled )
      m_pInfo->showInResults += 2;
   

   //m_cdf(FALSE)
   m_pInfo->rawScoreMin = m_minRaw;
   m_pInfo->rawScoreMax = m_maxRaw;
   m_pInfo->rawScoreUnits = m_rawUnits;   

   return 1;
   }


bool PPIniModels::StoreChanges()
   {
   SaveModel();
   return true;
   }


void PPIniModels::MakeDirty()
   {
   m_isModelDirty = true;
   m_pParent->MakeDirty( INI_MODELS );
   }



void PPIniModels::OnLbnChkchangeModels()
   {
   MakeDirty();
   }



void PPIniModels::OnBnClickedBrowse()
   {
   DirPlaceholder d;

   TCHAR _path[ MAX_PATH ];
   GetModuleFileName( NULL, _path, MAX_PATH );

   for ( int i=0; i < lstrlen( _path ); i++ )
      _path[ i ] = tolower( _path[ i ] );

   TCHAR *end = _tcsstr( _path, _T("envision") );
   if ( end != NULL )
      {
      end += 8;
      *end = NULL;

      _chdir( _path );
      }

   CFileDialog dlg( TRUE, "dll", "", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "DLL files|*.dll|All files|*.*||");

   if ( dlg.DoModal() == IDOK )
      {
      m_path = dlg.GetPathName();

      UpdateData( 0 );
      MakeDirty();
      }
   }


void PPIniModels::OnBnClickedAdd()
   {
   DirPlaceholder d;

   CString _path = PathManager::GetPath( PM_ENVISION_DIR );

   //char *cwd = _getcwd( NULL, 0 );
   //TCHAR _path[ MAX_PATH ];
   //GetModuleFileName( NULL, _path, MAX_PATH );
   //
   //for ( int i=0; i < lstrlen( _path ); i++ )
   //   _path[ i ] = tolower( _path[ i ] );
   //
   //TCHAR *end = _tcsstr( _path, _T("envision") );
   //if ( end != NULL )
   //   {
   //   end += 8;
   //   *end = NULL;
   //
   //   _chdir( _path );
   //   }
   
   CFileDialog dlg( TRUE, "dll", _path, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "DLL files|*.dll|All files|*.*||");

   if ( dlg.DoModal() == IDOK )
      {
      CString path = dlg.GetPathName();
      int index = m_models.AddString( path );
      m_models.SetCurSel( index );

      EnvLoader loader;
      loader.LoadModel( &gpDoc->m_model, path, path, 0, 1, 0, 0, NULL, NULL, NULL, 0, 1, 0 );

      LoadModel();
      MakeDirty();

      // update metagoals
      PPIniMetagoals *pMetagoal = &(m_pParent->m_metagoals);
      pMetagoal->AddEvalModel( path );
      }
   }


void PPIniModels::OnBnClickedRemove()
   {
   int index = m_models.GetCurSel();

   if ( index < 0 )
      return;

   m_models.DeleteString( index );

   gpDoc->m_model.RemoveModel(index);

   // update metagoals
   PPIniMetagoals *pMetagoal = &(m_pParent->m_metagoals);
   pMetagoal->RemoveEvalModel( index );

   if ( gpDoc->m_model.GetModelCount() > 0 )
      m_models.SetCurSel( 0 );
   else
      m_models.SetCurSel( -1 );

   LoadModel();
   MakeDirty();
   }


void PPIniModels::OnBnClickedUsecol()
   {
   // TODO: Add your control notification handler code here
   }


void PPIniModels::OnBnClickedCdf()
   {
   // TODO: Add your control notification handler code here
   }


void PPIniModels::OnEnChangeLabel()
   {
   UpdateData();

   int index = this->m_models.GetCurSel();

   if ( index >= 0 )
      {
      m_models.InsertString( index, m_label );
      m_models.DeleteString( index+1 );
      m_models.SetCurSel( index );

      ENV_EVAL_MODEL *pInfo = EnvModel::GetModelInfo( index );

      if ( pInfo != NULL )
         pInfo->name = m_label;
   
      MakeDirty();
      }
   }


void PPIniModels::OnLbnSelchangeModels()
   {
   if( m_isModelDirty )
      SaveModel();

   LoadModel();
   }
