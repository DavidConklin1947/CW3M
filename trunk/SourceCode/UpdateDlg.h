#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include <PtrArray.h>


// UpdateDlg dialog

class StudyAreaInfo
   {
   public:
      CString m_name;
      CString m_path;
      CString m_localVer;
      CString m_webVer;
   };

class UpdateDlg : public CDialogEx
{
	DECLARE_DYNAMIC(UpdateDlg)

public:
	UpdateDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~UpdateDlg();

// Dialog Data
	enum { IDD = IDD_UPDATE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

   PtrArray< StudyAreaInfo > m_saArray;

	DECLARE_MESSAGE_MAP()
public:
   CStatic m_versionInfo;
   BOOL m_disable;
   
   bool Init();  // return true to run dialg, false to not run dialog

   virtual BOOL OnInitDialog();
   virtual void OnCancel();
   virtual void OnOK();

   void SaveUpdateStatusToReg( void );

   CFont m_updateFont;

   
   CListCtrl m_studyAreas;
   CStatic m_updateText;

   CString m_updateMsg;
   };
