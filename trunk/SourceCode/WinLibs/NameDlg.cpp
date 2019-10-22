// NameDlg.cpp : implementation file
//

#include "winlibs.h"
#include "Libs.h"
#include "NameDlg.h"
#include "afxdialogex.h"


// NameDlg dialog

IMPLEMENT_DYNAMIC(NameDlg, CDialog)

NameDlg::NameDlg(CWnd* pParent /*=NULL*/)
	: CDialog(NameDlg::IDD, pParent)
   , m_name(_T(""))
   , m_title( _T("Enter name") )
   {

}

NameDlg::~NameDlg()
{
}

void NameDlg::DoDataExchange(CDataExchange* pDX)
{
CDialog::DoDataExchange(pDX);
DDX_Text(pDX, IDC_LIB_NAME, m_name);
   }


BEGIN_MESSAGE_MAP(NameDlg, CDialog)
END_MESSAGE_MAP()


// NameDlg message handlers

BOOL NameDlg::OnInitDialog() 
   {      
	CDialog::OnInitDialog();
   SetWindowText( m_title );

   //HWND hWnd;
   //GetDlgItem( IDC_LIB_NAME, &hWnd);
   //::PostMessage(hWnd, WM_SETFOCUS, 0, 0);

	return TRUE;
   }
