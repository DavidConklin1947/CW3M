// Report.cpp : implementation file
//

#include "winlibs.h"
#include "ReportBox.h"
//#include "afxdialogex.h"
//#include "afxDesktopAlertWnd.h"


///////////////////////////////////////////////////////////////////////////

// ReportBox dialog

IMPLEMENT_DYNAMIC(ReportBox, CDialogEx)

ReportBox::ReportBox(CWnd* pParent /*=NULL*/)
	: CDialogEx(ReportBox::IDD, pParent)
   , m_msg(_T(""))
   , m_output(FALSE)
   , m_noShow(FALSE)
   {

}

ReportBox::~ReportBox()
{
}

void ReportBox::DoDataExchange(CDataExchange* pDX)
{
CDialogEx::DoDataExchange(pDX);
DDX_Text(pDX, IDC_LIB_MSG, m_msg);
DDX_Check(pDX, IDC_LIB_OUTPUT, m_output);
DDX_Check(pDX, IDC_LIB_NOMSGS, m_noShow);
   }


BEGIN_MESSAGE_MAP(ReportBox, CDialogEx)
END_MESSAGE_MAP()


// ReportBox message handlers


void ReportBox::OnOK()
   {
   UpdateData( 1 );

   CDialogEx::OnOK();
   }


BOOL ReportBox::OnInitDialog()
   {
   CDialogEx::OnInitDialog();

   this->SetWindowText( this->m_hdr );

   // TODO:  Add extra initialization here

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
   }
