#pragma once
#include "afxwin.h"


// SpatialOperatorsDlg dialog

class SpatialOperatorsDlg : public CDialog
{
	DECLARE_DYNAMIC(SpatialOperatorsDlg)

public:
	SpatialOperatorsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~SpatialOperatorsDlg();

// Dialog Data
	enum { IDD = IDD_SPATIALOPERATORS };
   int m_group;
   float m_distance;
   float m_areaThreshold;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CEdit m_comment;
   virtual BOOL OnInitDialog();
protected:
   virtual void OnOK();
public:
   afx_msg void OnBnClickedNextto();
   afx_msg void OnBnClickedNexttoarea();
   afx_msg void OnBnClickedWithin();
   afx_msg void OnBnClickedWithinarea();
};
