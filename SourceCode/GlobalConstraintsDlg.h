#pragma once


class PolEditor;


// GlobalConstraintsDlg dialog

class GlobalConstraintsDlg : public CDialog
{
	DECLARE_DYNAMIC(GlobalConstraintsDlg)

public:
	GlobalConstraintsDlg(PolEditor *pPolEditor, CWnd* pParent = NULL);   // standard constructor
	virtual ~GlobalConstraintsDlg();

   PolEditor *m_pPolEditor;

   bool StoreGlobalChanges( void );
   void LoadGlobalConstraint( void );

   CListBox m_globalConstraints;
   CString m_maxAnnualValueExpr;

// Dialog Data
	enum { IDD = IDD_GLOBALCONSTRAINTS };

protected:
   int m_globalConstraintIndex;   // current global constraint

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

   void MakeDirtyGlobal( void );
   void EnableControls( void );

	DECLARE_MESSAGE_MAP()

   virtual BOOL OnInitDialog();

   afx_msg void OnBnClickedResourceLimit();  // for all buttons
   afx_msg void OnBnClickedAddgc();
   afx_msg void OnBnClickedDeletegc();
   afx_msg void OnLbnSelchangeGlobalconstraints();
   afx_msg void OnEnChangeMaxAnnualValue();

};
