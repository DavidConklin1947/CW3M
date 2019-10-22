// ConstraintsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Envision.h"
#include "ConstraintsDlg.h"
#include "afxdialogex.h"

#include "EnvDoc.h"
#include "QueryDlg.h"
#include "Maplayer.h"

extern CEnvDoc  *gpDoc;
extern MapLayer *gpCellLayer;

// ConstraintsDlg dialog

IMPLEMENT_DYNAMIC(ConstraintsDlg, CDialog)

ConstraintsDlg::ConstraintsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(ConstraintsDlg::IDD, pParent)
{

}

ConstraintsDlg::~ConstraintsDlg()
{
}

void ConstraintsDlg::DoDataExchange(CDataExchange* pDX)
{
CDialog::DoDataExchange(pDX);
DDX_Control(pDX, IDC_CONSTRAINTS, m_constraints);
   }


BEGIN_MESSAGE_MAP(ConstraintsDlg, CDialog)
   ON_BN_CLICKED(IDC_ADD, &ConstraintsDlg::OnBnClickedAdd)
   ON_BN_CLICKED(IDC_REMOVE, &ConstraintsDlg::OnBnClickedRemove)
   ON_BN_CLICKED(IDC_EDIT, &ConstraintsDlg::OnBnClickedEdit)
END_MESSAGE_MAP()


// ConstraintsDlg message handlers


BOOL ConstraintsDlg::OnInitDialog()
   {
   CDialog::OnInitDialog();

   LoadConstraints();

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
   }

void ConstraintsDlg::LoadConstraints()
   {
   m_constraints.ResetContent();

   for ( int i=0; i < gpDoc->GetConstraintCount(); i++ )
      {
      CString &constraint = gpDoc->GetConstraint( i );
      m_constraints.AddString( constraint );
      }

   m_constraints.SetCurSel( 0 );
   }


void ConstraintsDlg::OnOK()
   {
   gpDoc->RemoveConstraints();

   for ( int i=0; i < m_constraints.GetCount(); i++ )
      {
      CString constraint;
      m_constraints.GetText( i, constraint );
      if ( ! constraint.IsEmpty() )
         gpDoc->AddConstraint( constraint ); 
      }

   CDialog::OnOK();
   }


void ConstraintsDlg::OnBnClickedAdd()
   {
   QueryDlg dlg( gpCellLayer, -1, this );
   if ( dlg.DoModal() == IDCANCEL )
      {
      CString constraint = dlg.m_queryString;
      m_constraints.AddString( constraint );
      }
   }


void ConstraintsDlg::OnBnClickedRemove()
   {
   int index = m_constraints.GetCurSel();

   if ( index < 0 )
      return;

   m_constraints.DeleteString( index );
   m_constraints.SetCurSel( 0 );
   }


void ConstraintsDlg::OnBnClickedEdit()
   {
   int index = m_constraints.GetCurSel();

   if ( index < 0 )
      return;

   QueryDlg dlg( gpCellLayer, -1, this );
   
   CString constraint;
   m_constraints.GetText( index, constraint );
   dlg.m_queryString = constraint;
   if ( dlg.DoModal() == IDCANCEL )
      {
      constraint = dlg.m_queryString;
      m_constraints.DeleteString( index );
      m_constraints.InsertString( index, constraint );
      m_constraints.SetCurSel( index );
      }
   }
