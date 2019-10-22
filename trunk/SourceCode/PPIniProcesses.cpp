// PPIniProcesses.cpp : implementation file
//

#include "stdafx.h"
#include "PPIniProcesses.h"

#include "IniFileEditor.h"
#include "EnvDoc.h"
#include "EnvEngine\EnvConstants.h"
#include "EnvEngine\EnvLoader.h"
#include <DirPlaceholder.h>
#include <PathManager.h>
#include <maplayer.h>

extern CEnvDoc   *gpDoc;


// PPIniProcesses dialog

IMPLEMENT_DYNAMIC(PPIniProcesses, CTabPageSSL)

PPIniProcesses::PPIniProcesses(IniFileEditor *pParent /*=NULL*/)
	: CTabPageSSL(PPIniProcesses::IDD, pParent)
   , m_pParent( pParent )
   , m_label(_T(""))
   , m_path(_T(""))
   , m_timing(FALSE)
   , m_ID(0)
   , m_useCol(FALSE)
   , m_fieldName(_T(""))
   , m_initStr(_T(""))
   , m_isProcessDirty( false )
   , m_isProcessesDirty( false )
{ }

PPIniProcesses::~PPIniProcesses()
{}

void PPIniProcesses::DoDataExchange(CDataExchange* pDX)
{
CTabPageSSL::DoDataExchange(pDX);
DDX_Control(pDX, IDC_APS, m_aps);
DDX_Text(pDX, IDC_LABEL, m_label);
DDX_Text(pDX, IDC_PATH, m_path);
DDX_Radio(pDX, IDC_PREYEAR, m_timing);
DDX_Text(pDX, IDC_ID, m_ID);
DDX_Check(pDX, IDC_USECOL, m_useCol);
DDX_Text(pDX, IDC_FIELD, m_fieldName);
DDX_Text(pDX, IDC_INITINFO, m_initStr);
}


BEGIN_MESSAGE_MAP(PPIniProcesses, CTabPageSSL)
   ON_BN_CLICKED(IDC_BROWSEAPS, &PPIniProcesses::OnBnClickedBrowse)
   ON_BN_CLICKED(IDC_ADDAPS, &PPIniProcesses::OnBnClickedAdd)
   ON_BN_CLICKED(IDC_REMOVEAPS, &PPIniProcesses::OnBnClickedRemove)
   ON_LBN_SELCHANGE(IDC_APS, &PPIniProcesses::OnLbnSelchangeAps)
   ON_CLBN_CHKCHANGE(IDC_APS, &PPIniProcesses::OnLbnChkchangeAps)
   ON_WM_ACTIVATE()
   ON_EN_CHANGE(IDC_LABEL, &PPIniProcesses::OnEnChangeLabel)
   ON_EN_CHANGE(IDC_PATH, &PPIniProcesses::OnEnChangePath)
   ON_EN_CHANGE(IDC_ID, &PPIniProcesses::OnEnChangeID)
   ON_EN_CHANGE(IDC_FIELD, &PPIniProcesses::OnEnChangeField)
   ON_EN_CHANGE(IDC_INITINFO, &PPIniProcesses::OnEnChangeInitStr)
   ON_BN_CLICKED(IDC_PREYEAR, &PPIniProcesses::OnBnClickedTiming)
   ON_BN_CLICKED(IDC_POSTYEAR, &PPIniProcesses::OnBnClickedTiming)
   ON_BN_CLICKED(IDC_USECOL, &PPIniProcesses::OnBnClickedUsecol)
END_MESSAGE_MAP()


BOOL PPIniProcesses::OnInitDialog()
   {
   CTabPageSSL::OnInitDialog();

   ENV_AUTO_PROCESS  *pInfo  = NULL;

   for ( int i=0; i < gpDoc->m_model.GetAutonomousProcessCount(); i++ )
      {
      pInfo = gpDoc->m_model.GetAutonomousProcessInfo( i );
      m_aps.AddString( pInfo->name );

      m_aps.SetCheck( i, pInfo->use ? 1 : 0 );
      }

   m_aps.SetCurSel( 0 );

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

   if ( gpDoc->m_model.GetAutonomousProcessCount() > 0 )
      LoadAP();

   return TRUE;  // return TRUE unless you set the focus to a control
   }


void PPIniProcesses::LoadAP()
   {
   ENV_AUTO_PROCESS *pInfo = GetAP();

   if ( pInfo == NULL )
      return;

   m_label   = pInfo->name;
   m_path    = pInfo->path;
   m_timing  = pInfo->timing;
   m_ID      = pInfo->id;
   m_useCol  = pInfo->col >= 0 ? TRUE : FALSE;
   m_fieldName  = pInfo->fieldName;
   m_initStr = pInfo->initInfo;

   m_aps.SetCheck( m_aps.GetCurSel(), pInfo->use ? 1 : 0 );

   m_isProcessDirty = false;

   UpdateData( 0 );
   }


ENV_AUTO_PROCESS *PPIniProcesses::GetAP()
   {
   int index = m_aps.GetCurSel();

   if ( index >= 0 )
      return EnvModel::GetAutonomousProcessInfo( index );
   else
      return NULL;
   }


bool PPIniProcesses::StoreChanges()
   {
   if ( m_isProcessDirty )
      {
      // not needed at this point
      }

   return true;
   }

void PPIniProcesses::MakeDirty()
   {
   m_isProcessesDirty = true;
   m_pParent->MakeDirty( INI_PROCESSES );
   }


void PPIniProcesses::OnLbnChkchangeAps()
   {
   ENV_AUTO_PROCESS *pInfo = GetAP();

   if ( pInfo != NULL )
      {
      int index = m_aps.GetCurSel();
      int check = m_aps.GetCheck( index );

      pInfo->use = ( check ? true : false);
      MakeDirty();
      }
   }


void PPIniProcesses::OnBnClickedAdd()
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
      int index = m_aps.AddString( path );
      m_aps.SetCurSel( index );

      EnvLoader loader;
      loader.LoadAP( &(gpDoc->m_model), path, path, 0, 1, 0, 0, 0, NULL, NULL, NULL, 0 );

      LoadAP();
      MakeDirty();
      }
   }


void PPIniProcesses::OnBnClickedRemove()
   {
   int index = m_aps.GetCurSel();

   if ( index < 0 )
      return;

   m_aps.DeleteString( index );

   gpDoc->m_model.RemoveAutonomousProcess( index );
   m_aps.SetCurSel( 0 );
   LoadAP();
   MakeDirty();
   }


void PPIniProcesses::OnLbnSelchangeAps()
   {
   LoadAP();
   }


void PPIniProcesses::OnBnClickedBrowse()
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


void PPIniProcesses::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
   {
   CTabPageSSL::OnActivate(nState, pWndOther, bMinimized);
   }


void PPIniProcesses::OnEnChangeLabel()
   {
   UpdateData();

   int index = m_aps.GetCurSel();

   if ( index >= 0 )
      {
      m_aps.InsertString( index, m_label );
      m_aps.DeleteString( index+1 );
      m_aps.SetCurSel( index );

      ENV_AUTO_PROCESS *pInfo = EnvModel::GetAutonomousProcessInfo( index );

      if ( pInfo != NULL )
         pInfo->name = m_label;
   
      MakeDirty();
      }
   }

void PPIniProcesses::OnEnChangePath()
   {
   UpdateData();

   ENV_AUTO_PROCESS *pInfo = GetAP();

   if ( pInfo != NULL )
      {
      pInfo->path = m_path;
      MakeDirty();
      }
   }

void PPIniProcesses::OnEnChangeID()
   {
   UpdateData();

   ENV_AUTO_PROCESS *pInfo = GetAP();

   if ( pInfo != NULL )
      {
      pInfo->id = m_ID;
      MakeDirty();
      }
   }


void PPIniProcesses::OnBnClickedTiming()
   {
   ENV_AUTO_PROCESS *pInfo = GetAP();

   if ( pInfo == NULL )
      return;

   if ( this->IsDlgButtonChecked( IDC_PREYEAR ) )
      pInfo->timing = 0;
   else
      pInfo->timing = 1;
   
   MakeDirty();
   }


void PPIniProcesses::OnBnClickedUsecol()
   {
   UpdateData();
   ENV_AUTO_PROCESS *pInfo = GetAP();

   if ( pInfo == NULL )
      return;

   MakeDirty();

   if ( this->IsDlgButtonChecked( IDC_FIELD ) )
      {
      GetDlgItem( IDC_FIELD )->EnableWindow( 1 );
      pInfo->fieldName = m_fieldName;
      }
   else
      {
      GetDlgItem( IDC_FIELD )->EnableWindow( 0 );
      pInfo->fieldName.Empty();
      }
   }

void PPIniProcesses::OnEnChangeField()
   {
   UpdateData();

   ENV_AUTO_PROCESS *pInfo = GetAP();

   if ( pInfo != NULL )
      {
      pInfo->fieldName = m_fieldName;
      MakeDirty();
      }
   }

void PPIniProcesses::OnEnChangeInitStr()
   {
   UpdateData();

   ENV_AUTO_PROCESS *pInfo = GetAP();

   if ( pInfo != NULL )
      {
      pInfo->initInfo = m_initStr;
      MakeDirty();
      }
   }