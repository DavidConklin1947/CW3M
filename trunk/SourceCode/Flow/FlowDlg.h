#pragma once

#include <TreePropertySheet\TreePropSheetEx.h>
using namespace TreePropSheet;

#include "PPBasics.h"
#include "PPOutputs.h"

// FlowDlg

class FlowDlg : public CTreePropSheetEx
{
	DECLARE_DYNAMIC(FlowDlg)

public:
	FlowDlg(FlowModel *pFlow, LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	virtual ~FlowDlg();

   PPBasics  m_basicPage;                
   
   // PPInputsPage         m_inputsPage;       // climate, tables
   // PPMethodsPage        m_methodsPage;      // global methods, flux handlers
   // PPReservoirPage      m_reservoirPage;    // reservoir, control points? (may need a separte page for control pts?)
   // PPParamEstPage       m_paramEstPage;     // parameter Estimation
   // PPControlPointsPage  m_controlPtsPage
   // PPExtraSVsPage       m_extraSVPage;
   PPOutputs m_outputsPage;   // add videoCapture?

protected:
	DECLARE_MESSAGE_MAP()
public:
   //virtual BOOL OnInitDialog();
   };


