// PPPlotSetup.cpp : implementation file
//

#include "winlibs.h"

#include "graphwnd.h"
#include "PPPlotSetup.h"
#include ".\ppplotsetup.h"


// PPPlotSetup dialog

IMPLEMENT_DYNAMIC(PPPlotSetup, CPropertyPage)
PPPlotSetup::PPPlotSetup( GraphWnd *pGraph )
	: CPropertyPage(PPPlotSetup::IDD),
   m_pGraph( pGraph )
{
}

PPPlotSetup::~PPPlotSetup()
{
}

void PPPlotSetup::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(PPPlotSetup, CPropertyPage)
   ON_BN_CLICKED(IDC_FONTTITLE, &PPPlotSetup::OnBnClickedFonttitle)
   ON_BN_CLICKED(IDC_FONTXAXIS, &PPPlotSetup::OnBnClickedFontxaxis)
   ON_BN_CLICKED(IDC_FONTYAXIS, &PPPlotSetup::OnBnClickedFontyaxis)
   ON_EN_CHANGE(IDC_CAPTION, &PPPlotSetup::OnEnChangeCaption)
   ON_EN_CHANGE(IDC_TITLE, &PPPlotSetup::OnEnChangeTitle)
   ON_EN_CHANGE(IDC_XAXIS, &PPPlotSetup::OnEnChangeXaxis)
   ON_EN_CHANGE(IDC_YAXIS, &PPPlotSetup::OnEnChangeYaxis)
   ON_BN_CLICKED(IDC_SHOWHORZ, &PPPlotSetup::OnBnClickedShowhorz)
   ON_BN_CLICKED(IDC_SHOWVERT, &PPPlotSetup::OnBnClickedShowvert)
   ON_BN_CLICKED(IDC_GROOVED, &PPPlotSetup::OnBnClickedGrooved)
   ON_BN_CLICKED(IDC_RAISED, &PPPlotSetup::OnBnClickedRaised)
   ON_BN_CLICKED(IDC_SUNKEN, &PPPlotSetup::OnBnClickedSunken)
END_MESSAGE_MAP()


BOOL PPPlotSetup::OnInitDialog()
   {
   CPropertyPage::OnInitDialog();

   //-- initialization --//
   char buffer[ 120 ];

   //-- window caption --//
   m_pGraph->GetWindowText( buffer, 120 );
   SetDlgItemText( IDC_CAPTION, (LPCSTR) buffer );

   //- plot labels --//
   SetDlgItemText( IDC_TITLE, (LPCSTR) m_pGraph->label[ 0 ].text );
   SetDlgItemText( IDC_XAXIS, (LPCSTR) m_pGraph->label[ 1 ].text );
   SetDlgItemText( IDC_YAXIS, (LPCSTR) m_pGraph->label[ 2 ].text );

   //-- check marks --//
   CheckDlgButton( IDC_SHOWVERT, m_pGraph->showXGrid );
   CheckDlgButton( IDC_SHOWHORZ, m_pGraph->showYGrid );

   CheckDlgButton( IDC_GROOVED, m_pGraph->frameStyle & FS_GROOVED );
   CheckDlgButton( IDC_SUNKEN,  m_pGraph->frameStyle & FS_SUNKEN );
   CheckDlgButton( IDC_RAISED,  m_pGraph->frameStyle & FS_RAISED );

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
   }


BOOL PPPlotSetup::OnCommand(WPARAM wParam, LPARAM lParam)
   {
   int ctrlCode = HIWORD( wParam );
   int ctrlID   = LOWORD( wParam );

   if ( ctrlCode = BN_CLICKED )
      {
      switch( ctrlID )
         {
         case IDC_FONTTITLE:
         case IDC_FONTXAXIS:
         case IDC_FONTYAXIS:
            {
            int labelID = TITLE;
            if ( ctrlID == IDC_FONTXAXIS )
               labelID = XAXIS;
            else if ( ctrlID == IDC_FONTYAXIS )
               labelID = YAXIS;

            // get current font:
            LOGFONT lf;
            memset( &lf, 0, sizeof( LOGFONT ) );

            /*CFont &font = m_pGraph->label[ labelID ].font;
            font.GetLogFont( &lf );

            CFontDialog dlg(&lf);
            if ( dlg.DoModal() == IDOK )
               {
               font.CreateFontIndirect( &lf );
               m_pGraph->RedrawWindow();
               } */
            } 

         }
      }

   return CPropertyPage::OnCommand(wParam, lParam);
   }



bool PPPlotSetup::Apply( bool redraw /*=true*/ )
   {
   //-- get current values and load into box --//
   char buffer[ 120 ];

   //-- caption --//
   GetDlgItemText( IDC_CAPTION, buffer, 120 );
   m_pGraph->SetWindowText((LPCSTR) buffer );

   GetDlgItemText( IDC_TITLE, buffer, 120 );
   m_pGraph->SetLabelText( TITLE, buffer );

   GetDlgItemText( IDC_XAXIS, buffer, 120 );
   m_pGraph->SetLabelText( XAXIS, buffer );

   GetDlgItemText( IDC_YAXIS, buffer, 120 );
   m_pGraph->SetLabelText( YAXIS, buffer );

   //-- scaling --//
   m_pGraph->showXGrid = IsDlgButtonChecked( IDC_SHOWVERT ) ? true : false;
   m_pGraph->showYGrid = IsDlgButtonChecked( IDC_SHOWHORZ ) ? true : false;

   //-- frame style --//
   m_pGraph->frameStyle = 0;
   if ( IsDlgButtonChecked( IDC_GROOVED ) )
      m_pGraph->frameStyle |= FS_GROOVED;
   else if ( IsDlgButtonChecked( IDC_SUNKEN ) )
      m_pGraph->frameStyle |= FS_SUNKEN;
   else if ( IsDlgButtonChecked( IDC_RAISED ) )
      m_pGraph->frameStyle |= FS_RAISED;
      
   if ( redraw )
      m_pGraph->RedrawWindow();

   return true;
   }


void PPPlotSetup::OnBnClickedFonttitle()
   {
   CFontDialog dlg(&(m_pGraph->label[ TITLE ].logFont ) );
   if ( dlg.DoModal() == IDOK )
      m_pGraph->RedrawWindow();
   }


void PPPlotSetup::OnBnClickedFontxaxis()
   {
   CFontDialog dlg(&(m_pGraph->label[ XAXIS ].logFont ) );
   if ( dlg.DoModal() == IDOK )
      m_pGraph->RedrawWindow();
   }


void PPPlotSetup::OnBnClickedFontyaxis()
   {
   CFontDialog dlg(&(m_pGraph->label[ YAXIS ].logFont ) );
   if ( dlg.DoModal() == IDOK )
      m_pGraph->RedrawWindow();
   }


void PPPlotSetup::OnEnChangeCaption()
   {
   CString text;
   GetDlgItemText( IDC_CAPTION, text );

   m_pGraph->SetWindowText( text );
   }


void PPPlotSetup::OnEnChangeTitle()
   {
   CString text;
   GetDlgItemText( IDC_TITLE, text );

   m_pGraph->label[ TITLE ].text = text;
   m_pGraph->RedrawWindow();
   }


void PPPlotSetup::OnEnChangeXaxis()
   {
   CString text;
   GetDlgItemText( IDC_XAXIS, text );

   m_pGraph->label[ XAXIS ].text = text;
   m_pGraph->RedrawWindow();
   }


void PPPlotSetup::OnEnChangeYaxis()
   {
   CString text;
   GetDlgItemText( IDC_YAXIS, text );

   m_pGraph->label[ YAXIS ].text = text;
   m_pGraph->RedrawWindow();
   }


void PPPlotSetup::OnBnClickedShowhorz()
   {
   m_pGraph->showYGrid = IsDlgButtonChecked( IDC_SHOWHORZ ) ? true : false;
   m_pGraph->RedrawWindow();
   }


void PPPlotSetup::OnBnClickedShowvert()
   {
   m_pGraph->showXGrid = IsDlgButtonChecked( IDC_SHOWVERT ) ? true : false;
   m_pGraph->RedrawWindow();
   }


void PPPlotSetup::OnBnClickedGrooved()
   {
   m_pGraph->frameStyle = FS_GROOVED;
   m_pGraph->RedrawWindow();
   }


void PPPlotSetup::OnBnClickedRaised()
   {
   m_pGraph->frameStyle = FS_RAISED;
   m_pGraph->RedrawWindow();
   }


void PPPlotSetup::OnBnClickedSunken()
   {
   m_pGraph->frameStyle = FS_SUNKEN;
   m_pGraph->RedrawWindow();
   }
