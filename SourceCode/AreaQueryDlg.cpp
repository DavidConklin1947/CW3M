// AreaQueryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Envision.h"
#include "AreaQueryDlg.h"
#include ".\areaquerydlg.h"
#include <maplayer.h>

extern MapLayer *gpCellLayer;


// AreaQueryDlg dialog

IMPLEMENT_DYNAMIC(AreaQueryDlg, CDialog)
AreaQueryDlg::AreaQueryDlg(CWnd* pParent /*=NULL*/)
	: CDialog(AreaQueryDlg::IDD, pParent)
{
}

AreaQueryDlg::~AreaQueryDlg()
{
}

void AreaQueryDlg::DoDataExchange(CDataExchange* pDX)
{
CDialog::DoDataExchange(pDX);
DDX_Control(pDX, IDC_FIELDS, m_fields);
DDX_Control(pDX, IDC_VALUE, m_value);
}


BEGIN_MESSAGE_MAP(AreaQueryDlg, CDialog)
END_MESSAGE_MAP()


// AreaQueryDlg message handlers

BOOL AreaQueryDlg::OnInitDialog()
   {
   CDialog::OnInitDialog();

   for ( int i=0; i < gpCellLayer->GetFieldCount(); i++ )
      m_fields.AddString( gpCellLayer->GetFieldLabel( i ) );

   m_fields.SetCurSel( 0 );

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
   }

void AreaQueryDlg::OnOK()
   {
   //int index = m_fields.GetCurSel();
   CString label;
   m_fields.GetWindowText( label );

   m_col = gpCellLayer->GetFieldCol( label );
   ASSERT( m_col >= 0 );

   TYPE type = gpCellLayer->GetFieldType( m_col );

   CString value;
   m_value.GetWindowText( value );

   switch( type )
      {
      case TYPE_CHAR:
      case TYPE_BOOL:
      case TYPE_UINT:
      case TYPE_INT:
      case TYPE_ULONG:
      case TYPE_LONG:
      case TYPE_SHORT:
         m_dataValue = atoi( value );
         break;

      case TYPE_FLOAT:
         m_dataValue = (float) atof( value );
         break;

      case TYPE_DOUBLE:
         m_dataValue = atof( value );
         break;

      case TYPE_STRING:
      case TYPE_DSTRING:
         m_dataValue = value;
         break;

      default: 
         ASSERT( 0 );
      }

   CDialog::OnOK();
   }
