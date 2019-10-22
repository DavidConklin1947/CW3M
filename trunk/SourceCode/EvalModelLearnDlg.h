#pragma once
#define NOMINMAX
#include "resource.h"
#include "afxwin.h"
#include "afxcmn.h"
#undef NOMINMAX

#include "EvalModelLearning.h"

#include <map>
#include <set>
using std::set;
using std::map;

class EvalModelLearnDlg : public CDialog
{
	DECLARE_DYNAMIC(EvalModelLearnDlg)

public:
	EvalModelLearnDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~EvalModelLearnDlg();

// Dialog Data
	enum { IDD = IDD_EVAL_MODEL_LEARN_DLG };

   map< int, Policy * > m_policySet;  // <PolID, Policy*> to be consumed by users of this
   set<int>          m_idxModelInfo; // indices in EnvModel::m_modelInfoArray
   set<Scenario *>   m_scenarioSet;


protected:
   void EvalModelLearnDlg::UpdatePolicyListControl();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   virtual BOOL OnInitDialog() ;
   virtual void OnOK(  );



	DECLARE_MESSAGE_MAP()
public:
   //afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
   CListBox m_scenarioListBox;
   CListCtrl m_policyListCtrl;
   afx_msg void OnLbnSelchangeLbIddEvalModelLearnDlg();
   CListBox m_modelListBox;
};
