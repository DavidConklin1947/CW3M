#pragma once

#include "EnvEngine\Policy.h"


// OutcomeConstraintsDlg dialog

class OutcomeConstraintsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(OutcomeConstraintsDlg)

public:
	OutcomeConstraintsDlg(CWnd* pParent, MultiOutcomeInfo *pOutcome, Policy *pPolicy );   // standard constructor
	virtual ~OutcomeConstraintsDlg();

// Dialog Data
	enum { IDD = IDD_OUTCOMECONSTRAINTS };
protected:

   bool m_isDirtyGlobal;
   bool m_isDirtyOutcome;

   Policy *m_pPolicy;
   MultiOutcomeInfo *m_pOutcome;   // ptr to OUR COPY of an outcome in the current policy
   PtrArray< GlobalConstraint > m_constraintArray;   // local copies of global constraitns stored in policy manager
   
   int m_globalConstraintIndex;    // current global constraint
   int m_outcomeConstraintIndex;   // current policy constraint

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

   // Generated message map functions
   virtual BOOL OnInitDialog();

   void MakeDirtyGlobal ( void ) { m_isDirtyGlobal = true; }
   void MakeDirtyOutcome( void ) { m_isDirtyOutcome = true; }
   void LoadOutcome( void );
   bool StoreGlobalChanges( void );
   bool StoreOutcomeChanges( void );
   void EnableControls( void );
   void LoadGlobalConstraint( void );
   void LoadOutcomeConstraint( void );

public:
   CListBox m_globalConstraints;
   CListBox m_outcomeConstraints;
   CString m_maxAnnualValueExpr;

   CComboBox m_assocGC;
   CString m_initCostExpr;
   CString m_maintenanceCostExpr;
   int m_duration;

   afx_msg void OnBnClickedAddgc();
   afx_msg void OnBnClickedDeletegc();
   afx_msg void OnBnClickedAddOutcomeConstraint();
   afx_msg void OnBnClickedDeleteOutcomeConstraint();
   afx_msg void OnLbnSelchangeOutcomeConstraints();
   afx_msg void OnLbnSelchangeGlobalconstraints();
   afx_msg void OnBnClickedResourceLimit();
   afx_msg void OnBnClickedUnitArea();
   afx_msg void OnCbnSelchangeGccombo();
   afx_msg void OnEnChangeMaxAnnualValue();
   afx_msg void OnEnChangeInitCost();
   afx_msg void OnEnChangeMaintenanceCost();
   afx_msg void OnEnChangeDuration();
   virtual void OnOK();
   };
