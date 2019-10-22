// EvalModelLearnDlg.cpp : implementation file
#include "stdafx.h"
#define NOMINMAX
#include <utility>
using std::pair;

#include "EvalModelLearnDlg.h"

#ifndef __INI_TEST__
#include "Sandbox.h"
#include ".\evalmodellearndlg.h"
extern PolicyManager   *gpPolicyManager ;
extern ScenarioManager *gpScenarioManager;
extern EnvModel        *gpModel;
const int bHeight = 50;
const int bWidth  = 80;
const int margin  = 3;
#endif
#undef NOMINMAX
//===============================
#ifdef __INI_TEST__             //
CArray<Policy> g_TestPolicies;  //
EnvModel model;                 //
EnvModel * gpModel = &model;               //
#endif                          //
//=======================================================================================
IMPLEMENT_DYNAMIC(EvalModelLearnDlg, CDialog)
EvalModelLearnDlg::EvalModelLearnDlg(CWnd* pParent /*=NULL*/)
	: CDialog(EvalModelLearnDlg::IDD, pParent)
{
}

EvalModelLearnDlg::~EvalModelLearnDlg()
{
}

BOOL EvalModelLearnDlg::OnInitDialog() 
{
   CDialog::OnInitDialog();

   Policy * pPolicy = NULL;
   int polID = -1;
   int lbi;
   int i;
   pair< map<int,Policy*>::iterator, bool > polPr;

#ifndef __INI_TEST__
   
   // TODO: Add extra initialization here
   ASSERT(gpScenarioManager);
   ASSERT(gpPolicyManager);
   ASSERT(gpModel);

   // Models list box
   DWORD modelCount = gpModel->GetModelCount();
   ENV_EVAL_MODEL * mi;
   int idx;

   for( DWORD m = 0; m < modelCount; ++m) 
      {
      mi = gpModel->GetModelInfo(m);
      idx = m_modelListBox.AddString(mi->name);
      if (idx < 0)
         {
         ASSERT (idx != LB_ERR && idx != LB_ERRSPACE);
         return true;
         }
      VERIFY ( LB_ERR != m_modelListBox.SetItemData(idx, m));
      VERIFY ( LB_ERR != m_modelListBox.SetItemDataPtr(idx, mi));
      VERIFY ( LB_ERR != m_modelListBox.SetSel(idx, 1));
      }

   // scenarios and spanning set of policies controls.
   int  scenCount = gpScenarioManager->GetCount();
   int  scenVarCount = -1;

   Scenario * pScenario = NULL; // can't const because Scenario has no const members

   ////LVITEM lvitem {                                       
   ////   mask,                                         
   ////      0, // iItem
   ////      0, // iSubItem
   ////      state,
   ////      LVIS_SELECTED, // stateMask
   ////      NULL, // pszText
   ////      32, //int cchTextMax; 
   ////      0, // iIMage
   ////      0, // lParam,
   ////      0, // iIndent
   ////      0, // iGroupId;
   ////      1, // UINT cColumns; // tile view columns
   ////      NULL //PUINT puColumns;
   ////   };
  for( i = 0; i < scenCount; ++i) 
      {
      pScenario = gpScenarioManager->GetScenario(i);
      scenVarCount = pScenario->GetVarCount();
      CString scenName = pScenario->m_name;
      lbi = this->m_scenarioListBox.AddString(scenName);
      if (lbi < 0)
         {
         ASSERT (lbi != LB_ERR && lbi != LB_ERRSPACE);
         return true;
         }

      this->m_scenarioListBox.SetItemDataPtr(lbi, pScenario);

      // UNSELECT Default Scenario
      scenName.MakeUpper();
      if (-1 == scenName.Find("DEFAULT"))
         {
         VERIFY (LB_ERR != m_scenarioListBox.SetSel(lbi, 1));
         } 
      else 
         {
         VERIFY ( LB_ERR != m_scenarioListBox.SetSel(lbi, 0));
         continue;                         
         }

      // GET THE SPANNING (over Scenario) SET OF POLICIES
      int polCount = pScenario->GetPolicyCount();
      for (int j = 0; j < polCount; ++j)
         {
         polID = -1;
         pPolicy = NULL;
         POLICY_INFO &info = pScenario->GetPolicyInfo(j);

         if ( info.inUse == false ) 
            continue;

         pPolicy = info.pPolicy;
         polID  = pPolicy->m_id;

         polPr = m_policySet.insert(pair<int,Policy*>(polID, pPolicy)); 
         }
      }
#endif
   //-------------------------------------------------------//
      #ifdef __INI_TEST__                                   //
      lbi = this->m_scenarioListBox.AddString("LB00");      //
      VERIFY (LB_ERR != m_scenarioListBox.SetSel(lbi, 1));  //
      lbi = this->m_scenarioListBox.AddString("LB01");      //
      VERIFY (LB_ERR != m_scenarioListBox.SetSel(lbi, 1));  //
      lbi = this->m_scenarioListBox.AddString("LB02");      //
      VERIFY (LB_ERR != m_scenarioListBox.SetSel(lbi, 1));  //
      //                                                    //
      g_TestPolicies.Add(Policy("P00", 0));                 //
      g_TestPolicies.Add(Policy("P01", 1));                 //
      g_TestPolicies.Add(Policy("P02", 2));                 //
      for( i=0; i< g_TestPolicies.GetCount(); ++i)          //////
         polPr = m_policySet.insert(pair<int,Policy*>(g_TestPolicies.GetAt(i).m_id, &g_TestPolicies.GetAt(i))); //
      #endif                                                /////
   //-------------------------------------------------------

   UpdatePolicyListControl();

   //UpdateData( false );

   return TRUE;   // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}
//=======================================================================================
void EvalModelLearnDlg::UpdatePolicyListControl()
   {
   int j;
   map< int, Policy * >::iterator polIt;
   UINT mask =  LVIF_STATE | LVIF_TEXT | LVIF_PARAM; // 
   UINT state = LVIS_SELECTED ;
   UINT stateMask = LVIS_SELECTED ;
   ASSERT(sizeof (LPARAM) == sizeof(polIt->second));

   m_policyListCtrl.DeleteAllItems();

   for (j = 0, polIt = m_policySet.begin(); polIt != m_policySet.end();  ++polIt )
      {
      int nItem = j++;
      m_policyListCtrl.InsertItem(nItem, polIt->second->m_name);
      if (0 == this->m_policyListCtrl.SetItem(
         nItem,                  //int nItem,
         -1,                     //0, //int nSubItem,
         mask,                   // UINT nMask,
         polIt->second->m_name,  // LPCTSTR lpszItem,
         0,                      // int nImage,
         state,                  //UINT nState,
         stateMask,              //UINT nStateMask,
         reinterpret_cast<LPARAM>(polIt->second), //LPARAM lParam,
         0                       //int nIndent 
         ))
         {
         ASSERT(0);// error
         }
      }
   }
//=======================================================================================
void EvalModelLearnDlg::OnOK() 
{
   // TODO: Add extra validation here
   
   // Ensure that your UI got the necessary input 
   // from the user before closing the dialog. The 
   // default OnOK will close this.

   //UpdateData(TRUE);

   for (int k = 0; k < m_policyListCtrl.GetItemCount(); ++k)      
      {
      if ( ! (LVIS_SELECTED & m_policyListCtrl.GetItemState(k, LVIS_SELECTED)) )
         {
         Policy * pPolicy = reinterpret_cast<Policy*>(m_policyListCtrl.GetItemData(k));
         int polID = pPolicy->m_id;
         m_policySet.erase(polID);
         }
      }

   DWORD_PTR dx;
   int midx;
   ENV_EVAL_MODEL * mi;
   for (int m = 0; m < m_modelListBox.GetCount(); ++ m)
      {
      if ( 0 < m_modelListBox.GetSel(m) )
         {
         mi = static_cast<ENV_EVAL_MODEL*> (m_modelListBox.GetItemDataPtr(m));
         dx = m_modelListBox.GetItemData(m); // strangely wrong
         midx = gpModel->FindModelIndex(mi->name);
         m_idxModelInfo.insert(midx);
         }
      }

   m_scenarioSet.clear();  
   CArray<int,int> aryListBoxSel;
   int nCount = m_scenarioListBox.GetCount();
   aryListBoxSel.SetSize(nCount);
   int nSel = m_scenarioListBox.GetSelItems(nCount, aryListBoxSel.GetData());
   for (int i=0; i<nSel; ++i)
      m_scenarioSet.insert(static_cast<Scenario*>(m_scenarioListBox.GetItemDataPtr(aryListBoxSel[i])));

   CDialog::OnOK(); // This will close the dialog and DoModal will return.
}

void EvalModelLearnDlg::DoDataExchange(CDataExchange* pDX)
{
CDialog::DoDataExchange(pDX);
DDX_Control(pDX, IDC_LB_IDD_EVAL_MODEL_LEARN_DLG, m_scenarioListBox);
DDX_Control(pDX, IDC_LC_IDD_EVAL_MODEL_LEARN_DLG, m_policyListCtrl);
DDX_Control(pDX, IDC_LB_MODELS_IDD_EVAL_MODEL_LEARN_DLG, m_modelListBox);
}


BEGIN_MESSAGE_MAP(EvalModelLearnDlg, CDialog)
   //ON_WM_CREATE()
   ON_LBN_SELCHANGE(IDC_LB_IDD_EVAL_MODEL_LEARN_DLG, OnLbnSelchangeLbIddEvalModelLearnDlg)
END_MESSAGE_MAP()

void EvalModelLearnDlg::OnLbnSelchangeLbIddEvalModelLearnDlg()
   {
   // TODO: Add your control notification handler code here
   // Get the indexes of all the selected items.
   // Redo the spanning set of policies.
   pair< map<int,Policy*>::iterator, bool > polPr;
   CArray<int,int> aryListBoxSel;
   int nCount = m_scenarioListBox.GetCount();
   aryListBoxSel.SetSize(nCount);
   int nSel = m_scenarioListBox.GetSelItems(nCount, aryListBoxSel.GetData());
   
   m_policySet.clear();

   for (int i=0; i<nSel; ++i)
      {    
      #ifndef __INI_TEST__
      Scenario * pScenario = static_cast<Scenario*>(m_scenarioListBox.GetItemDataPtr(aryListBoxSel[i]));
      int polCount = pScenario->GetPolicyCount();

      for (int j = 0; j < polCount; ++j)
         {
         int polID = -1;
         Policy *pPolicy = NULL;
         POLICY_INFO &info = pScenario->GetPolicyInfo(j);

         if ( info.inUse == false ) 
            continue;

         pPolicy = info.pPolicy;
         polID  = pPolicy->m_id;

         polPr = m_policySet.insert(pair<int,Policy*>(polID, pPolicy)); 
         }
      #endif
      #ifdef __INI_TEST__
         polPr = m_policySet.insert(pair<int,Policy*>(g_TestPolicies[aryListBoxSel[i]].m_id, &g_TestPolicies[aryListBoxSel[i]]));          
      #endif
      }
   
   UpdatePolicyListControl();

   }
