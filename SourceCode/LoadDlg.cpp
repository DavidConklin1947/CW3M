// LoadDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Envision.h"
#include "LoadDlg.h"

#include <dirplaceholder.h>


// LoadDlg dialog

IMPLEMENT_DYNAMIC(LoadDlg, CDialog)

LoadDlg::LoadDlg(CWnd* pParent /*=NULL*/)
	: CDialog(LoadDlg::IDD, pParent)
   , m_name(_T(""))
   , m_path(_T(""))
   {

}

LoadDlg::~LoadDlg()
{
}

void LoadDlg::DoDataExchange(CDataExchange* pDX)
{
CDialog::DoDataExchange(pDX);
DDX_Text(pDX, IDC_NAME, m_name);
DDX_Text(pDX, IDC_PATH, m_path);
   }


BEGIN_MESSAGE_MAP(LoadDlg, CDialog)
   ON_BN_CLICKED(IDC_BROWSE, &LoadDlg::OnBnClickedBrowse)
END_MESSAGE_MAP()


// LoadDlg message handlers

void LoadDlg::OnBnClickedBrowse()
   {
   DirPlaceholder d;

   CFileDialog dlg( TRUE, "dll", m_path, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "DLL files|*.dll|All files|*.*||");

   if ( dlg.DoModal() == IDOK )
      {
      m_path = dlg.GetPathName();

      if ( m_name.IsEmpty() )
         m_name = m_path;

      UpdateData( 0 );
      }

   }

void LoadDlg::OnOK()
   {
   UpdateData( 1 );
   CDialog::OnOK();
   }

BOOL LoadDlg::OnInitDialog()
   {
   CDialog::OnInitDialog();

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
   }
