// SysInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Envision.h"
#include "SysInfoDlg.h"
#include "afxdialogex.h"

#include <SysInfo.h>


// SysInfoDlg dialog

IMPLEMENT_DYNAMIC(SysInfoDlg, CDialog)

SysInfoDlg::SysInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(SysInfoDlg::IDD, pParent)
   , m_memUsed(_T(""))
   , m_physMemAvail(_T(""))
   , m_virtMemAvail(_T(""))
   , m_cpu(_T(""))
   {

}

SysInfoDlg::~SysInfoDlg()
{
}

void SysInfoDlg::DoDataExchange(CDataExchange* pDX)
{
CDialog::DoDataExchange(pDX);
DDX_Text(pDX, IDC_MEMUSED, m_memUsed);
DDX_Text(pDX, IDC_PHYSMEMAVAIL, m_physMemAvail);
DDX_Text(pDX, IDC_VIRTMEMAVAIL, m_virtMemAvail);
DDX_Text(pDX, IDC_CPU, m_cpu);
   }


BEGIN_MESSAGE_MAP(SysInfoDlg, CDialog)
END_MESSAGE_MAP()


// SysInfoDlg message handlers


BOOL SysInfoDlg::OnInitDialog()
   {
   CSysInfo si;
   si.Init();

   LONG memUsed = si.GetProcessMemoryUsage();
   //memUsed /= (1024*1024);
   m_memUsed.Format( _T("Process Memory Used: %li MB"), memUsed );

   DWORD64 physMemAvail = si.GetTotalPhys();
   physMemAvail /= (1024*1024);
   m_physMemAvail.Format( _T("Physical Memory Available: %li MB"), (LONG) physMemAvail );

   DWORD64 virtMemAvail = si.GetTotalVirtual();
   virtMemAvail /= (1024*1024);
   m_virtMemAvail.Format( _T("Virtual Memory Available: %li MB"), (LONG) virtMemAvail );

   LPCTSTR name = si.GetCPUNameString();
   //LPCTSTR id = si.GetCPUIdentifier();
   DWORD speed = si.GetCPUSpeed();
   DWORD processors = si.GetNumProcessors();
   m_cpu.Format( _T("CPU: %s, Speed=%i, Processors=%i"), name, (int) speed, (int) processors );
   
   CDialog::OnInitDialog();

   return TRUE;
   }
