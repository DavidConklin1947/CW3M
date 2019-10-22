#pragma once

#include <TabPageSSL.h>
#include "resource.h"
#include "EnvEngine\Policy.h"
#include "afxwin.h"


class PolEditor;


// PPPolicyConstraints dialog

class PPPolicyConstraints : public CTabPageSSL
{
friend class PolEditor;

DECLARE_DYNAMIC(PPPolicyConstraints)

public:
	PPPolicyConstraints( PolEditor*, Policy *&pPolicy );
	virtual ~PPPolicyConstraints();

// Dialog Data
	enum { IDD = IDD_POLICYCONSTRAINTS };

protected:
   Policy    *&m_pPolicy;
   PolEditor  *m_pParent;
   
   int m_policyConstraintIndex;   // current policy constraint

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

   // Generated message map functions
   virtual BOOL OnInitDialog();

   void MakeDirtyPolicy( void );
   void LoadPolicy( void );
   bool StorePolicyChanges( void );
   void EnableControls( void );
   void LoadPolicyConstraint( void );
   void LoadGCCombo( void );

public:
   CString m_lookupExpr;
   CListBox m_policyConstraints;

   CComboBox m_assocGC;
   CString m_initCostExpr;
   CString m_maintenanceCostExpr;
   int m_duration;

   afx_msg void OnBnClickedAddpolicyconstraint();
   afx_msg void OnBnClickedDeletepolicyconstraint();
   afx_msg void OnLbnSelchangePolicyconstraints();
   afx_msg void OnEnChangeLookupTable();
   afx_msg void OnBnClickedUnitArea();  // used for both unit area and absolute
   afx_msg void OnCbnSelchangeGccombo();
   afx_msg void OnEnChangeInitCost();
   afx_msg void OnEnChangeMaintenanceCost();
   afx_msg void OnEnChangeDuration();
   afx_msg void OnBnClickedAddglobalconstraint();
   };
