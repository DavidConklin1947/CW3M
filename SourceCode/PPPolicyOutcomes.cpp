// PPPolicyOutcomes.cpp : implementation file
//
#include "stdafx.h"

#include ".\pppolicyoutcomes.h"

#include "EnvEngine\Policy.h"
#include "PolEditor.h"
#include "outcomeEditor.h"
#include "EnvEngine\EnvModel.h"

extern PolicyManager *gpPolicyManager;
extern EnvModel      *gpModel;

// PPPolicyOutcomes dialog

IMPLEMENT_DYNAMIC(PPPolicyOutcomes, CTabPageSSL)

PPPolicyOutcomes::PPPolicyOutcomes( PolEditor *pParent, Policy *&pPolicy )
	: CTabPageSSL()
   , m_pPolicy( pPolicy )
   , m_pParent( pParent )
{ }

PPPolicyOutcomes::~PPPolicyOutcomes()
{ }


void PPPolicyOutcomes::DoDataExchange(CDataExchange* pDX)
{
CTabPageSSL::DoDataExchange(pDX);
DDX_Control(pDX, IDC_OUTCOMELIST, m_outcomeList);
DDX_Control(pDX, IDC_OUTCOME, m_outcomes);
}

BEGIN_MESSAGE_MAP(PPPolicyOutcomes, CTabPageSSL)
   ON_BN_CLICKED(IDC_ADD, OnBnClickedAdd)
   ON_BN_CLICKED(IDC_EDIT, OnBnClickedEdit)
   ON_LBN_SELCHANGE(IDC_OUTCOMELIST, OnLbnSelchangeOutcomelist)
   ON_EN_CHANGE(IDC_OUTCOME, OnEnChangeOutcomes)
   ON_BN_CLICKED(IDC_DELETE, &PPPolicyOutcomes::OnBnClickedDelete)
   ON_BN_CLICKED(IDC_UPDATE, &PPPolicyOutcomes::OnBnClickedUpdate)
END_MESSAGE_MAP()


// PPPolicyOutcomes message handlers

BOOL PPPolicyOutcomes::OnInitDialog()
   {
   CTabPageSSL::OnInitDialog();

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
   }


void PPPolicyOutcomes::LoadPolicy()
   {
   m_outcomeList.ResetContent();
   m_outcomes.SetWindowText( "" );

   if ( m_pPolicy == NULL )
      return;

   int moCount= m_pPolicy->m_multiOutcomeArray.GetSize();
   if ( moCount == 0 )
      return;

   for ( int i=0; i < moCount; i++ )
      {
      MultiOutcomeInfo &moInfo = m_pPolicy->m_multiOutcomeArray.GetMultiOutcome( i );

      CString moStr;
      moInfo.ToString( moStr );
      m_outcomeList.AddString( moStr );
      }

   if ( moCount > 0 )
      m_outcomeList.SetCurSel( 0 );
   else
      m_outcomeList.SetCurSel( -1 );

   RefreshOutcomes();

   BOOL enabled = FALSE;
   if ( m_pParent->IsEditable() )
      enabled = TRUE;
      
   GetDlgItem( IDC_OUTCOME )->EnableWindow( enabled );
   GetDlgItem( IDC_UPDATE )->EnableWindow( 0 );
   }


void PPPolicyOutcomes::RefreshOutcomes()
   {
   CString multiOutcome;

   for ( int i=0; i < m_outcomeList.GetCount(); i++ )
      {
      CString outcome;
      m_outcomeList.GetText( i, outcome );

      multiOutcome += outcome;

      if ( i != m_outcomeList.GetCount()-1 )
         multiOutcome += ";\r\n";
      }

   m_outcomes.SetWindowText( multiOutcome );
   GetDlgItem( IDC_UPDATE )->EnableWindow( 0 );
   }


// make sure any changes are stored in the policy
bool PPPolicyOutcomes::StoreChanges()
   {
   if ( m_pParent->IsDirty( DIRTY_OUTCOMES ) )
      {
      CString multiOutcome;
      m_outcomes.GetWindowText( multiOutcome );

      multiOutcome.Replace( _T("\n"), NULL );   // strip newlines
      multiOutcome.Replace( _T("\r"), NULL );   // strip line returns
      multiOutcome.Replace( _T("\t"), NULL );   // strip tabs
      
      // is the outcome string legitimate?  NOTE THIS DOESN"T WORK IF CONSTRAINTS ARE DEFINED
      if ( m_pPolicy->SetOutcomes( multiOutcome ) == false )
         {
         CString msg( "Error compiling policy outcome: " );
         msg += multiOutcome;
         msg += "   You can continue, but this policy will not be functional until the query is fixed.";
         Report::WarningMsg( msg );
         }
      else
         gpPolicyManager->ValidateOutcomeFields( m_pPolicy, gpModel->m_pIDULayer );

      // policy outcomes string okay, now check if the probabilities add up to 100

      int outcomeCount = m_pPolicy->GetMultiOutcomeCount();
      int totalWeight = 0;
      for ( int i = 0; i < outcomeCount; i++ )
         totalWeight += m_pPolicy->GetMultiOutcome( i ).GetProbability();

      if ( totalWeight < 100 )
         {
         CString msg;
         msg.Format( "The sum of outcome probabilities is %d.  ", totalWeight );
         msg.AppendFormat( "The remaining %d will be interpreted as the chance that no action will be taken.", 100 - totalWeight );
         Report::WarningMsg( msg );
         }
      }

   m_pParent->MakeClean( DIRTY_OUTCOMES );   

   return true;
   }


void PPPolicyOutcomes::OnBnClickedAdd()
   {
   OutcomeEditor dlg( this->m_pPolicy );

   if ( dlg.DoModal() == IDOK )
      {
      if ( dlg.m_outcome.GetLength() > 0 )
         {
         int index = m_outcomeList.AddString( dlg.m_outcome );
         m_outcomeList.SetCurSel( index );
         RefreshOutcomes();
         m_pParent->MakeDirty( DIRTY_OUTCOMES );
         }
      }
   }


void PPPolicyOutcomes::OnBnClickedEdit()
   {
   int index = m_outcomeList.GetCurSel();

   if ( index < 0 )
      return;

   OutcomeEditor dlg( m_pPolicy );

   // set the dialogs m_outcome editor text to the currently selected outcome;
   m_outcomeList.GetText( index, dlg.m_outcome );
   
   if ( dlg.DoModal() == IDOK )
      {
      int index = m_outcomeList.GetCurSel();
      
      m_outcomeList.InsertString( index, dlg.m_outcome );
      m_outcomeList.DeleteString( index+1 );
      m_outcomeList.SetCurSel( index );
      RefreshOutcomes();
      m_pParent->MakeDirty( DIRTY_OUTCOMES );
      }
   }


void PPPolicyOutcomes::OnBnClickedDelete()
   {
   int index = m_outcomeList.GetCurSel();

   if ( index < 0 )
      return;

   int count = m_outcomeList.DeleteString( index );
   index = count - 1;
   m_outcomeList.SetCurSel( index );
   RefreshOutcomes();
   m_pParent->MakeDirty( DIRTY_OUTCOMES );
   }


void PPPolicyOutcomes::OnLbnSelchangeOutcomelist()
   {
   //LoadOutcome();
   }


void PPPolicyOutcomes::OnEnChangeOutcomes()
   {
   GetDlgItem( IDC_UPDATE )->EnableWindow( 1 );
   m_pParent->MakeDirty( DIRTY_OUTCOMES );
   }


void PPPolicyOutcomes::OnBnClickedUpdate()
   {
   TCHAR multiOutcome[ 512 ];
   m_outcomes.GetWindowText( multiOutcome, 512 );

   m_outcomeList.ResetContent();

   LPTSTR token;
   LPTSTR nextToken;

   token = _tcstok_s( multiOutcome, _T(";\r\n"), &nextToken );
   while ( token != NULL )
      {
      m_outcomeList.AddString( token );
      token = _tcstok_s( NULL, _T(";\r\n"), &nextToken );
      }

   GetDlgItem( IDC_UPDATE )->EnableWindow( 0 );
   m_outcomeList.SetCurSel( 0 );
   }



