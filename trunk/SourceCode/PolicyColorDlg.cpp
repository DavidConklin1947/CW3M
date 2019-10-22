// PolicyColorDlg.cpp : implementation file
//

#include "stdafx.h"
#pragma hdrstop

#include "Envision.h"
#include "EnvEngine\EnvModel.h"
#include ".\policycolordlg.h"
#include "EnvEngine\policy.h"


extern PolicyManager *gpPolicyManager;


// PolicyColorDlg dialog

IMPLEMENT_DYNAMIC(PolicyColorDlg, CDialog)
PolicyColorDlg::PolicyColorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(PolicyColorDlg::IDD, pParent)
{
}

PolicyColorDlg::~PolicyColorDlg()
{
}

void PolicyColorDlg::DoDataExchange(CDataExchange* pDX)
{
CDialog::DoDataExchange(pDX);
DDX_Control(pDX, IDC_RED, m_red);
DDX_Control(pDX, IDC_GREEN, m_green);
DDX_Control(pDX, IDC_BLUE, m_blue);
}


BEGIN_MESSAGE_MAP(PolicyColorDlg, CDialog)
   ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// PolicyColorDlg message handlers

void PolicyColorDlg::OnBnClickedOk()
   {
   int red   = m_red.GetCurSel()   - 1;
   int green = m_green.GetCurSel() - 1;
   int blue  = m_blue.GetCurSel()  - 1;

   // get scaling factors
   int count = EnvModel::GetMetagoalCount();
 
   float *maxScoreArray = new float[ count ];
   float *minScoreArray = new float[ count ];

   for ( int i=0; i < count; i++ )
      {
      maxScoreArray[ i ] = -3;
      minScoreArray[ i ] = 3;
      }

   for ( int i=0; i < gpPolicyManager->GetPolicyCount(); i++ )
      {
      Policy *pPolicy = gpPolicyManager->GetPolicy( i );

      for ( int j=0; j < count; j++ )
         {
         float score = pPolicy->GetGoalScore( j );
         if ( score > maxScoreArray[ j ] )
            maxScoreArray[ j ] = score;
         if ( score < minScoreArray[ j ] )
            minScoreArray[ j ] = score;
         }
      }

      // having scaling factors, get colors

   for ( int i=0; i < gpPolicyManager->GetPolicyCount(); i++ )
      {
      Policy *pPolicy = gpPolicyManager->GetPolicy( i );

      int _red = 0, _green = 0, _blue = 0;

      if ( red >= 0 )
         _red = int( 255 * ( pPolicy->GetGoalScore( red ) + minScoreArray[red] ) / (maxScoreArray[red]-minScoreArray[red]) );
      
      if ( green >= 0 )
         _green = int( 255 * ( pPolicy->GetGoalScore( green ) + minScoreArray[green] ) / (maxScoreArray[green]-minScoreArray[green]) );
   
      if ( blue >= 0 )
         _blue = int( 255 * ( pPolicy->GetGoalScore( blue ) + minScoreArray[blue] ) / (maxScoreArray[blue]-minScoreArray[blue]) );

      pPolicy->m_color = RGB( _red, _green, _blue );
      }

   delete [] minScoreArray;
   delete [] maxScoreArray;

   OnOK();
   }


BOOL PolicyColorDlg::OnInitDialog()
   {
   CDialog::OnInitDialog();

   m_red.AddString( "<don't use this color>" );
   m_green.AddString( "<don't use this color>" );
   m_blue.AddString( "<don't use this color>" );

   // add metagoals to the check list
   int count = EnvModel::GetMetagoalCount();    // only worry about metagoals for policies
   for ( int i=0; i < count; i++ )
      {
      ENV_EVAL_MODEL *pInfo = EnvModel::GetMetagoalModelInfo( i );
      if ( pInfo != NULL )
         {
         m_red.AddString( (LPCSTR) pInfo->name );
         m_green.AddString( (LPCSTR) pInfo->name );
         m_blue.AddString( (LPCSTR) pInfo->name );
         }
      }

   m_red.SetCurSel( 0 );
   m_green.SetCurSel( 0 );
   m_blue.SetCurSel( 0 );

   // TODO:  Add extra initialization here

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
   }
