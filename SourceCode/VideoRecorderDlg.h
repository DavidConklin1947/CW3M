#pragma once
#include "afxwin.h"


// VideoRecorderDlg dialog

class VideoRecorderDlg : public CDialogEx
{
	DECLARE_DYNAMIC(VideoRecorderDlg)

public:
	VideoRecorderDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~VideoRecorderDlg();

// Dialog Data
	enum { IDD = IDD_VIDEORECORDER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

   void LoadVR( void );
   void EnableControls();

   bool m_dirty;

   //int AddWindow( LPCTSTR name, CWnd *pWnd );
	DECLARE_MESSAGE_MAP()
public:
   CCheckListBox m_recorders;

   int m_frameRate;
   int m_quality;
   CString m_path;
   int m_timerInterval;
   BOOL m_useTimer;
   BOOL m_useMessage;
   BOOL m_useApplication;

   virtual BOOL OnInitDialog();
   afx_msg void OnLbnSelchangeWindows();
   afx_msg void OnEnChangeFramerate();
   afx_msg void OnEnChangeQuality();
   afx_msg void OnBnClickedCaptureinterval();
   afx_msg void OnEnChangeInterval();
   afx_msg void OnBnClickedMessage();
   afx_msg void OnBnClickedApplication();
   afx_msg void OnEnChangePath();
   afx_msg void OnCheckRecorders();
   };
